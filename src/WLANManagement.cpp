/* Manage the WLAN part :
 * WiFi (Connexion)
 * Web Server (Handles)
 *
 * Creation Date : 24/08/2020
 * Creator : Nora465
*/

//TODO faire une fonction de validation (pour toutes les handles), pour éviter la répétitivité
#include "MainHeader.h"

extern ScheduleDay gSchedule[6];
extern AlarmID_t gMyAlarmID;
extern bool gStates[2];
extern bool gModeAuto;
extern uint8_t gCurRange;

/**
 * Connect to the last wifi saved in WiFiManager (if there isn't, create an AP to connect)
 */
void ConnectToAP() {

	WiFiManager wifiManager;
	//wifiManager.resetSettings(); //reset of the saved network (DEBUG&TEST ONLY)

	//Disable the WiFiManager Debug (too many outputs)
	wifiManager.setDebugOutput(false);

	//Set the captive portal       (IP					  , Gateway			  , Subnet Mask)
	wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

	//Callback : Execute itself when no network is saved (show the captive portal)
	wifiManager.setAPCallback([](WiFiManager *wiFiManager) {
		if (SHOW_DEBUG) Serial.print("\n[WiFi] Config \"Portail Captif\" en cours : " + String(wiFiManager->getConfigPortalSSID()));
	});

	//Attempt to connect to a registered network
	//If unsuccessful, creates an AP and asks the user to connect to a network (blocking task)
	if (SHOW_DEBUG) Serial.print("[WiFi] Connexion au réseau en cours...");
	wifiManager.autoConnect("PILOTAGE_CHAUDIERE");
	if (SHOW_DEBUG) Serial.println("\n[WiFi] Connexion réussie !");

	//Display the network name and the @IP of the ESP
	if (SHOW_DEBUG) Serial.println();
	if (SHOW_DEBUG) Serial.print("[WiFi] Connecté à \"");
	if (SHOW_DEBUG) Serial.print(WiFi.SSID()); Serial.println("\"");
	if (SHOW_DEBUG) Serial.print("[WiFi] @IP : ");
	if (SHOW_DEBUG) Serial.println(WiFi.localIP());
}

/**
 * ESP ==> AndroidApp : Send the States to the WebClient
 * @param request *AsyncWebServerRequest - Reference to the request of a client
 */
void HandleGetState(AsyncWebServerRequest *request) { //URI : /GetStates

	//Send the value of the 2 circuits
	String stateValues = String(gStates[0]) + " " + String(gStates[1]);
	if (SHOW_DEBUG) Serial.println("[Web-getStates] OK : States have been sent to WebClient : " + stateValues);
	request->send(200, "text/plain", stateValues);
}

/**
 * AndroidApp ==> ESP : Force the state of a relay (only if in mode "Manual") by a WebClient
 * @param request *AsyncWebServerRequest - Reference to the request of a client
 */
void HandleForceState(AsyncWebServerRequest *request) { //URI : /ForceState?circuit=(1or2)&state=(0or1)

	//Check : Presence of parameters "circuit" and "state"
	if (!request->hasParam("circuit") || !request->hasParam("state")) {
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] Err : Missing GET Argument(s)");
		return request->send(400, "text/plain", "Erreur : Il manque un ou des paramètre(s) GET");
	}

	//Check : Validity of the value of the Param "state" (must be 0 or 1)
	uint8_t stateValue = request->getParam("state")->value().toInt();
	if (stateValue != 0 && stateValue != 1) {
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] Err : Bad Value for \"state\" param (0or1)");
		return request->send(400, "text/plain", "Erreur : Mauvaise valeur pour le paramètre \"state\" (0 ou 1)");
	}
	
	//Check : Validity of the value of the Param "circuit" (must be 1 or 2)
	uint8_t circuitNumber = request->getParam("circuit")->value().toInt();
	if (circuitNumber != 1 && circuitNumber != 2) {
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] Err : Bad Value for \"circuit\" param (1or2)");
		return request->send(400, "text/plain", "Erreur : Mauvaise valeur pour le paramètre \"circuit\" (1 ou 2)");
	}

	//Change the selected output with the new value
	bool circuitState = (stateValue == 1);
	bool success = ToggleCircuitState(circuitNumber, circuitState);

	if (!success) {
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] Err : Couldn't change the state (already in this state ? physical button ?)");
		return request->send(400, "text/plain", "Erreur : le bouton \"mode\" est peut etre activé, ou le relai est déjà dans cette position");
	} else {
		//send states to WebClient
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] OK : New State for CC" + String(circuitNumber) + " : " + ((gStates[circuitNumber - 1]) ? "Enabled" : "Disabled"));
		return request->send(200, "text/plain", String(gStates[0]) + " " + String(gStates[1]));
	}
}

