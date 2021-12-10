/* Manage the WLAN part :
 * WiFi (Connexion)
 * Web Server (Handles)
 *
 * Creation Date : 24/08/2020
 * Creator : Nora465
*/

//TODO faire une fonction de validation (pour toutes les handles), pour éviter la répétitivité
#include "MainHeader.h"

extern ScheduleDay gOldSchedule[7];
extern ScheduleDay gSchedule[7];
extern AlarmID_t gMyAlarmID;
extern bool gStates[2];
extern bool gModeAuto;
extern bool gBPModeManu;
extern uint8_t gCurRange;
extern bool gShowFullWeek[2];
extern bool gSchedIsInEEPROM[NUM_OF_PERIODS];

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
	if (SHOW_DEBUG) {
		Serial.println();
		Serial.print("[WiFi] Connecté à \"");
		Serial.print(WiFi.SSID()); Serial.println("\"");
		Serial.print("[WiFi] @IP : ");
		Serial.println(WiFi.localIP());
	}
}

/**
 * ESP ==> AndroidApp : Send the States to the WebClient
 * @param request *AsyncWebServerRequest - Reference to the request of a client
 */
void HandleGetState(AsyncWebServerRequest *request) { //URI : /GetStates

	//Send the state of the 2 circuits
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
	String success = ToggleCircuitState(circuitNumber, circuitState);

	if (success != "") {
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] Err : Couldn't change the state (already in this state ? physical button ?)");
		request->send(400, "text/plain", "Erreur : le bouton physique est peut etre activé, ou le relai est déjà dans cette position");
	} else {
		//send states to WebClient
		if (SHOW_DEBUG) Serial.println("[Web-ForcStat] OK : New State for CC" + String(circuitNumber) + " : " + ((gStates[circuitNumber - 1]) ? "Enabled" : "Disabled"));
		request->send(200, "text/plain", String(gStates[0]) + " " + String(gStates[1]));
		appendStrToFile("[IO] Etat force ! circuit " + String(circuitNumber) + " => " + String(circuitState) + " ("+request->client()->remoteIP().toString()+")");
	}
}

