/*
 * MAIN.cpp (Entry Point)
 * Creation Date : 24/08/2020
 * Creator : Nora465
*/

#include "MainHeader.h"

//TODO revoir le log de tout les fichiers

//------------- GLOBAL-DECLARATIONS --------------------------------------------------------------
AsyncWebServer	server(50500);	// Instantiate an AsyncWebServer (listen to requests on port 50500)
WiFiUDP 		ntpUDP; //The UDP instance for NTPClient
NTPClient 		timeClient(ntpUDP, "europe.pool.ntp.org", 0, TIME_UPDATE_INTERVAL);

//TimeZone STD "Standard" and DST "DayLight Savings"
TimeChangeRule frSTD = {"CET",  Last, Sun, Oct, 3, 60};  //CET  : Starts the last Sunday of October at 3H00 => UTC+1 (Winter)
TimeChangeRule frDST = {"CEST", Last, Sun, Mar, 2, 120}; //CEST : Starts the last sunday of March   at 2H00 => UTC+2 (Summer)
Timezone TZ_fr(frDST, frSTD);

//Schedule
ScheduleDay gSchedule[7]; //index 0->6
bool gShowFullWeek[2] = {true, true}; //how to display the schedule ON PHONE (the 7 days of the week, or the work week and the weekend)
bool gNextState= false; //Next state of the circuit 1 (defined by the schedule)
AlarmID_t gMyAlarmID;
bool gSchedIsInEEPROM[NUM_OF_PERIODS] = {false, false}; //The schedule has been configured by user (= "Auto" mode can be enabled)

//State of boiler and Buttons
bool gModeAuto; //Mode of the circuit (true: Mode AUTOmatic // false: Mode MANUal) (can be modified only by androidApp)
bool gBPModeManu; //Physical state of button1 "Mode Auto/Manu" (can be modified only by physical buttons)
bool gMemBPMANUAUTO, gMemBPFORCEON_OFF; //memorize the previous state (for loop())
uint8_t gCurRange= 1; //Current Range (Range2 - Heaters)
bool gStates[2] = {true, true}; //circuits states (NOT RELAYS) CC1: Boiler || CC2: Heaters

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
	if (SHOW_DEBUG) Serial.println(debug);
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
		HandleGetSchedule(request);
	});

	//TODO problem : https://github.com/me-no-dev/ESPAsyncWebServer/issues/902 and Issue#904 !
	//HTTP_POST = 2 for MeNoDev (but it's 3 for the esp8266 lib ?)
	server.on("/ModifySchedule", 2, [](AsyncWebServerRequest *request) {
		HandleModifySchedule(request);
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

	server.on("/getTime", [] (AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(day(now())) + "/" + String(month(now())) + "/" + String(year(now())) + " " + String(hour(now())) + "h" + String(minute(now())) + "m" + String(second(now())) + "s");
	});

	server.begin();
	if (SHOW_DEBUG) Serial.println("[WebServer] Server HTTP DEMARRE !\n");
//---------------------------------------------------------------------------------------------------

	StartNTPClient(timeClient); //start the NTPClient and force update the Time (and NTPTime)

	//Read Schedule from EEPROM and Auto/Manu button1 state
	gModeAuto = LoadEEPROMSchedule(gSchedule, true);
	gMemBPMANUAUTO = !gBPModeManu;
	gBPModeManu = not digitalRead(BP1_AUTOMANU);

	if (gBPModeManu) {
		//button1 is "Manu" => get On/Off value of button2
		ToggleCircuitState(1, digitalRead(BP2_FORCEON_OFF));
		appendStrToFile("[Setup] Button1 is Manu => state of button2: " + String(digitalRead(BP2_FORCEON_OFF)?"ON":"OFF"));
	} else if (gModeAuto) {
		//schedule exists and button1 is "auto" => enable automatic alarm
		CreateNewAlarm();
		appendStrToFile("[Setup] Button1 is Auto => Schedule exists => mode AUTO");
	} else if (not gModeAuto) {
		//Schedule error and button1 is "auto" => enable Manual mode, and activate both circuits
		ToggleCircuitState(1, true);
		//appendStrToFile("[ProgHoraire] Aucun programme enregistré, passage en mode MANUel !");
		appendStrToFile("[Setup] Button1 is Auto => Schedule doesn't exists => mode Manu");
	}
	ToggleCircuitState(2, true); //TODO gérer le stockage de l'état du CC2 à chaque modifications
	
	if (SHOW_DEBUG) Serial.println("[Setup] BP AutoManu is : " + String(gBPModeManu?"Manu":"Auto"));
	appendStrToFile("[Setup] BP AutoManu is : " + String(gBPModeManu?"Manu":"Auto"));

	appendStrToFile("===========end setup===========");

}

