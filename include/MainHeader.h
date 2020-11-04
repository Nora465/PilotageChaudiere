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

	#include <LittleFS.h>			//Accès au système de fichier
	#include <ArduinoJson.h>
	
	#include "DailySchedule.h"		//Gestion de la programmation journalière

//------------ DEFINITIONS DES PINS / VARIABLES ----------------------------------------------------
	#define CAPT_TEMP A0
	#define USB_D_plus D1 //SCL (I2C) (fil blanc)
	#define USB_D_moins D2 //SDA (I2C) (fil vert)
	#define BP1_AUTOMANU D3
	#define BP2_FORCEON_OFF D4
	#define LED_CC1 D7 //Circuit "Radiateurs"
	#define LED_CC2 D8 //Circuit "Plancher Chauffant"
	#define RELAY_CC1 D5
	#define RELAY_CC2 D6

//------------- Prototypes de Fonctions ------------------------------------------------------------
	void HandleForceState(AsyncWebServerRequest *request, bool states[2]);
	void HandleGetState(AsyncWebServerRequest *request, bool states[2]);
	void HandleChangeMode(AsyncWebServerRequest *request, bool gModeAuto);

	void ConnectToAP();
	void SetPinsMode();
	bool ToggleCircuitState(uint8_t circuit, uint8_t state);
  
	void StartLittleFS();
	String formatBytes(size_t bytes);

	Schedule_OneWeek ReadSchedule();
	void WriteSchedule(Schedule_OneWeek schedule);

#endif