/**
 * ESP ==> AndroidApp : Send the current Mode of the ESP
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleGetMode(AsyncWebServerRequest *request) { //URI : /GetMode
	//FIXME Quand je démarre l'application android, il affiche uniquement "AUTO" dans le serial
	if (SHOW_DEBUG) Serial.println("[Web-getMode] OK : Mode have been sent to the client : " + gModeAuto? "AUTO": "MANU");
	request->send(200, "text/plain", String(gModeAuto));
}

/**
 * AndroidApp ==> ESP : Change the Mode, between MANUal(0) and AUTOmatic(1)
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleSetMode(AsyncWebServerRequest *request) { //URI : /SetMode?mode=(0or1)
	//Check: Presence of the GET parameter "mode"
	if (!request->hasParam("mode")) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : Missing GET Argument : \"mode\" !");
		return request->send(400, "text/plain", "Erreur : Argument \"mode\" manquant !");
	}

	//Check: Validity of the "mode" GET parameter value (must be 0 or 1)
	uint8_t modeValue = request->getParam("mode")->value().toInt();
	if (modeValue != 0 && modeValue != 1) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : Bad Value for \"mode\" param (0or1) !");
		return request->send(400, "text/plain", "Erreur : Mauvaise valeur pour le param \"mode\" (0 ou 1) !");
	}

	//Check: Physical Button "Mode" is in AUTO position
	if (!digitalRead(BP1_AUTOMANU)) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : Can't change the mode (Physical SW enabled) !");
		return request->send(400, "text/plain", "Impossible de changer le mode : Le Mode MANU est forcé physiquement !");
	}

	//Check: Value of "mode" is different from the actual mode
	if (gModeAuto == (modeValue)) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : The New Mode is the same as the actual one !");
		return request->send(400, "text/plain", "Impossible de changer le mode : Le mode est le même que l'actuel !");
	}

	//Store new mode in global variable
	gModeAuto = (modeValue == 1);
	if (gModeAuto) 	CreateNewAlarm(); //if auto mode : create new alarm
	else 			Alarm.free(gMyAlarmID); //if manu mode : Delete the alarm

	if (SHOW_DEBUG) Serial.println("[Web-ChgMode] OK : Mode is now : " + String(gModeAuto? "AUTO": "MANU"));
	request->send(200, "text/plain", "Mode Changé : " + String(gModeAuto? "AUTO": "MANU"));	
}

/**
 * ESP ==> AndroidApp : Send the current Range (1:Work // 2:Holiday) of the ESP
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleGetRange(AsyncWebServerRequest *request) {
	//FIXME Quand je démarre l'application android, il affiche uniquement "Work(1)" dans le serial
	if (SHOW_DEBUG) Serial.println("[Web-getRange] OK : Range have been sent to the client : " + (gCurRange==1)? "Work(1)": "Holiday(2)");
	request->send(200, "text/plain", String(gCurRange));
}

/**
 * AndroidApp ==> ESP : Change the Range, between Work (1) and Holiday (2)
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleSetRange(AsyncWebServerRequest *request) {
	//Check: Presence of the GET parameter "range"
	if (!request->hasParam("range")) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgRange] Err : Missing GET Argument : \"range\" !");
		return request->send(400, "text/plain", "Erreur : Argument \"range\" manquant !");
	}

	//Check: Validity of the "range" GET parameter value (must be 1 or 2)
	uint8_t RangeValue = request->getParam("range")->value().toInt();
	if (RangeValue != 1 && RangeValue != 2) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgRange] Err : Bad Value for \"range\" param (1or2) !");
		return request->send(400, "text/plain", "Erreur : Mauvaise valeur pour le param \"range\" (1 ou 2) !");
	}

	//Check: Value of "range" is different from the actual one
	if (gCurRange == RangeValue) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgRange] Err : The New Range is the same as the actual one !");
		return request->send(400, "text/plain", "Impossible de changer la plage : La plage est la même que l'actuelle !");
	}

	//Store new range in global variable
	gCurRange = RangeValue;

	if (SHOW_DEBUG) Serial.println("[Web-ChgRange] OK : Range is now : " + String((gCurRange==1)? "Work(1)": "Holiday(2)"));
	request->send(200, "text/plain", "Plage Changée : " + String((gCurRange==1)? "Work(1)": "Holiday(2)"));

	CreateNewAlarm(); //Create new alarm, with the new range
}

/**
 * ESP ==> AndroidApp : Send the Schedule for the requested Range to the web client
 * @param request *AsyncWebServerRequest - reference to the request of a client
 * @param schedule ScheduleDay[6] - Reference to the schedule, stored in a structure
 * @param showFullWeek bool[2] - Reference to how we display the schedule on phone (the 7 days of the week, or the work week and the weekend)
 */
