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
	#include <Timezone.h>			//Timezones (Also manage the DST)

	//Storage-Related Libraries
	#include <EEPROM.h>				//Access EEPROM (store the schedule)
	#include <LittleFS.h>			//Access to the File System (SPIFFS is outdated)
	#include <ArduinoJson.h>		//Json

	#include "DailySchedule.h"		//Schedule Struct

//------------ DEFINITIONS DES PINS / VARIABLES ----------------------------------------------------
	#define TEMP_SENSOR A0
	//DON'T USE D0 : Dead IO ? Internal malfunctioning ?
	#define USB_D_PLUS D1 //SCL (I2C) (fil blanc)
	#define USB_D_MINUS D2 //SDA (I2C) (fil vert)
	#define RELAY_CC1 D3
	#define RELAY_CC2 D4
	#define BP1_AUTOMANU D5
	#define BP2_FORCEON_OFF D6
	#define LED_CC1 D7 //Circuit "Radiateurs"
	#define LED_CC2 D8 //Circuit "Plancher Chauffant"

	#define NUM_OF_PERIODS 2
	
	#define TIME_UPDATE_INTERVAL 1 * 24 * 60 * 60 * 1000 //1 Day

	#define SHOW_DEBUG true //Show the Serial.println() in the console (Useless when in production)

//------------- Functions Prototypes ----------------------------------------------------------------------
	//WLANManagement.cpp
	void ConnectToAP();
	void HandleGetState(AsyncWebServerRequest *request);
	void HandleForceState(AsyncWebServerRequest *request);
	void HandleGetMode(AsyncWebServerRequest *request);
	void HandleSetMode(AsyncWebServerRequest *request);
	void HandleGetRange(AsyncWebServerRequest *request);
	void HandleSetRange(AsyncWebServerRequest *request);
	void HandleGetSchedule(AsyncWebServerRequest *request);
	void HandleModifySchedule(AsyncWebServerRequest *request);
	//IOPinsManagement.cpp
	void SetPinsMode();
	String ToggleCircuitState(uint8_t circuit, bool state);
	
	//FSManagement.cpp
	void appendStrToFile(String str);
	bool DeleteLogFile();
	String FullDate();
	String TwoDigit(int someDigit);
	//String 		formatBytes(size_t bytes);
	//ScheduleWeek 	ReadSchedule();
	//void 			WriteSchedule(ScheduleWeek schedule);

	//ScheduleStorage.cpp
	bool LoadEEPROMSchedule(ScheduleDay schedule[7], bool displayValues= false);
	void WriteScheduleToEEPROM(ScheduleDay schedule[7]);

	//TimeManagement.cpp
	void StartNTPClient(NTPClient &timeClient);
	void TryToUpdateTime(NTPClient &timeClient, bool forceUpdate= false);
	uint8_t GetNonRetardDay();
	void CreateNewAlarm();
	void TimeHandle();

	//TEMP/TEST Handle/functions
	void startLittleFS();
	
	//Main.cpp
	int8_t testBP(uint8_t BPPin, bool invertInput, bool *memBP);
#endif