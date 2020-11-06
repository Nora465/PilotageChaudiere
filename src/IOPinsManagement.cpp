/* Management of the IO pins
 *
 * Creation Date : 04/09/20 - 17h47
 * Creator : Nora465
*/

#include "MainHeader.h"

/** Set the pinMode for all pins & Set the stored value
 *  Execute ONCE at the start of program 
*/ 
void SetPinsMode() {
	pinMode(TEMP_SENSOR, INPUT);
	pinMode(USB_D_PLUS, OUTPUT);
	pinMode(USB_D_MINUS, OUTPUT);
	pinMode(BP1_AUTOMANU, INPUT);
	pinMode(BP2_FORCEON_OFF, INPUT);
	pinMode(LED_CC1, OUTPUT);
	pinMode(LED_CC2, OUTPUT);
	pinMode(RELAY_CC1, OUTPUT);
	pinMode(RELAY_CC2, OUTPUT);
	
	digitalWrite(USB_D_PLUS, LOW);
	digitalWrite(USB_D_MINUS, LOW);
	digitalWrite(LED_CC1, LOW); //TODO stocker les états des relais, et activer les relay/leds en fonction (au démarrage)
	digitalWrite(LED_CC2, LOW);
	digitalWrite(RELAY_CC1, LOW);
	digitalWrite(RELAY_CC2, LOW);
}

/** 
 * Change the state of a relay
 * @param circuit <uint8> the circuit you want to change (must be 1 or 2).
 * @param state   <bool> Wanted state of the circuit (must be 0 or 1)
 * @return <bool> True if success - False if fail
*/
bool ToggleCircuitState(uint8_t circuit, bool state) {
	if (circuit == 1) {
		//If Mode Manu is enable, ignore the change
		if (!digitalRead(BP1_AUTOMANU)) return false;
		
		digitalWrite(RELAY_CC1, !state); //inverted, because the relay contacts is NC
		//digitalWrite(LED_CC1, state);
	} else if (circuit == 2) {
		digitalWrite(RELAY_CC2, !state); //inverted, because the relay contacts is NC
		//digitalWrite(LED_CC2, state);
	} else return false;
	return true;
}