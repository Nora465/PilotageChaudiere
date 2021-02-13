/* Management of the TIME
 * (NTP / Alarms)
 *
 * Creation Date : 05/11/2020 19h41
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File
#include "DailySchedule.h"	//Management of the daily schedule

//Global variables : Main.cpp
extern ScheduleDay gSchedule[6];
extern NTPClient timeClient;
extern AlarmID_t gMyAlarmID;
extern uint8_t gCurRange;
extern bool gStates[2];
extern bool gNextState; //Next state of the circuit 1 (defined by the schedule)

/*
 * Start the NTPClient, configure it, and force update of Time - Should be called ONE time
*/
void StartNTPClient(NTPClient &timeClient) {
	timeClient.begin();
	TryToUpdateTime(timeClient, true); //2nd argument will force the update

	//If ESP can't get the time, reboot the ESP
	//TODO if reboot, send a notification ? or a mail
	if (timeStatus() == timeSet) {
		if (SHOW_DEBUG) Serial.println("[NTP Client] Client NTP Démarré (il est " + String(timeClient.getFormattedTime()) + ")");
	} else {
		if (SHOW_DEBUG) Serial.println("[NTP Client] Erreur, impossible de mettre à jour l'heure \nRedemarrage...");
		ESP.restart();
	}
}

/**
 * Update the Time (and NTPTime) if update Interval (1 day) is reached
 * @param timeClient NTPClient - Reference of the NTPClient
 * @param forceUpdate Bool - will force the update if true (default is false)
 */
void TryToUpdateTime(NTPClient &timeClient, bool forceUpdate) {
	//if forceUpdate is True,  Force the update (Send a request to NTPServer to get the Epoch time)
	//if forceUpdate is False, will update only if lastUpdate > updateInterval (1 day))
	bool TimeWasUpdated = forceUpdate ? timeClient.forceUpdate() : timeClient.update();
	//If NTPTime has been updated, update the Time
	if (TimeWasUpdated) setTime((time_t) timeClient.getEpochTime() + TIME_OFFSET_S);
}

/**
 * Return the current day of the week (0 is monday, and 6 is sunday)
 * Use weekday() (this return 1 for sunday, 2 is monday ... and 6 for sunday)
 * @return uint8_t - The current day of the week
 */
uint8_t GetNonRetardDay() {
	if (weekday() == 1) return 6;
	else 				return weekday() - 2;
}

/**
 * Set or Overwrite the alarm of the schedule (and updates the state of circuit 1)
 * @return AlarmdID_t(uint8_t) - ID of the alarm (0 if error)
 */
AlarmID_t CreateNewAlarm() {
	if (Alarm.isAllocated(gMyAlarmID)) Alarm.free(gMyAlarmID); //Delete the old alarm (if already allocated)

	uint8_t curDay = GetNonRetardDay(); //Current day of the week (0: Monday .. 6:Sunday) for the schedule array
	uint8_t retardDayTrigger = weekday(); //Set the day on which the alarm need to be triggered
	
	uint8_t hourToTrigger = 0;
	if (gCurRange == 1) {//Work Range
		if (hour() < gSchedule[curDay].P1Start) {
			hourToTrigger = gSchedule[curDay].P1Start;
			gNextState = true;
			if (SHOW_DEBUG) Serial.println("start");
			appendStrToFile("NextState : start");
		}
		else if (hour() < gSchedule[curDay].P1Stop) {
			hourToTrigger = gSchedule[curDay].P1Stop;
			gNextState = false; 
			if (SHOW_DEBUG) Serial.println("stop");
			appendStrToFile("NextState : stop");
		}
		else if (hour() >= gSchedule[curDay].P1Stop) {
			hourToTrigger = gSchedule[(curDay==6)? 0 : curDay+ 1].P1Start; //if curDay is 6, adding one will be out of range, so taking 0
			retardDayTrigger += 1; //The trigger is for next day
			gNextState = true;
			if (SHOW_DEBUG) Serial.println("start+1");
			appendStrToFile("NextState : start (next day)");
		}
		else {
			if (SHOW_DEBUG) Serial.println("dab" + String(hour()) + " P1start: " + String(gSchedule[curDay].P1Start) + " P1Stop: " +String(gSchedule[curDay].P1Stop));
		}

	} else if (gCurRange == 2) { //Holiday Range
		if (hour() < gSchedule[curDay].P2Start) {
			hourToTrigger = gSchedule[curDay].P2Start; 
			gNextState = true;
			if (SHOW_DEBUG) Serial.println("start");
			appendStrToFile("NextState : start");
		}
		else if (hour() < gSchedule[curDay].P2Stop) {
			hourToTrigger = gSchedule[curDay].P2Stop;
			gNextState = false; 
			if (SHOW_DEBUG) Serial.println("stop");
			appendStrToFile("NextState : stop");
		}
		else if (hour() >= gSchedule[curDay].P2Stop) {
			hourToTrigger = gSchedule[(curDay==6)? 0 : curDay+ 1].P2Start; //if curDay is 6, adding one will be out of range, so taking 0
			retardDayTrigger += 1; //The trigger is for next day
			gNextState = true;
			if (SHOW_DEBUG) Serial.println("start+1");
			appendStrToFile("NextState : start (next day)");
		}
		else {
			if (SHOW_DEBUG) Serial.println("dab" + String(hour()) + " P2start: " + String(gSchedule[curDay].P2Start) + " P2Stop: " +String(gSchedule[curDay].P2Stop));
		}
	} else return 0;

	//Update the outputs (relays and leds)
	ToggleCircuitState(1, !gNextState);

	if (SHOW_DEBUG) Serial.println("day: "+String(curDay) + " actualHour: "+ String(hour()) +" hour: " + String(hourToTrigger));
	appendStrToFile("(dayofweekNR:" + String(curDay) + ") il est " + String(hour()) + "h // On attend l'alarme pour " + String(hourToTrigger)+"h");
	appendStrToFile("[currentState] = " + String(digitalRead(RELAY_CC1)));

	return Alarm.alarmOnce((timeDayOfWeek_t)retardDayTrigger, hourToTrigger, 0, 0, TimeHandle);
}

/**
 * Handle : executed when alarm is triggered
 */
void TimeHandle() {
	bool success = ToggleCircuitState(1, gNextState);
	appendStrToFile("[TimeTriggered] La modification du circuit est : " + String(success));

	gMyAlarmID = CreateNewAlarm();
	appendStrToFile("==========================");
}