void HandleGetSchedule(AsyncWebServerRequest *request, bool showFullWeek[2]) { //URI : /GetSchedule?range=(1 ou 2)
	//Representation of JSON document if FullWeek     : ["Lun1", "Lun2", "Mar1", "Mar2", ...., "Dim1", "Dim2"]
	//Representation of JSON document if Week+Weekend : ["Week1", "Week2", "WE1", "WE2"]

	//check : Presence of GET parameters "range"
	if (!request->hasParam("range")) {
		if (SHOW_DEBUG) Serial.println("[Web-sendSched] Err : Missing GET Argument : \"range\"");
		return request->send(400, "text/plain", "Erreur : Le paramètre GET \"range\" n'existe pas !");
	}

	//Check : Validity of the value of the Param "range" (must be 1 or 2)
	uint8_t paramRange = request->getParam("range")->value().toInt(); //atoi: convert Str to Int
	if (paramRange != 1 && paramRange != 2) {
		if (SHOW_DEBUG) Serial.println("[Web-sendSched] Err : Bad Value for \"range\" param (1or2)");
		return request->send(400, "text/plain", "Erreur : Le paramètre GET \"range\" doit être 1 ou 2");
	}
	
	//Adding data to the JSON string
	String jsonString;

	if (showFullWeek[paramRange - 1]) {//FullWeek : we send all the schedule to the WebClient
		for (uint8_t i= 0; i < 7; i++) { 
			//Adding schedule data to the JSON
			if 		(paramRange == 1) 	jsonString += String(gSchedule[i].P1Start) + "," + String(gSchedule[i].P1Stop);
			else if (paramRange == 2) 	jsonString += String(gSchedule[i].P2Start) + "," + String(gSchedule[i].P2Stop);
			
			if (i < 6) jsonString += ","; //don't put a comma at the end
		}
	} else { //Week+Weekend : we only send the monday values 5 times, and Saturday 2 times
		if 		(paramRange == 1) 	jsonString += String(gSchedule[0].P1Start) + "," + String(gSchedule[0].P1Stop)  + ","
												+ String(gSchedule[5].P1Start) + "," + String(gSchedule[5].P1Stop);
		else if (paramRange == 2) 	jsonString += String(gSchedule[0].P2Start) + "," + String(gSchedule[0].P2Stop)  + ","
												+ String(gSchedule[5].P2Start) + "," + String(gSchedule[5].P2Stop);
	}

	//send the JSON String to the Web Client (Android Application)
	if (SHOW_DEBUG) Serial.println("[Web-sendSched] Schedule of range " + String(paramRange) + " has been sent to WebClient");
	request->send(200, "text/plain", "[" + jsonString + "]");
}

/**
 * AndroidApp ==> ESP : Change the schedule of the specified range
 * @param request *AsyncWebServerRequest - reference to the request of a client
 * @param schedule ScheduleDay[6] - Reference to the schedule, stored in a structure
 * @param showFullWeek bool[2] - Reference to how we display the schedule on phone (the 7 days of the week, or the work week and the weekend)
 */
