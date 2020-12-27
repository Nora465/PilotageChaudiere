/* Management of the TIME
 * (NTP / Alarms)
 *
 * Creation Date : 05/11/2020 19h41
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File
#include "DailySchedule.h"	//Management of the daily schedule

/*
 * Start the NTPClient, configure it, and force update of Time - Should be called ONE time
*/
void StartNTPClient(NTPClient &timeClient) {
	timeClient.begin();
	TryToUpdateTime(timeClient, true); //2nd argument will force the update
	if (timeStatus() == timeSet) {
		Serial.println("[NTP Client] Client NTP Démarré (il est " + String(timeClient.getFormattedTime()) + ")");
	} else {
		Serial.println("[NTP Client] Erreur, impossible de mettre à jour l'heure \nRedemarrage...");
		ESP.restart();
	}
}

/**
 * Update the Time (and NTPTime) if update Interval (1 day) is reached
 * @param timeClient <NTPClient> Reference of the NTPClient
 * @param forceUpdate <Bool> will force the update if true (default is false)
 */
void TryToUpdateTime(NTPClient &timeClient, bool forceUpdate) {
	//if forceUpdate is True,  Force the update (Send a request to NTPServer to get the Epoch time)
	//if forceUpdate is False, will update only if lastUpdate > updateInterval (1 day))
	bool TimeWasUpdated = forceUpdate ? timeClient.forceUpdate() : timeClient.update();
	//If NTPTime has been updated, update the Time
	if (TimeWasUpdated) setTime((time_t) timeClient.getEpochTime() + TIME_OFFSET_S);
}

void SetTheNextAlarm(TimeAlarmsClass alarm) {
	
}