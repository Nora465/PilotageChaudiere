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
bool LoadEEPROMSchedule(ScheduleDay schedule[7], bool displayValues) {

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
	for (uint8_t i=0; i < DAYS_PER_WEEK; i++) {
		EEPROM.get((sizeof(typeof(schedule[i])) *i) +3, schedule[i]);
	}

	//affichage du contenu de l'eeprom
	/*for (uint8_t i = 0; i < EEPROM.length(); i++) {
		Serial.println(String(i) + " " + String(EEPROM.read(i)));
	}*/

	//Printing values of schedule (takes ~30ms)
	if (displayValues && SHOW_DEBUG) {
		Serial.print("[EEPROM] Struct Readed : \n");
		
		for (uint8_t i= 0; i < DAYS_PER_WEEK; i++) {
			if (SHOW_DEBUG) Serial.println(String(i+1) + "eme jour  P1(1er période) : " + String(schedule[i].Ranges[0].Periods[0].Start) + "H-" + String(schedule[i].Ranges[0].Periods[0].Stop) + "H");
			if (SHOW_DEBUG) Serial.println(String(i+1) + "eme jour  P1(2em période) : " + String(schedule[i].Ranges[0].Periods[1].Start) + "H-" + String(schedule[i].Ranges[0].Periods[1].Stop) + "H");
		}
		if (SHOW_DEBUG) Serial.println("");
		for (uint8_t i= 0; i < DAYS_PER_WEEK; i++) {
			if (SHOW_DEBUG) Serial.println(String(i+1) + "eme jour  P2(1er période) : " + String(schedule[i].Ranges[1].Periods[0].Start) + "H-" + String(schedule[i].Ranges[1].Periods[0].Stop) + "H");
			if (SHOW_DEBUG) Serial.println(String(i+1) + "eme jour  P2(2em période) : " + String(schedule[i].Ranges[1].Periods[1].Start) + "H-" + String(schedule[i].Ranges[1].Periods[1].Stop) + "H");
		}
	}
	EEPROM.end();
	return true; //no error
}

/**
 * Write the new Schedule inside the EEPROM memory
 * @param schedule ScheduleDay[7] - The array of struct where to write the EEPROM schedule
 * @param showFullWeek bool[2] - Define how we display the schedule on phone (the 7 days of the week, or the work week and the weekend)
 */
void WriteScheduleToEEPROM(ScheduleDay schedule[7]) {
	
	EEPROM.begin(EEPROM_LENGTH);
	
	EEPROM.write(0, EEPROM_MAGICNUMBER >> 8);
	EEPROM.write(1, EEPROM_MAGICNUMBER & 0xFF);
	EEPROM.write(2, (gShowFullWeek[0] << 1) | gShowFullWeek[1]); // 0000 00XY : useFullWeek[0]= X .. [1] = Y
	
	for (uint8_t i=0; i < DAYS_PER_WEEK; i++) {
		EEPROM.put((sizeof(typeof(schedule[i])) *i) +3, schedule[i]);
	}

	EEPROM.end();
	if (SHOW_DEBUG) Serial.println("EEPROM has been writed !");
}