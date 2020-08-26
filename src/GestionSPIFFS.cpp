/*#include "MainHeader.h"

void StartSPIFFS()
{
	//On démarre la mémoire SPIFFS (et on affiche l'état)
	if(SPIFFS.begin()) Serial.println("SPIFFS ISOK");
	else Serial.println("SPIFFS NOK");

	//Affichage du contenu actuel du SPIFFS
	Serial.println("SPIFFS started. Contents:");

	Dir dir = SPIFFS.openDir("/");
	while (dir.next())
	{
		String fileName = dir.fileName();
		size_t fileSize = dir.fileSize();
		Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
	}
    Serial.printf("\n");
}*/