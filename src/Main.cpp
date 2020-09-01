/*
 *
 * Date de création : 24/08/2020
 * Créateur : Nora465
*/

#include "MainHeader.h"

//------------- DECLARATION-INSTANCES-ET-OBJETS -----------------------------------------------------
AsyncWebServer	server(50500);	// Créé un objet "server" à partir de la lib "WebServer" qui écoute les requêtes sur le port 80
//bool gCC1State, gCC2State = false; 	//a modifier par un fichier du SPIFFS
bool gStates[2] = {true, true}; //Etats des CIRCUITS (pas des relais)
//---------------------------------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);
	Serial.println();

	//Serial.println(ESP.getFreeSketchSpace());

	//----------------------- Gestion_Pins ---------------------------------------
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);
	pinMode(LED_CC1, OUTPUT);
	pinMode(LED_CC2, OUTPUT);
	pinMode(RELAY_CC1, OUTPUT);
	pinMode(RELAY_CC2, OUTPUT);

	digitalWrite(LED_CC1, LOW);
	digitalWrite(LED_CC2, LOW);
	digitalWrite(RELAY_CC1, LOW);
	digitalWrite(RELAY_CC2, LOW);

	//----------------------- Gestion_WiFi ---------------------------------------
	
	ConnectToAP(); //Connexion en utilisant WiFiManager (portail captif)

	//------------- HANDLE WEB ---------------------------------------------------

	//StartServerHandles();
	server.on("/ForceState", [](AsyncWebServerRequest *request) {
		handleForceState(request, gStates);
	});

	server.on("/GetStates", [](AsyncWebServerRequest *request) {
		handleGetState(request, gStates);
	});

	server.begin();
	Serial.println("[WebServer] Server HTTP DEMARRE !\n");
}

void loop() {}