/**
 * ESP ==> AndroidApp : Send the current Mode of the ESP
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleGetMode(AsyncWebServerRequest *request) { //URI : /GetMode
	if (SHOW_DEBUG) Serial.println("[Web-getMode] OK : Mode have been sent to the client : " + String(gModeAuto? "AUTO": "MANU") + " et BP1: " + String(gBPModeManu?"MANU":"AUTO"));
	request->send(200, "text/plain", String(gBPModeManu) + " " + String(gModeAuto and !gBPModeManu));
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
	if (gBPModeManu) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : Can't change the mode (Physical Manual SW enabled) !");
		return request->send(400, "text/plain", "Impossible de changer le mode : Le Mode MANU est forcé physiquement !");
	}

	//Check: Value of "mode" is different from the actual mode
	if (gModeAuto == modeValue) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : The New Mode is the same as the actual one !");
		return request->send(400, "text/plain", "Impossible de changer le mode : Le mode est le même que l'actuel !");
	}

	//Check : Schedule must exist (for this current range) if the new mode is "AUTO"
	if (modeValue and !gSchedIsInEEPROM[gCurRange - 1]) {
		if (SHOW_DEBUG) Serial.println("[Web-ChgMode] Err : Schedule doesn't exist, can go to \"AUTO\" mode !");
		return request->send(400, "text/plain", "Impossible de passer en mode AUTO : Le planning n'a pas été configuré pour cette plage !");
	}

	//Store new mode in global variable
	gModeAuto = modeValue;
	if (gModeAuto) 	CreateNewAlarm(); //if auto mode : create new alarm
	else 			Alarm.free(gMyAlarmID); //if manu mode : Delete the alarm

	if (SHOW_DEBUG) Serial.println("[Web-ChgMode] OK : Mode is now : " + String(gModeAuto? "AUTO": "MANU"));
	appendStrToFile("[Mode] Mode has been changed : " + String(gModeAuto? "AUTO": "MANU") + "("+request->client()->remoteIP().toString()+")");
	request->send(200, "text/plain", "Mode Changé : " + String(gModeAuto? "AUTO": "MANU"));	
}

/**
 * ESP ==> AndroidApp : Send the current Range (1:Work // 2:Holiday) of the ESP
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleGetRange(AsyncWebServerRequest *request) {
	if (SHOW_DEBUG) Serial.println("[Web-getRange] OK : Range have been sent to the client : " + String(gCurRange == 1? "Work(1)": "Holiday(2)"));
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

	//Check : the schedule of the new Range has been configured
	if (gModeAuto and !gSchedIsInEEPROM[RangeValue - 1]) {
		gModeAuto = false; //mode is manu, because of the unconfigured schedule
		if (SHOW_DEBUG) Serial.println("[Web-ChgRange] Info : the schedule isn't configured for the new range (mode is now \"manu\")");
		return request->send(200, "text/plain", "Le planning n'existe pas pour cette plage \nPassage en mode manu");
		//TODO envoyer l'info au tel que le nouveau mode est MANU (intégrer un websocket ?)
		//Actuellement, l'appli renvoit un getMode à chaque échéance de SetRange en HTTP400
	}

	if (SHOW_DEBUG) Serial.println("[Web-ChgRange] OK : Range is now : " + String((gCurRange==1)? "Work(1)": "Holiday(2)"));
	request->send(200, "text/plain", "Plage Changée : " + String((gCurRange==1)? "Work(1)": "Holiday(2)"));

	if (gModeAuto) CreateNewAlarm(); //Create new alarm, with the new range
	appendStrToFile("[Plage] Plage changee :" + String((gCurRange==1)? "Work(1)": "Holiday(2)") + "("+request->client()->remoteIP().toString()+")");
}

/**
 * ESP ==> AndroidApp : Send the Schedule for the requested Range to the web client
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleGetSchedule(AsyncWebServerRequest *request) { //URI : /GetSchedule?range=(1 ou 2)

	//first element of array indicate the fullweek (or not)
	/*Representation of JSON document if FullWeek     : 
	[
		0 (FullWeek), 
		[Lun1Start, Lun1Stop, Lun2Start, Lun2Stop],
		[Mar1Start, Mar1Stop, Mar2Start, Mar2Stop],
		[Mer1Start, Mer1Stop, Mer2Start, Mer2Stop],
		[Jeu1Start, Jeu1Stop, Jeu2Start, Jeu2Stop],
		[Ven1Start, Ven1Stop, Ven2Start, Ven2Stop],
		[Sam1Start, Sam1Stop, Sam2Start, Sam2Stop],
		[Dim1Start, Dim1Stop, Dim2Start, Dim2Stop]
	]

	Representation of JSON document if Week+Weekend : 
	[
		1 (FullWeek), 
		[Week1Start, Week1Stop, Week2Start, Week2Stop],
		[WE1Start,   WE1Stop,   WE2Start,   WE2Stop]
	]
	*/
	//check : Presence of GET parameters "range"
	if (!request->hasParam("range")) {
		if (SHOW_DEBUG) Serial.println("[Web-sendSched] Err : Missing GET Argument : \"range\"");
		return request->send(400, "text/plain", "Erreur : Le paramètre GET \"range\" n'existe pas !");
	}

	//Check : Validity of the value of the Param "range" (must be 1 or 2)
	uint8_t paramRange = request->getParam("range")->value().toInt();
	if (paramRange != 1 && paramRange != 2) {
		if (SHOW_DEBUG) Serial.println("[Web-sendSched] Err : Bad Value for \"range\" param (1or2)");
		return request->send(400, "text/plain", "Erreur : Le paramètre GET \"range\" doit être 1 ou 2");
	}
	
	//Adding data to the JSON string
	String jsonString = String(!gShowFullWeek[paramRange - 1]) + ",";

	if (gShowFullWeek[paramRange - 1]) {//FullWeek : we send all the schedule to the WebClient
		for (uint8_t i=0; i < DAYS_PER_WEEK; i++) { 
			//Adding schedule data to the JSON

			jsonString += "[";
			for (uint8_t j=0; j < NUM_OF_PERIODS; j++) {
				jsonString += String(gSchedule[i].Ranges[paramRange-1].Periods[j].Start) + ',';
				jsonString += String(gSchedule[i].Ranges[paramRange-1].Periods[j].Stop);
				if (j < NUM_OF_PERIODS -1) jsonString += ',';
			}
			jsonString += "]";

			if (i < 6) jsonString += ","; //don't put a comma at the end
		}
	} else { //Week+Weekend : we only send the monday values 5 times, and Saturday 2 times

		jsonString += "[";
		for (uint8_t i=0; i < NUM_OF_PERIODS; i++) {
			jsonString += String(gSchedule[0].Ranges[paramRange-1].Periods[i].Start) + ',';
			jsonString += String(gSchedule[0].Ranges[paramRange-1].Periods[i].Stop);
			if (i < NUM_OF_PERIODS -1) jsonString += ',';
		}
		jsonString += "],[";
		for (uint8_t i=0; i < NUM_OF_PERIODS; i++) {
			jsonString += String(gSchedule[5].Ranges[paramRange-1].Periods[i].Start) + ',';
			jsonString += String(gSchedule[5].Ranges[paramRange-1].Periods[i].Stop);
			if (i < NUM_OF_PERIODS -1) jsonString += ',';
		}
		jsonString += "]";
	}
	Serial.println(jsonString);

	//send the JSON String to the Web Client (Android Application)
	if (SHOW_DEBUG) Serial.println("[Web-sendSched] Schedule of range " + String(paramRange) + " has been sent to WebClient");
	request->send(200, "text/plain", "[" + jsonString + "]");
}

