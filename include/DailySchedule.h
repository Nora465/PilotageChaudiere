/* Header file containing : 
 * the storage structures of the time schedule package
 *
 * Creation Date : 01/09/2020 - 23h45
 * Creator : Nora465
*/

#ifndef DAILYSCHEDULE_H
	#define DAILYSCHEDULE_H		//total for the structure : 31 bytes (2 bytes of validation, 1 byte of Schedule use, and 28 bytes of data)

	struct ScheduleDay	//P1 : Range 1 (Work)    P2 : Range 2 (Holiday)
	{
		uint8_t P1Start;
		uint8_t P1Stop;
		uint8_t P2Start;
		uint8_t P2Stop;
	};

	//the ScheduleWeek structure takes 28 bytes (+2 for verifications and +1 for the schedule use)
	#define EEPROM_LENGTH 30 // EEPROM starts at @0 => 31-1 = 30

	//random number stored in @0-1 of EEPROM
	//Checks if the schedule has been write (at least) one time by this specific program
	const uint16_t EEPROM_MAGICNUMBER = 29127;

#endif