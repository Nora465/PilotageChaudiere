/* 
 * Handle the writing of the schedule in the EEPROM
 * And the Reading of it
 * 
 * Creation Date : 06/09/2020 - 22h09
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File

/**
 * Start EEPROM, and load the daily-schedule stored inside
 * @param displayValues <bool> If you want to display the Schedule struct
 * @param schedule <ScheduleDay[7]> The array of struct where to write the EEPROM schedule
 */
void LoadEEPROMSchedule(ScheduleDay *schedule, bool displayValues) {

	EEPROM.begin(30);
	Serial.println("[EEPROM] Started ! Length : " + String(EEPROM.length()));
	
	//check the magic number at @0 (this number  if the data has been writed at least one time by this program)
	uint16_t eepromMagicNumber = (EEPROM.read(0) << 8) + EEPROM.read(1);
	
	bool dataError = (eepromMagicNumber != EEPROM_MAGICNUMBER);
	if (dataError) { //TODO change mode to manual (and wait for smartphone)
		//DISABLE THE AUTOMATIC MODE (and wait for a schedule update from the smartphone)
		//..
		schedule = {}; //initialize to zero
		Serial.println("[EEPROM] Erreur fichier config non initialisé");

		return; //est ce qu'on est obligé de retourner un "false" pour informer le main qu'on a un pb de données ?
	} else Serial.println("[EEPROM] Data are valids ! ");

	//Get the struct (offset : 2 because @0&1 are the magic number)
	for (uint8_t i=0; i < 7; i++) {
		EEPROM.get((sizeof(ScheduleDay) * i)+2, schedule[i]);
	}
	//affichage du contenu de l'eeprom
	/*for (uint8_t i = 0; i < EEPROM_LENGTH; i++) {
		Serial.println(String(i) + " " + String(EEPROM.read(i)));
	}*/

	//Affichage des valeurs (takes ~30ms)
	if (displayValues) {
		Serial.print("[EEPROM] Struct Readed : \n");
		
		for (uint8_t i= 0; i < 7; i++) {
			Serial.println(String(i+1) + "eme jour  P1 : " + String(schedule[i].P1Start) + "H-" + String(schedule[i].P1Stop) + "H");
			Serial.println(String(i+1) + "eme jour  P2 : " + String(schedule[i].P2Start) + "H-" + String(schedule[i].P2Stop) + "H");
		}
	}
	EEPROM.end();
}

void writeFirstThing() {
	return; //evitons d'écrire de facon inutile
	const uint16_t magicNumber = 29127; //Random

	EEPROM.begin(30);
	
	EEPROM.write(0, magicNumber >> 8);
	EEPROM.write(1, magicNumber & 0x00FF);

	for (int i= 2; i < 30; i++) {
		EEPROM.write(i, i-1);
		//Serial.println(String(i) + " " + String((i-1)));
	}
	
	EEPROM.end();
}

/**
 * Write the new Schedule inside the EEPROM memory
 * @param schedule <ScheduleDay[7]> The array of struct where to write the EEPROM schedule
 */
void WriteScheduleToEEPROM(ScheduleDay *schedule) {
	return;

	EEPROM.begin(30);
	
	EEPROM.write(0, EEPROM_MAGICNUMBER >> 8);
	EEPROM.write(1, EEPROM_MAGICNUMBER & 0xFF);

	for (uint8_t i=0; i < 7; i++) {
		EEPROM.put((sizeof(ScheduleDay) * i)+2, schedule[i]);
	}	
	EEPROM.end();
}