void loop() {
	//Update the Time if the "update Interval" (inside NTPClient) has been reached (every day)
	TryToUpdateTime(timeClient);

	//Check : State of button1 BP1_AUTOMANU
	int8_t resultBP = testBP(BP1_AUTOMANU, false, &gMemBPMANUAUTO); //1 is auto, 0 is manu
	if (resultBP == 1) { //Raising Edge (Auto)
		gBPModeManu = false;
		gMemBPFORCEON_OFF = !digitalRead(BP2_FORCEON_OFF); //reset the memory to update the state of circuit 1
		//TODO check if real autoMode is enabled : if so, start new alarm, if not, don't do anything
		
		CreateNewAlarm(); //Enable Alarm
		if (SHOW_DEBUG) Serial.println("[IO] Bouton Physique \"AUTO/MANU\" a changé ! Etat : AUTOMATIQUE");
		appendStrToFile("[IO] Physical Button1 \"AUTO/MANU\" has been toggle! Now : AUTOMATIC");
	} else if (resultBP == 0) { //Falling Edge (Manu)
		gBPModeManu = true;
		gMemBPFORCEON_OFF = !digitalRead(BP2_FORCEON_OFF); //reset the memory to update the state of circuit 1
		
		Alarm.free(gMyAlarmID); //disable Alarm
		if (SHOW_DEBUG) Serial.println("[IO] Bouton Physique \"AUTO/MANU\" a changé ! Etat : MANUEL");
		appendStrToFile("[IO] Physical Button1 \"AUTO/MANU\" has been toggle! Now : MANUAL");
	}

	//If button1 is "Manu"
	if (gBPModeManu) {
		//Check : State of button2 BP2_FORCEON_OFF
		resultBP = testBP(BP2_FORCEON_OFF, false, &gMemBPFORCEON_OFF); //0 is OFF, 1 is ON
		if (resultBP == 1) { //Raising Edge (Force ON)
			String test = ToggleCircuitState(1, true);
			appendStrToFile("DABDAB ON: " + test);
			if (SHOW_DEBUG) Serial.println("[IO] Physical Button2 \"Force\" is now : ON");
			appendStrToFile("[IO] Physical Button2 \"FORCE STATE\" has been toggle! Now : FORCE ON");
		} else if (resultBP == 0) { //Falling Edge (Force OFF)
			String test = ToggleCircuitState(1, false);
			appendStrToFile("DABDAB OFF : " + test);
			if (SHOW_DEBUG) Serial.println("[IO] Physical Button2 \"Force\" is now : OFF");
			appendStrToFile("[IO] Physical Button2 \"FORCE STATE\" has been toggle! Now : FORCE OFF");
		}
	}

	//In AUTO Mode, check if an alarm has reached the threshold, and execute the Handle
	if (gModeAuto) Alarm.delay(1000);
	else delay(1000);

	/* //display of temperature
	//Temperature acquisition (LM61 sensor)
	float voltage = analogRead(A0) * (3.3/1023.0);
	float temp = (voltage - 0.6) * 100.0;
	if (SHOW_DEBUG) Serial.println("Temperature : " + String(temp));
	delay(5000);
	*/
}

/**
 * @brief test for raising/falling edge for a pin of the ESP8266
 * 
 * @param BPPin pin to test
 * @param invertInput read the opposite of the pin
 * @param MemBP a global boolean to store the previous state of button

 * @return -1 if no change  
 * @return 0 for a falling edge
 * @return 1 for a raising edge
 */
int8_t testBP(uint8_t BPPin, bool invertInput, bool *memBP) {

	//Inversion of input if needed
	bool BPpos = digitalRead(BPPin) xor invertInput;

	if (BPpos and not *memBP) {
		*memBP = true;
		return 1;
	} else if (not BPpos and *memBP) {
		*memBP = false;
		return 0;
	}
	return -1;
}