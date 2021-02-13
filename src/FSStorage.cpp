/* 
 * Use the File System to store :
 * Logs (Pushed BPs, event of turning ON and OFF the circuits), ...
 *
 * Creation Date : 06/09/2020 - 22h09
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File

void startLittleFS() {
	if(LittleFS.begin()){ 
			Serial.println("[LittleFS] OK : LittleFS is mounted !");
	} else 	Serial.println("[LittleFS] Err : while mounting LittleFS !");
}

void appendStrToFile(String str) {
	File file = LittleFS.open("/log.txt", "a");
	if (!file) {
		Serial.println("error with the file \"log.txt\"");
		return;
	}
	String curDate = String(day()) + "/" +String(month()) + "/" + String(year()) + "--";
	String curHour = String(hour()) + ":"+String(minute()) + ":" + String(second());

	file.println("[[" + curDate + curHour + "]]  " +  str); //print to file
	
	file.close();
}

bool DeleteLogFile() {
	return LittleFS.remove("/log.txt");
}
/*
//////Read a file
	
	//On démarre la mémoire LittleFS (et on affiche l'état)
	if(LittleFS.begin()) Serial.println("LittleFS ISOK");
	else Serial.println("LittleFS NOK");

	//Affichage du contenu actuel du LittleFS
	Serial.println("LittleFS started. Contents:");

	Dir dir = LittleFS.openDir("/");
	while (dir.next()) {
		Serial.println("FS File : " + dir.fileName() + ", size : " + formatBytes(dir.fileSize()));

		String fileName = dir.fileName();
		size_t fileSize = dir.fileSize();
		Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
		*//*
	}
    Serial.printf("\n");

*/


//Retourne le nombre d'octet (donné en argument), formaté en Ko, Mo, et Go
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "o";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "Ko";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "Mo";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "Go";
  }
}


/* Lit le fichier "Schedule.json"
 * @return <Schedule_OneWeek> une structure contenant la prog horaire de la semaine
*//*
void ReadSchedule(String fileName) {
	const uint8_t JSONCapacity = 5 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5); //5j /semaine (4 données)
	//String fileName = "Schedule.json";

	//Vérification de la présence du fichier
	File file = LittleFS.open(fileName, "r");
	
	if (!file) {
		Serial.println("Fichier Schedule.json absent");
	} else {
		//Conversion du contenu du fichier en documentJSON
		File file = LittleFS.open(fileName, "r");

		DynamicJsonDocument docSchedule(JSONCapacity);
		DeserializationError err = deserializeJson(docSchedule, file);
		if (err) {
			Serial.print("deserializeJson() failed: ");
			Serial.println(err.c_str());
		}

		//Affectation des valeurs 


		file.close();
	}
}
*/
/* Ecrit le fichier "Schedule.json"
 * @param <Schedule_OneWeek> une structure contenant la prog horaire de la semaine
*//*
void WriteSchedule(ScheduleWeek schedule) {} //CREER LE FICHIER SI INEXISTANT !!
*/