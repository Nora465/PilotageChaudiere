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
	#include <ESP8266WiFi.h>        //WiFi for ESP8266
	#include <WiFiManager.h>		//(DONOT REMOVE FROM HERE:Creates an error) Connect to WiFi AP with a captive portal
	#define WEBSERVER_H				//Prevent the error of double definition in <ESPAsyncServer> -- see https://github.com/me-no-dev/ESPAsyncWebServer/issues/418#issuecomment-667976368
	#include <ESPAsyncWebServer.h>  //Asynchronous WebServer

	//Time-Related Libraries
	#include <WiFiUdp.h>			//UDP Lib for NTPClient
	#include <NTPClient.h>			//Retreive the Network Time
	#include <TimeLib.h>			//Time Lib, byArduino
	#include <TimeAlarms.h>			//

	//Storage-Related Libraries
	#include <LittleFS.h>			//Access to the File System (SPIFFS is outdated)
	#include <ArduinoJson.h>		//Json

	#include "DailySchedule.h"		//Schedule Struct

//------------ DEFINITIONS DES PINS / VARIABLES ----------------------------------------------------
	#define TEMP_SENSOR A0
	//DON'T USE D0 : Dead IO ? Internal malfunctioning ?
	#define USB_D_PLUS D1 //SCL (I2C) (fil blanc)
	#define USB_D_MINUS D2 //SDA (I2C) (fil vert)
	#define BP1_AUTOMANU D3
	#define BP2_FORCEON_OFF D4
	#define LED_CC1 D7 //Circuit "Radiateurs"
	#define LED_CC2 D8 //Circuit "Plancher Chauffant"
	#define RELAY_CC1 D5
	#define RELAY_CC2 D6

	#define TIME_OFFSET_S 3600 //France : UTC+1 (3600 seconds)
	#define TIME_UPDATE_INTERVAL 1 * 24 * 60 * 60 * 1000 //1 Day

//------------- Functions Prototypes ----------------------------------------------------------------------
	//WLANManagement.cpp
	void ConnectToAP();
	void HandleForceState(AsyncWebServerRequest *request, bool states[2]);
	void HandleGetState(AsyncWebServerRequest *request, bool states[2]);
	void HandleChangeMode(AsyncWebServerRequest *request, bool gModeAuto);

	//IOPinsManagement.cpp
	void SetPinsMode();
	bool ToggleCircuitState(uint8_t circuit, bool state);
	
	//FSManagement.cpp
	void 				StartLittleFS();
	String 				formatBytes(size_t bytes);
	Schedule_OneWeek 	ReadSchedule();
	void 				WriteSchedule(Schedule_OneWeek schedule);

	//TimeManagement.cpp
	void StartNTPClient(NTPClient &timeClient);
	void TryToUpdateTime(NTPClient &timeClient, bool forceUpdate= false);

#endif