/**
 * AndroidApp ==> ESP : Change the schedule of the specified range
 * @param request *AsyncWebServerRequest - reference to the request of a client
 */
void HandleModifySchedule(AsyncWebServerRequest *request) { //URI : /ModifySchedule (data is POST)
	/*Representation of JSON document if FullWeek     : 
	[1,1,[6,9,255,255],[17,23,255,255],[17,23,255,255],[17,23,255,255],[17,23,255,255],[17,23,255,255],[17,23,255,255]]
	[
		Plage (1 ou 2)
		0 (FullWeek), 
		[Lun1Start, Lun1Stop, Lun2Start, Lun2Stop],
		[Mar1Start, Mar1Stop, Mar2Start, Mar2Stop],
		[Mer1Start, Mer1Stop, Mer2Start, Mer2Stop],
		[Jeu1Start, Jeu1Stop, Jeu2Start, Jeu2Stop],
		[Ven1Start, Ven1Stop, Ven2Start, Ven2Stop],
		[Sam1Start, Sam1Stop, Sam2Start, Sam2Stop],
		[Dim1Start, Dim1Stop, Dim2Start, Dim2Stop]
	]
	[1,1,[6,9,17,23],[9,23,255,255]]
	Representation of JSON document if Week+Weekend : 
	[
		Plage (1 ou 2)
		1 (Week+Weekend), 
		[Week1Start, Week1Stop, Week2Start, Week2Stop],
		[WE1Start,   WE1Stop,   WE2Start,   WE2Stop]
	]
	*/

	//Check : Presence of POST parameter "body"
	if (!request->hasParam("body", true)) {
		if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : Missing POST parameter \"body\"");
		return request->send(400, "text/plain", "Erreur : Aucune donnee dans le \"body\" (POST)!");
	}

	//Parse JSON doc
	const int capacity = 7*JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(9); //(7days * (2Period * 2startStop) + 9 array size (in total)
	StaticJsonDocument<capacity> doc;
	DeserializationError err = deserializeJson(doc, request->getParam("body", true)->value());
	
	//Check : Deserialisation of JSON string
	if (err) {
		if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : deserializeJson() has failed with code : " + String(err.c_str()));
		return request->send(400, "text/plain", "Erreur : Deserialisation du JSON !");
	}

	//Check : the validity of hours (must be between 00H and 24H)
	for (uint8_t i= 2; i < doc.size(); i++) { //Offset of 1 (because the first value is the range)
		Serial.println(String((int)doc[i][0]) + " " + String((int)doc[i][1]) + " " + String((int)doc[i][2]) + " " + String((int)doc[i][3]));
		for (uint8_t j=0; j< 2 * NUM_OF_PERIODS; j++) {
			//Test : Between 0 et 23
			if (!((int)doc[i][j] >= 0 && (int)doc[i][j] <= 23)) {
				//if the hour is 255 AND is another period than the first => it's allowed
				if ((int)doc[i][j] == 255 and j < 2) {
					if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : one or more hours are not valid (between 0 and 23)");
					return request->send(400, "text/plain", "Erreur : Les heures doivent etre entre 00H et 24H !");
				}
			}
		}
	}

	//Check : Validity of hours (Hour X must be less than every following hour)
	for (uint8_t i = 2; i < doc.size(); i++) {
		if (((doc[i][0] >= doc[i][1] or doc[i][0] >= doc[i][2] or doc[i][0] >= doc[i][3]) and doc[i][0] != 255)
		 or ((doc[i][1] >= doc[i][2] or doc[i][1] >= doc[i][3]) and doc[i][1] != 255)
		 or ((doc[i][2] >= doc[i][3]) and doc[i][2] != 255)) {
			 if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] Err : All hours must be strictly less than the following hours");
			return request->send(400, "text/plain", "Erreur : Les heures doivent etre inférieure aux heures suivantes");
		}
	}

	uint8_t schedRange = (uint8_t) doc[0] - 1; //Range for the current schedule

	//save the method to display the schedule
	gShowFullWeek[schedRange] = ((int)doc[1] == 0); //doc[1] is FullWeek information from phone app

	//Save that a schedule had been configured for this range
	gSchedIsInEEPROM[schedRange] = true;

	//Put the new schedule in the structure
	for (uint8_t i= 0; i < DAYS_PER_WEEK; i++) {
		//Populating the schedule Structure
		for (uint8_t j = 0; j < NUM_OF_PERIODS; j++) {
			if (gShowFullWeek[schedRange]) { //show FullWeek
				gSchedule[i].Ranges[schedRange].Periods[j].Start = doc[2+i][2 *j];
				gSchedule[i].Ranges[schedRange].Periods[j].Stop  = doc[2+i][2 *j +1];
			} else { //show Week+Weekend
				//(i<5)?2:3 => 5 first days is the "week" and the last 2 are "weekend"
				gSchedule[i].Ranges[schedRange].Periods[j].Start = doc[(i<5) ?2 :3][2 *j];
				gSchedule[i].Ranges[schedRange].Periods[j].Stop  = doc[(i<5) ?2 :3][2 *j +1];
			}
		}
	}

	//Print the values of the read structure
	if (SHOW_DEBUG) {
		for (uint8_t i= 0; i < DAYS_PER_WEEK; i++) {
			Serial.println(String(i+1) + "eme jour  P1(1er période) : " + String(gSchedule[i].Ranges[0].Periods[0].Start) + "H-" + String(gSchedule[i].Ranges[0].Periods[0].Stop) + "H");
			Serial.println(String(i+1) + "eme jour  P1(2em période) : " + String(gSchedule[i].Ranges[0].Periods[1].Start) + "H-" + String(gSchedule[i].Ranges[0].Periods[1].Stop) + "H");
		}
		Serial.println("");
		for (uint8_t i= 0; i < DAYS_PER_WEEK; i++) {
			Serial.println(String(i+1) + "eme jour  P2(1er période) : " + String(gSchedule[i].Ranges[1].Periods[0].Start) + "H-" + String(gSchedule[i].Ranges[1].Periods[0].Stop) + "H");
			Serial.println(String(i+1) + "eme jour  P2(2em période) : " + String(gSchedule[i].Ranges[1].Periods[1].Start) + "H-" + String(gSchedule[i].Ranges[1].Periods[1].Stop) + "H");
		}
	}

	//Write the schedule structure in the EEPROM
	//WriteScheduleToEEPROM(gSchedule);

	if (gModeAuto) CreateNewAlarm();

	if (SHOW_DEBUG) Serial.println("[Web-GetNewSched] New Schedule for range " + String((int)doc[0]) + " has been writed !");
	appendStrToFile("[ProgHoraire] Programmation Modifiee par " + request->client()->remoteIP().toString());
	request->send(200, "text/plain", "Planning Horaire Ecrit !");
}