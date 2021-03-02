/* 
 * Handle the reading/writing of the schedule in the EEPROM
 * 
 * Creation Date : 06/09/2020 - 22h09
 * Creator : Nora465
*/

#include "MainHeader.h"		//Global Header File
extern bool gShowFullWeek[2];

/**
 * Write the schedule structure inside the EEPROM
 * @param schedule ScheduleDay[7] - The array of struct that store the schedule
 * @param useFullWeek bool[2] - Define how we display the schedule on phone (the 7 days of the week, or the work week and the weekend)
 * @param displayValues bool - If true, display the stored schedule
 * @return bool - if true, there is no error while loading schedule from EEPROM
 */
bool LoadEEPROMSchedule(ScheduleDay schedule[6], bool displayValues) {

	EEPROM.begin(EEPROM_LENGTH);
	if (SHOW_DEBUG) Serial.println("[EEPROM] Started ! Length : " + String(EEPROM.length()));
	
	//check the magic number at @0 (verify if the data has been writed at least one time by this program)
	uint16_t readMagicNumber = (EEPROM.read(0) << 8) + EEPROM.read(1);
	
	bool dataError = (readMagicNumber != EEPROM_MAGICNUMBER);
	if (dataError) {
		if (SHOW_DEBUG) Serial.println("[EEPROM] Erreur : données EEPROM non initialisé \nProgrammez la chaudière !");
		return false; //error
	} else if (SHOW_DEBUG) Serial.println("[EEPROM] Data are valids ! ");

	//how to display the schedule ON PHONE (the 7 days of the week, or the work week and the weekend)
	gShowFullWeek[0] = (EEPROM.read(2) & 0b00000010) >> 1; // 0000 00XY => useFullWeek[0] = X  ... [1] = Y
	gShowFullWeek[1] = (EEPROM.read(2) & 0b00000001);

	//Get the schedule (offset :3 because @0&1 are the magic number, and @2 is the type of schedule)
	for (uint8_t i=0; i < 7; i++) {
		EEPROM.get((sizeof(ScheduleDay) *i) +3, schedule[i]);
	}

	//affichage du contenu de l'eeprom
	/*for (uint8_t i = 0; i < EEPROM.length(); i++) {
		Serial.println(String(i) + " " + String(EEPROM.read(i)));
	}*/

	//Affichage des valeurs (takes ~30ms)
	if (displayValues && SHOW_DEBUG) {
		Serial.print("[EEPROM] Struct Readed : \n");
		
		for (uint8_t i= 0; i < 7; i++) {
			Serial.println(String(i+1) + "eme jour  P1 : " + String(schedule[i].P1Start) + "H-" + String(schedule[i].P1Stop) + "H");
			Serial.println(String(i+1) + "eme jour  P2 : " + String(schedule[i].P2Start) + "H-" + String(schedule[i].P2Stop) + "H");
		}
	}
	EEPROM.end();
	return true; //no error
}

/**
 * Write dummy values in the EEPROM, for testing
 * The return instruction at the begining prevent from writing multiple time
 */
void writeFirstThing() {
	return; //Don't always write

	EEPROM.begin(EEPROM_LENGTH);	
	EEPROM.write(0, EEPROM_MAGICNUMBER >> 8);
	EEPROM.write(1, EEPROM_MAGICNUMBER & 0x00FF);
	EEPROM.write(2, 0x03); //write full week for both range (Monday To Sunday)

	uint8_t cptrHours = 0;
	for (int i= 3; i < EEPROM_LENGTH+1; i++) {
		EEPROM.write(i, cptrHours);
		cptrHours++;
		if (cptrHours > 23) cptrHours = 0;
	}
	EEPROM.end();
}

/**
 * Write the new Schedule inside the EEPROM memory
 * @param schedule ScheduleDay[7] - The array of struct where to write the EEPROM schedule
 * @param showFullWeek bool[2] - Define how we display the schedule on phone (the 7 days of the week, or the work week and the weekend)
 */
void WriteScheduleToEEPROM(ScheduleDay schedule[6]) {

	EEPROM.begin(EEPROM_LENGTH);
	
	EEPROM.write(0, EEPROM_MAGICNUMBER >> 8);
	EEPROM.write(1, EEPROM_MAGICNUMBER & 0xFF);
	EEPROM.write(2, (gShowFullWeek[0] << 1) + gShowFullWeek[1]); // 0000 00XY : useFullWeek[0]= X .. [1] = Y

	for (uint8_t i=0; i < 7; i++) {
		EEPROM.put((sizeof(ScheduleDay) * i)+3, schedule[i]);
	}
	EEPROM.end();
	if (SHOW_DEBUG) Serial.println("EEPROM has been writed !");
}