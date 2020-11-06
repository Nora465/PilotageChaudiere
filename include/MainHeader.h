/* Header File, contains : 
 * Libraries, and others hand-made header file
 * Declarations of Instances//Prototypes
 *
 * Creation Date : 24/08/2020 12h54
 * Creator : Nora465
*/

#ifndef MAINHEADER_H
	#define MAINHEADER_H

//-------------- LIBRARIES ---------------------------------------------------------------
	#include <Arduino.h>

	//WiFi and WebServer Libraries
	#include <ESP8266WiFi.h>        //WiFi pour ESP8266
	#define WEBSERVER_H				//empêche l'erreur de double déf dans <ESPAsyncServer> -- voir https://github.com/me-no-dev/ESPAsyncWebServer/issues/418#issuecomment-667976368
	#include <ESPAsyncWebServer.h>  //Asynchronous WebServer

	//Time-Related Libraries
	#include <WiFiUdp.h>			//UDP Lib for NTPClient
	#include <NTPClient.h>			//Retreive the Network Time

	//Storage-Related Libraries
	#include <LittleFS.h>			//Access to the File System (SPIFFS is outdated)
	#include <ArduinoJson.h>		//Json
	
	#include "DailySchedule.h"		//Gestion de la programmation journalière

//------------ DEFINITIONS DES PINS / VARIABLES ----------------------------------------------------
	#define TEMP_SENSOR A0
	#define USB_D_PLUS D1 //SCL (I2C) (fil blanc)
	#define USB_D_MINUS D2 //SDA (I2C) (fil vert)
	#define BP1_AUTOMANU D3
	#define BP2_FORCEON_OFF D4
	#define LED_CC1 D7 //Circuit "Radiateurs"
	#define LED_CC2 D8 //Circuit "Plancher Chauffant"
	#define RELAY_CC1 D5
	#define RELAY_CC2 D6

//------------- Prototypes de Fonctions ------------------------------------------------------------
	//WLANManagement.cpp
	void ConnectToAP();
	void HandleForceState(AsyncWebServerRequest *request, bool states[2]);
	void HandleGetState(AsyncWebServerRequest *request, bool states[2]);
	void HandleChangeMode(AsyncWebServerRequest *request, bool gModeAuto);

	//IOPinsManagement.cpp
	void SetPinsMode();
	bool ToggleCircuitState(uint8_t circuit, uint8_t state);
	
	//FSManagement.cpp
	void 				StartLittleFS();
	String 				formatBytes(size_t bytes);
	Schedule_OneWeek 	ReadSchedule();
	void 				WriteSchedule(Schedule_OneWeek schedule);

	//TimeManagement.cpp
	void StartNTPClient(NTPClient &timeClient);

#endif