void HandleModifySchedule(AsyncWebServerRequest *request, bool showFullWeek[2]) { //URI : /ModifySchedule (data is POST)
	//Representation of JSON document if FullWeek    : ["Plage (1 ou 2)", "Lun1", "Lun2", "Mar1", "Mar2", ...., "Dim1", "Dim2"]
	//Representation of JSON document if Week+Weekend: ["Plage (1 ou 2)", "Week1", "Week2", "WE1", "WE2"]

	//Check : Presence of POST parameter "body"
	if (!request->hasParam("body", true)) {
		if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : Missing POST parameter \"body\"");
		return request->send(400, "text/plain", "Erreur : Aucune donnée dans le \"body\" (POST)!");
	}

	//Parse JSON doc
	const int capacity = JSON_ARRAY_SIZE(15) + 60; //array=15 (7 days * 2hours + 1 for the range) + 60 (strings size)
	StaticJsonDocument<capacity> doc;
	DeserializationError err = deserializeJson(doc, request->getParam("body", true)->value());
	
	//Check : Deserialisation of JSON string
	if (err) {
		if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : deserializeJson() has failed with code : " + String(err.c_str()));
		return request->send(400, "text/plain", "Erreur : Désérialisation du JSON !");
	}

	//Checks : the validity of hours (must be between 00H and 24H)
	for (uint8_t i= 1; i < doc.size(); i++) { //Offset of 1 (because the first value is the range)
		//Test : Between 0 et 23
		if (!((int)doc[i] >= 0 && (int)doc[i] <= 23)) {
			if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : one or more hours are not valid (between 0 and 23)");
			return request->send(400, "text/plain", "Erreur : Les heures doivent être entre 00H et 24H !");
		}
	}
	//Check : Validity of hours (Start Hour is less than the End Hour (and can't be the same))
	for (uint8_t i = 1; i < doc.size(); i+= 2) { //Increment by 2, because the hours come by pair
		if ((int)doc[i] >= (int)doc[i+1]) {
			if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : start hour must be strictly less than end hour (" 
											+ String((int)doc[i]) + " >= " + String((int)doc[i+1]) + ")");
			return request->send(400, "text/plain", "Erreur : Les heures de départ doivent être strictement inférieurs aux heures de fin ("
											+ String((int)doc[i]) + " >= " + String((int)doc[i+1]) + ")");
		}
	}

	//save the method to display the schedule (doc[0] is the selected range)
	showFullWeek[(int)doc[0] - 1] = (doc.size() == 15); //size=15 OR 5
	
	//Put the new schedule in the structure
	uint8_t startHourIndex = 0;
	for (uint8_t i= 0; i < 7; i++) {
		//Computing the index of the array from the android application
		if (showFullWeek[(int)doc[0] - 1]) {//show FullWeek
							startHourIndex= 1 + 2*i; //1 (offset for the Range)+ 2*i (a 14 array go into a 7 array, so i*2)
		} else {//show Week+Weekend
			//5 first day is the "Week" and the 2 last is the "WeekEnd"
			if (i < 5) 		startHourIndex= 1; //+1 (offset for the Range)
			else 			startHourIndex= 3; 
		}
		
		//Populating the Schedule Structure
		if (doc[0] == "1") { //doc[0] is the range
			gSchedule[i].P1Start = doc[startHourIndex]; 
			gSchedule[i].P1Stop  = doc[startHourIndex + 1]; //+1 (offset for the Stop Hour (that comes just after the start hour))
		} else if (doc[0] == "2") {
			gSchedule[i].P2Start = doc[startHourIndex];
			gSchedule[i].P2Stop  = doc[startHourIndex + 1]; //+1 (offset for the Stop Hour (that comes just after the start hour))
		}
	}

	//Write the schedule structure in the EEPROM
	WriteScheduleToEEPROM(gSchedule);

	CreateNewAlarm();

	if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] New Schedule for range " + String((int)doc[0]) + " has been writed !");
	request->send(200, "text/plain", "Planning Horaire Ecrit !");
}

//========================================= TEMPORAIRE ! ================================
//temp function to test the time schedule
//TODO à supprimer si tests finis
void HandledabSchedule(AsyncWebServerRequest *request) {
	uint8_t myHour = request->getParam("hour")->value().toInt();
	uint8_t myMinute = request->getParam("minute")->value().toInt();
	uint8_t mySecond = request->getParam("second")->value().toInt();
	uint8_t myDay = request->getParam("day")->value().toInt();

	setTime(myHour, myMinute, mySecond, myDay,1,2021);
	Alarm.disable(gMyAlarmID);
	Alarm.enable(gMyAlarmID);
	Serial.println("TIME IS NOW (day=" + String(day()) + "): " + String(hour()) + " H "+ String(minute()) + " MIN " + String(second())+" SEC");
	request->send(200, "text/plain", "OK");

}