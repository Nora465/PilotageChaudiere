/* Management of the IO pins
 *
 * Creation Date : 04/09/20 - 17h47
 * Creator : Nora465
*/

#include "MainHeader.h"
extern bool gStates[2];

/** Set the pinMode for all pins & Set the stored value
 *  Execute ONCE at the start of program 
*/ 
void SetPinsMode() {
	pinMode(TEMP_SENSOR, INPUT);
	pinMode(USB_D_PLUS, OUTPUT);
	pinMode(USB_D_MINUS, OUTPUT);
	pinMode(BP1_AUTOMANU, INPUT_PULLUP);
	pinMode(BP2_FORCEON_OFF, INPUT_PULLUP);
	pinMode(LED_CC1, OUTPUT);
	pinMode(LED_CC2, OUTPUT);
	pinMode(RELAY_CC1, OUTPUT);
	pinMode(RELAY_CC2, OUTPUT);
	
	digitalWrite(USB_D_PLUS, LOW);
	digitalWrite(USB_D_MINUS, LOW);
	digitalWrite(LED_CC1, HIGH); //TODO stocker les états des relais, et activer les relay/leds en fonction (au démarrage)
	digitalWrite(LED_CC2, HIGH);
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
	//TODO ajouter un argument à la fonction : reason (et archiver la raison du changement (Forcing, AUTO ...))
	//pas obligé de rajouter un argument, on peut call FS.Archive("IO", <reasonOfChange>)
	//TODO au lieu de return un <bool> (on connait pas l'erreur), on peut return "" si OK, ou "<raison de l'erreur>" 
	//si erreur (en sortie de la fonction, on fera un if (ToggleCircuitState(1,0) == ""))

	if (circuit == 1) {
		if (!digitalRead(BP1_AUTOMANU)) return false; //If (Physical) Manual Mode is enable, ignore the change
		if (digitalRead(RELAY_CC1) == !state) return false; //don't change if the new state = old state

		digitalWrite(RELAY_CC1, !state); //inverted, because the relay contact is NC
		digitalWrite(LED_CC1, state);
	} else if (circuit == 2) {
		if (digitalRead(RELAY_CC2) == !state) return false; //don't change if the new state = old state
		
		digitalWrite(RELAY_CC2, !state); //inverted, because the relay contact is NC
		digitalWrite(LED_CC2, state);
	} else return false;

	gStates[circuit - 1] = state;
	if (SHOW_DEBUG) Serial.println("[IO] OK - Changed state of circuit " + String(circuit) + ": " + (gStates[circuit] ? "activé" : "désactivé"));
	//appendStrToFile("[IO] OK - Modification de l'etat du circuit " + String(circuit) + " => " + (gStates[circuit] ? "ACTIF(1)" : "DESACTIVE(0)"));
	return true;
}