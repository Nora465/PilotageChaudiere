/* Header file containing : 
 * the storage structures of the time schedule package
 *
 * Creation Date : 01/09/2020 - 23h45
 * Creator : Nora465
*/

#ifndef DAILYSCHEDULE_H
	#define DAILYSCHEDULE_H		//total for the structure : 31 bytes (2 bytes of validation, 1 byte of Schedule use, and 28 bytes of data)
	/*
	struct ScheduleDay	//P1 : Range 1 (Work)    P2 : Range 2 (Holiday)
	{
		uint8_t P1Start;
		uint8_t P1Stop;
		uint8_t P2Start;
		uint8_t P2Stop;
	};
	*/

	struct Period {
		uint8_t Start;
		uint8_t Stop;
	};

	struct Range {
		Period Periods[2]; //2 periods
	};

	struct ScheduleDay {
		Range Ranges[2]; //2 ranges
	};

	//the ScheduleDay structure takes 28 bytes (+2 for verifications and +1 for the schedule use)
	//ScheduleDay => 8 bytes
	//for 7 days * 8 => 56 (+2 for magic number and +1 for the schedule use)
	#define EEPROM_LENGTH 58 // EEPROM starts at @0 => 59-1 = 58

	//random number stored in @0-1 of EEPROM
	//Checks if the schedule has been write (at least) one time by this specific program
	const uint16_t EEPROM_MAGICNUMBER = 35187;

#endif