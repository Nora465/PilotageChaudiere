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

ScheduleDay gDailySchedule[6];
uint8_t gCurrentDay; //Current day of the week (1: Monday ... 7: Sunday)

//bool gCC1State, gCC2State = false; 	//a modifier par un fichier du SPIFFS
bool gStates[2] = {true, true}; //Etats des CIRCUITS (pas des relais)
bool gModeAuto = true; //Mode de gestion des circuits (true: Mode AUTO // false: Mode MANU) TELEPHONE OU PHYSIQUE
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
		HandleChangeMode(request, gModeAuto);
	});

	server.on("/ModifySchedule", [](AsyncWebServerRequest *request) {
		HandleModifySchedule(request, gDailySchedule);
	});

	server.begin();
	Serial.println("[WebServer] Server HTTP DEMARRE !\n");
//---------------------------------------------------------------------------------------------------

	StartNTPClient(timeClient); //start the NTPClient and force update the Time (and NTPTime)

	//Read Schedule from EEPROM
	LoadEEPROMSchedule(gDailySchedule, true);

	//Gestion alarme TEST
	Alarm.timerRepeat(15, [](){
		Serial.println("TICK 15 secondes");
	});
}
 
void loop() {
	//Gestion Time
	TryToUpdateTime(timeClient); //Update the Time if "update Interval"(NTPClient) has been reached
	
	//Gestion ALARMES
	Alarm.delay(1000);

	//affichage temps (NTP)
	Serial.println(timeClient.getFormattedTime()); //the NTPTime

	/* //affichage température
	//Récupération de la température (capteur LM61)
	float tension = analogRead(A0) * (3.3/1023.0);
	float temp = (tension - 0.6) * 100.0;
	Serial.println("Temp : " + String(temp));
	delay(5000);
	*/
}