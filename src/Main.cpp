/*
 * MAIN.cpp (Entry Point)
 * Creation Date : 24/08/2020
 * Creator : Nora465
*/

#include "MainHeader.h"

//------------- GLOBAL-DECLARATIONS --------------------------------------------------------------
AsyncWebServer	server(50500);	// Instantiate an AsyncWebServer (listen to requests on port 50500)
WiFiUDP 		ntpUDP; //The UDP instance for NTPClient
NTPClient 		timeClient(ntpUDP, "europe.pool.ntp.org", TIME_OFFSET_S, TIME_UPDATE_INTERVAL);

//TODO passer la structure en un tableau 2D gSchedule[7(jours)][4(P1Start, P2...)]
ScheduleDay gSchedule[7] = {0,0,0,0,0,0,0};
bool gShowFullWeek[2] = {true, true}; //how to display the schedule ON PHONE (the 7 days of the week, or the work week and the weekend)

bool gStates[2] = {true, true}; //circuits states (NOT RELAYS) CC1: Boiler || CC2: Heaters
bool gModeAuto = true; //Mode of the circuit (true: Mode AUTOmatic // false: Mode MANUal) (can be modified by androidApp or physically)
uint8_t gCurRange= 1; //Current Range (Range2 - Heaters)
AlarmID_t gMyAlarmID;
bool gNextState= false; //Next state of the circuit 1 (defined by the schedule)
float gOutTemp = 0.0; //Outdoor temperature
//---------------------------------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);
	Serial.println();

//---------------------------- Gestion_Pins ---------------------------------------
	SetPinsMode(); //Pins Allocation Management (IN/OUT, output states by default)

//---------------------------- Gestion_WiFi ---------------------------------------
	ConnectToAP(); //Connect to WiFi AP with WiFiManager (captive portal)

	startLittleFS();
	String debug = "===\nREBOOT\n-PtrReason: " + String(ESP.getResetInfoPtr()->reason);
	debug += "\n-exccause: " + String(ESP.getResetInfoPtr()->exccause);
	debug += "\n-excvaddr: " + String(ESP.getResetInfoPtr()->excvaddr);
	debug += "\n-epc1: " + String(ESP.getResetInfoPtr()->epc1);
	debug += "\n-epc2: " + String(ESP.getResetInfoPtr()->epc2);
	debug += "\n-epc3: " + String(ESP.getResetInfoPtr()->epc3) + "\n===";
	appendStrToFile(debug);
	Serial.println(debug);
//------------------ HANDLE WEB ---------------------------------------------------
//TODO ajouter une handle "/" pour inviter à télécharger la dernière version de l'appli android
//----------- States of Relays
	server.on("/GetStates", [](AsyncWebServerRequest *request) {
		HandleGetState(request);
	});

	server.on("/ForceState", [](AsyncWebServerRequest *request) {
		HandleForceState(request);
	});
//----------- Mode AUTO/MANU
	server.on("/GetMode", [](AsyncWebServerRequest *request) {
		HandleGetMode(request);
	});

	server.on("/SetMode", [](AsyncWebServerRequest *request) {
		HandleSetMode(request);
	});
//----------- Range (1 is Work // 2 is holiday)
	server.on("/GetRange", [] (AsyncWebServerRequest *request) {
		HandleGetRange(request);
	});

	server.on("/SetRange", [] (AsyncWebServerRequest *request) {
		HandleSetRange(request);
	});

//--------- Schedule
	server.on("/GetSchedule", [](AsyncWebServerRequest *request) {
		HandleGetSchedule(request, gShowFullWeek);
	});

	//TODO problem : https://github.com/me-no-dev/ESPAsyncWebServer/issues/902 and Issue#904 !
	//HTTP_POST = 2 for MeNoDev (but it's 3 for the esp8266 lib ?)
	server.on("/ModifySchedule", 2, [](AsyncWebServerRequest *request) {
		HandleModifySchedule(request, gShowFullWeek);
	});

	//TODO Test Code - à supprimer quand les tests sont reussis
	server.on("/UpdateHour", [](AsyncWebServerRequest *request) {
		HandledabSchedule(request);
	});

	server.on("/deleteLog", [](AsyncWebServerRequest *request) {
		bool success = DeleteLogFile();
		request->send((success)?200:404, "text/plain", (success)?"Fichier log.txt effacé !":"erreur pendant la suppresion du fichier log.txt");
	});

	server.onNotFound([](AsyncWebServerRequest *request){
		if (request->url() == "/log") {
			return request->send(LittleFS, "/log.txt", "text/plain");
		}
		Serial.println(request->url() + " => Aucun path pour ce fichier web");
		request->send(404, "text/plain", "désolé, aucun fichier ne correspond à cette uri :'(");
	});

	server.begin();
	if (SHOW_DEBUG) Serial.println("[WebServer] Server HTTP DEMARRE !\n");
//---------------------------------------------------------------------------------------------------

	StartNTPClient(timeClient); //start the NTPClient and force update the Time (and NTPTime)

	//Read Schedule from EEPROM
	bool success = LoadEEPROMSchedule(gSchedule, true);
	if (success) gMyAlarmID = CreateNewAlarm(); //Create an alarm to change the state of circuit (when schedule say so)
	else {
		//if error while loading the schedule from EEPROM, enable Manual mode, and activate both circuits
		gModeAuto = false;
		ToggleCircuitState(1, true);
		ToggleCircuitState(2, true);
	}

	
	appendStrToFile("==========================");
}

void loop() {
	//Update the Time if the "update Interval" (inside NTPClient) has been reached (every day)
	TryToUpdateTime(timeClient);
	
	//In AUTO Mode, check if an alarm has reached the threshold, and execute the Handle
	if (gModeAuto) Alarm.delay(1000);

	//display of time (NTP)
	//if (SHOW_DEBUG) Serial.println(timeClient.getFormattedTime()); //the NTPTime
	//Serial.println(String(hour()) + ":"+String(minute()) + ":" + String(second()));
	
	/* //display of temperature
	//Temperature acquisition (LM61 sensor)
	float voltage = analogRead(A0) * (3.3/1023.0);
	float temp = (voltage - 0.6) * 100.0;
	if (SHOW_DEBUG) Serial.println("Temperature : " + String(temp));
	delay(5000);
	*/
}