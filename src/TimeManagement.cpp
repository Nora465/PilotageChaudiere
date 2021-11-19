/* Management of the TIME
 * (NTP / Alarms)
 *
 * Creation Date : 05/11/2020 19h41
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File

//Global variables : Main.cpp
extern Timezone TZ_fr;
extern ScheduleDay gSchedule[7];
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
		if (SHOW_DEBUG) Serial.println("[NTP Client] Client NTP Demarre (il est " + String(timeClient.getFormattedTime()) + ")");
	} else {
		if (SHOW_DEBUG) Serial.println("[NTP Client] Erreur, impossible de mettre a jour l'heure \nRedemarrage...");
		appendStrToFile("[Time] Unable to update the time, reboot ...");
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
	forceUpdate ? timeClient.forceUpdate() : timeClient.update();
	
	//True local time, depending on DST and time offset
	time_t localTime = TZ_fr.toLocal(timeClient.getEpochTime());

	//If the ESP time is not the local time => update the Time
	if (now() != localTime) {
		setTime(localTime);
	}
}

/**
 * Return the current day of the week (1 is monday, and 7 is sunday)
 * Use weekday() (this return 1 for sunday, 2 is monday ... and 6 for sunday)
 * @return uint8_t - The current day of the week
 */
uint8_t GetNormalWeekDay() {
	if (weekday() == 1) return 7;
	else 				return weekday() - 1;
}

/**
 * Set or Overwrite the alarm of the schedule (and updates the state of circuit 1)
 */
void CreateNewAlarm() {
	if (Alarm.isAllocated(gMyAlarmID)) Alarm.free(gMyAlarmID); //Delete the old alarm (if already allocated)

	uint8_t curDay = GetNormalWeekDay() - 1; //Current day of the week (0: Monday .. 6:Sunday) for the schedule array
	uint8_t curWeekDay = weekday(); //memorize the week day for further use

	//Range 1 : Work Range
	//Range 2 : Holiday Range
	uint8_t tempTriggerHour=0, hourToTrigger=0, tempRetardDayTrigger=0, retardDayTrigger=0;
	bool tempNextState=false, NextState=false;
		//Loop through all periods
	for (uint8_t i=0; i<NUM_OF_PERIODS; i++) {

		if (hour() < gSchedule[curDay].Ranges[gCurRange-1].Periods[i].Start) {
			tempTriggerHour = gSchedule[curDay].Ranges[gCurRange-1].Periods[i].Start;
			tempNextState = true;
			tempRetardDayTrigger= curWeekDay;
			if (SHOW_DEBUG) Serial.println("start");
			appendStrToFile("[Alrm-Plage1] Le prochain etat est : START");
		}
		else if (hour() < gSchedule[curDay].Ranges[gCurRange-1].Periods[i].Stop) {
			tempTriggerHour = gSchedule[curDay].Ranges[gCurRange-1].Periods[i].Stop;
			tempNextState = false; 
			tempRetardDayTrigger= curWeekDay;
			if (SHOW_DEBUG) Serial.println("stop");
			appendStrToFile("[Alrm-Plage1] Le prochain etat est : STOP");
		}
		//TODO peut etre mettre un else à la place ?
		else if (hour() >= gSchedule[curDay].Ranges[gCurRange-1].Periods[i].Stop) {
			//if curDay is 6, adding one will be out of range, so taking 0
			tempTriggerHour = gSchedule[(curDay==6)? 0 : curDay+ 1].Ranges[gCurRange-1].Periods[i].Start;
			tempNextState = true;
			tempRetardDayTrigger = curWeekDay + 1; //The trigger is for next day
			if (SHOW_DEBUG) Serial.println("start+1");
			appendStrToFile("[Alrm-Plage1] Le prochain etat est : START (next day)");
		}

		//First loop => don't check
		if (i == 0) {
			NextState= tempNextState;
			hourToTrigger= tempTriggerHour;
			retardDayTrigger= tempRetardDayTrigger;
		} else {
			//1st case : Both start Periods are "True"
			if (NextState and tempNextState) {
				NextState = true;
				//1.1 case : Both periods start today => TriggerHour takes the lowest trigger hour
				if (tempRetardDayTrigger == curWeekDay and retardDayTrigger == curWeekDay) {
					hourToTrigger = min(hourToTrigger, tempTriggerHour);
					retardDayTrigger = curWeekDay;
					if (SHOW_DEBUG) Serial.println("case 1.1");
					appendStrToFile("case 1.1 - " + String(hourToTrigger) + "/" + String(tempTriggerHour));
				//1.2 case : Both periods start tomorrow => TriggerHour takes the lowest trigger hour
				}
				else if (tempRetardDayTrigger == curWeekDay+1 and retardDayTrigger == curWeekDay+1) {
					hourToTrigger = min(hourToTrigger, tempTriggerHour);
					retardDayTrigger = curWeekDay + 1;
					if (SHOW_DEBUG) Serial.println("case 1.2");
					appendStrToFile("case 1.2 - "+ String(hourToTrigger) + "/" + String(tempTriggerHour));
				//1.3 case : One of periods start tomorrow => TriggerHour takes the "true" start period
				//TODO on peut le passer en "else" non ?
				}
				else if (tempRetardDayTrigger == curWeekDay+1 or retardDayTrigger == curWeekDay+1) {
					hourToTrigger = (tempRetardDayTrigger == curWeekDay)? tempTriggerHour: hourToTrigger;
					retardDayTrigger = (tempRetardDayTrigger== curWeekDay)? tempRetardDayTrigger: retardDayTrigger;
					if (SHOW_DEBUG) Serial.println("case 1.3");
					appendStrToFile("case 1.3 - "+ String(hourToTrigger) + "/" + String(tempTriggerHour));
				}
			}
			//2nd case : One of periods is "FALSE" => TriggerHour takes the false period's hour
			else if(!NextState or !tempNextState) {
				if (SHOW_DEBUG) Serial.println("second case");
				appendStrToFile("case 2 - "+ String(hourToTrigger) + "/" + String(tempTriggerHour));
				NextState = false;
				hourToTrigger= !tempNextState? tempTriggerHour: hourToTrigger;
			}
		}
	}

	if (SHOW_DEBUG) Serial.println("Next State is : " + String(NextState?"True":"False"));
	if (SHOW_DEBUG) Serial.println("Next Alarm hour : " + String(hourToTrigger));
	appendStrToFile("Next State is " + String(NextState?"true":"false") +" \nNext Alarm hour : "+hourToTrigger);

	gNextState= NextState;

	//Update the outputs (relays and leds)
	ToggleCircuitState(1, !gNextState);

	if (SHOW_DEBUG) Serial.println("day: "+String(curDay) + " actualHour: "+ String(hour()) +" hour: " + String(hourToTrigger));
	appendStrToFile("(dayofweekNR:" + String(curDay) + ") il est " + String(hour()) + "h // On attend l'alarme pour " + String(hourToTrigger)+"h");
	appendStrToFile("[StateCC1] = " + String(!digitalRead(RELAY_CC1)));

	gMyAlarmID = Alarm.alarmOnce((timeDayOfWeek_t)retardDayTrigger, hourToTrigger, 0, 0, TimeHandle);
}

/**
 * Handle : executed when alarm is triggered
 */
void TimeHandle() {
	bool success = ToggleCircuitState(1, gNextState);
	appendStrToFile("[AlarmRing] L'alarme est arrivee! La modif du circuit 1 est un : " + String(success?"Succes":"Echec"));

	CreateNewAlarm();
	appendStrToFile("==========================");
}