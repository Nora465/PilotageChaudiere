/* Fichiers header qui contient : 
 * les #include des librairies et autres headers perso
 * les déclarations des Instances//Objets//Prototypes
 *
 * Date de création : 24/08/2020 12h54
 * Créateur : Nora465
*/

#ifndef MAINHEADER_H
	#define MAINHEADER_H

//------------- INCLUSION-LIBRAIRIES ---------------------------------------------------------------
	#include <Arduino.h>

	#include <ESP8266WiFi.h>        //WiFi pour ESP8266
	
	#include <WiFiManager.h>		//permet de la configuration de la connexion WiFi sans mettre les identifiants dans le code
	#define WEBSERVER_H				//empêche l'erreur de double déf dans <ESPAsyncServer> -- voir https://github.com/me-no-dev/ESPAsyncWebServer/issues/418#issuecomment-667976368
	#include <ESPAsyncWebServer.h>  //WebServer Asynchrone

	//#include <FS.h>                 //SPIFFS
	//#include <ESP8266WebServer.h>   //lib WebServer Sync
	
	#include "DailySchedule.h"		//Gestion de la programmation journalière

//------------ DEFINITIONS DES PINS / VARIABLES ----------------------------------------------------
	#define SW1 D3
	#define SW2 D4
	#define LED_CC1 D7
	#define LED_CC2 D8
	#define RELAY_CC1 D5
	#define RELAY_CC2 D6

//------------- Prototypes de Fonctions ------------------------------------------------------------
	bool handleModifyRelayChange(AsyncWebServerRequest *request);
	void handleGetRelayState(AsyncWebServerRequest *request, bool CC1State);

	void ConnectToAP();
  
	//String formatBytes(size_t bytes);
	//void StartSPIFFS();


#endif