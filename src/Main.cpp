/*
 *
 * Date de création : 24/08/2020
 * Créateur : Nora465
*/

#include "MainHeader.h"

//------------- DECLARATION-INSTANCES-ET-OBJETS -----------------------------------------------------
AsyncWebServer	server(80);	// Créé un objet "server" à partir de la lib "WebServer" qui écoute les requêtes sur le port 80
bool gRelayState = false; 	//a modifier par un fichier du SPIFFS
//---------------------------------------------------------------------------------------------------

void setup() {
	Serial.begin(115200);
	Serial.println();
	delay(1000); // safe-boot

	//Serial.println(ESP.getFreeSketchSpace());

	//----------------------- Gestion_Pins ---------------------------------------
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);
	pinMode(LED_CC1, OUTPUT);
	pinMode(LED_CC2, OUTPUT);
	digitalWrite(LED_CC1, LOW);
	digitalWrite(LED_CC2, LOW);
	pinMode(RELAY_CC1, OUTPUT);
	pinMode(RELAY_CC2, OUTPUT);

	//----------------------- Gestion_WiFi ---------------------------------------
	
	ConnectToAP();

	//------------- HANDLE WEB ---------------------------------------------------

	//StartServer();
	server.on("/relay", [](AsyncWebServerRequest *request) {
		gRelayState = handleModifyRelayChange(request);
	});

	server.on("/GetRelayState", [](AsyncWebServerRequest *request) {
		handleGetRelayState(request, gRelayState);
	});

	server.begin();
	Serial.println("[WebServer] Server HTTP DEMARRE !\n");
}

void loop() {}