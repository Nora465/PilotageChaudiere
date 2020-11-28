/* Header file containing : 
 * the storage structures of the time schedule package
 *
 * Creation Date : 01/09/2020 - 23h45
 * Creator : Nora465
*/

#ifndef DAILYSCHEDULE_H
	#define DAILYSCHEDULE_H		//total for the structure : 30 bytes (28 bytes of data, 2 bytes of validation)

	struct ScheduleDay	//P1 : Range 1 (Work)    P2 : Range 2 (Holiday)
	{
		uint8_t P1Start;
		uint8_t P1Stop;
		uint8_t P2Start;
		uint8_t P2Stop;
	};

	#define EEPROM_LENGTH 30 //the ScheduleWeek structure takes 28 bytes (+2 for verifications)

	//random number stored in @0-1 of EEPROM
	//Checks if the schedule has been write (at least) one time
	const uint16_t EEPROM_MAGICNUMBER = 29127;

#endif