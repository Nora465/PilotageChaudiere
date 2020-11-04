/*
 * MAIN.cpp (Entry Point)
 * Date de création : 24/08/2020
 * Créateur : Nora465
*/

#include "MainHeader.h"

//------------- DECLARATIONS-GLOBALES -----------------------------------------------------
AsyncWebServer	server(50500);	// Créé un objet "server" à partir de la lib "WebServer" qui écoute les requêtes sur le port 80
//bool gCC1State, gCC2State = false; 	//a modifier par un fichier du SPIFFS
bool gStates[2] = {true, true}; //Etats des CIRCUITS (pas des relais)
bool gModeAuto = true; //Mode de gestion des circuits (true: Mode AUTO // false: Mode MANU) TELEPHONE OU PHYSIQUE
Schedule_OneWeek schedule;
//---------------------------------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);
	Serial.println();

	//Serial.println(ESP.getFreeSketchSpace());

	//----------------------- Gestion_Pins ---------------------------------------
	SetPinsMode(); //Gestion de l'attribution des pins (IN/OUT, état des sorties par défaut)

	//------------------ ----- Gestion_WiFi ---------------------------------------
	ConnectToAP(); //Connexion WiFi en utilisant WiFiManager (portail captif)

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

	server.begin();
	Serial.println("[WebServer] Server HTTP DEMARRE !\n");
}

void loop() {
	/*
	//Récupération de la température (capteur LM61)
	float tension = analogRead(A0) * (3.3/1023.0);
	float temp = (tension - 0.6) * 100.0;
	Serial.println("Temp : " + String(temp));
	delay(5000);
	*/
}