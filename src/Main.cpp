/*
 * MAIN.cpp (Entry Point)
 * Creation Date : 24/08/2020
 * Creator : Nora465
*/

#include "MainHeader.h"

//------------- GLOBAL-DECLARATIONS --------------------------------------------------------------
AsyncWebServer	server(50500);	// Instantiate an AsyncWebServer (listen to requests on port 50500)
WiFiUDP ntpUDP; //The UDP instance for NTPClient
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", TIME_OFFSET_S, TIME_UPDATE_INTERVAL);

ScheduleDay gSchedule[6];
bool gShowFullWeek[2]; //how we show the schedule on phone (the 7 days of the week, or the work week and the weekend)
uint8_t gCurrentDay; //Current day of the week (1: Monday ... 7: Sunday)

//bool gCC1State, gCC2State = false; 	//a modifier par un fichier du SPIFFS
bool gStates[2] = {true, true}; //CIRCUITS states (NOT RELAYS)
bool gModeAuto = true; //Mode of the circuit (true: Mode AUTOmatic // false: Mode MANUal) (can be modified by androidApp or physically)
//---------------------------------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);
	Serial.println();

	//----------------------- Gestion_Pins ---------------------------------------
	SetPinsMode(); //Pins Allocation Management (IN/OUT, output states by default)

	//------------------ ----- Gestion_WiFi ---------------------------------------
	ConnectToAP(); //Connect to WiFi AP with WiFiManager (captive portal)

	//------------- HANDLE WEB ---------------------------------------------------

	server.on("/ForceState", [](AsyncWebServerRequest *request) {
		HandleForceState(request, gStates);
	});

	server.on("/GetStates", [](AsyncWebServerRequest *request) {
		HandleGetState(request, gStates);
	});

	server.on("/ChangeMode", [](AsyncWebServerRequest *request) {
		HandleChangeMode(request, &gModeAuto);
	});

	//---- HTTP HANDLES : SCHEDULE ---------------
	server.on("/GetSchedule", [](AsyncWebServerRequest *request) {
		HandleGetSchedule(request, gSchedule, gShowFullWeek);
	});

	//TODO problem : https://github.com/me-no-dev/ESPAsyncWebServer/issues/902 and Issue#904 !
	//HTTP_POST = 2 for MeNoDev (but is 3 for the basic lib ?)
	server.on("/ModifySchedule", 2, [](AsyncWebServerRequest *request) {
		HandleModifySchedule(request, gSchedule, gShowFullWeek);
		//TODO function : alarms update ?
	});

	server.begin();
	if (SHOW_DEBUG) Serial.println("[WebServer] Server HTTP DEMARRE !\n");
//---------------------------------------------------------------------------------------------------

	//StartNTPClient(timeClient); //start the NTPClient and force update the Time (and NTPTime)
	
	//Read Schedule from EEPROM
	LoadEEPROMSchedule(gSchedule, gShowFullWeek, true);

	//Alarm FOR TESTING
	Alarm.timerRepeat(15, [](){
		if (SHOW_DEBUG) Serial.println("TICK 15 secondes");
	});
}

void loop() {
	//Time
	TryToUpdateTime(timeClient); //Update the Time if "update Interval"(NTPClient) has been reached
	
	//Gestion ALARMES
	Alarm.delay(1000);

	//display of time (NTP)
	if (SHOW_DEBUG) Serial.println(timeClient.getFormattedTime()); //the NTPTime

	/* //display of temperature
	//Temperature acquisition (LM61 sensor)
	float voltage = analogRead(A0) * (3.3/1023.0);
	float temp = (voltage - 0.6) * 100.0;
	if (SHOW_DEBUG) Serial.println("Temperature : " + String(temp));
	delay(5000);
	*/
}