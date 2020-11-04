/* Gestion des sorties (Relais et LEDs)
 *
 * Date de création : 04/09/20 - 17h47
 * Créateur : Nora465
*/

#include "MainHeader.h"

//
void SetPinsMode() {
	pinMode(CAPT_TEMP, INPUT);
	pinMode(USB_D_plus, OUTPUT);
	pinMode(USB_D_moins, OUTPUT);
	pinMode(BP1_AUTOMANU, INPUT);
	pinMode(BP2_FORCEON_OFF, INPUT);
	pinMode(LED_CC1, OUTPUT);
	pinMode(LED_CC2, OUTPUT);
	pinMode(RELAY_CC1, OUTPUT);
	pinMode(RELAY_CC2, OUTPUT);
	
	digitalWrite(USB_D_plus, LOW);
	digitalWrite(USB_D_moins, LOW);
	digitalWrite(LED_CC1, LOW); //TODO stocker les états des relais, et activer les leds en fonction (au démarrage)
	digitalWrite(LED_CC2, LOW);
	digitalWrite(RELAY_CC1, LOW);
	digitalWrite(RELAY_CC2, LOW);
}

//@ret <bool> succès de l'opération
bool ToggleCircuitState(uint8_t circuit, uint8_t state) {
	if (circuit == 1) {
		//Si les 2 BP sont ON, le relay est déjà activé en Mode Forcé (mécaniquement)
		if (!digitalRead(BP1_AUTOMANU)) return false;
		
		digitalWrite(RELAY_CC1, !state); //inversé car les contacts du relai sont en NF
		//digitalWrite(LED_CC1, state);
	} else if (circuit == 2) {
		digitalWrite(RELAY_CC2, !state); //inversé car les contacts du relai sont en NF
		//digitalWrite(LED_CC2, state);
	} else return false;
	return true;
}