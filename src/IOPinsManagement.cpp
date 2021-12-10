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
 * @param circuit <uint8> the circuit you want to change (must be 1 or 2)
 * @param state   <bool>  Wanted state of the circuit (must be 0 or 1)
 * @return 		  <String> "" if success / "error name" if error
*/
String ToggleCircuitState(uint8_t circuit, bool state) {
	//TODO ajouter un argument à la fonction : reason (et archiver la raison du changement (Forcing, AUTO ...))
	//pas obligé de rajouter un argument, on peut call FS.Archive("IO", <reasonOfChange>)

	String returnStr = "";

	if (circuit == 1) {
		if (digitalRead(RELAY_CC1) == !state) returnStr = "new=old"; //new state = old state

		digitalWrite(RELAY_CC1, !state); //inverted, because the relay contact is NC
		digitalWrite(LED_CC1, state);
	} else if (circuit == 2) {
		if (digitalRead(RELAY_CC2) == !state) returnStr = "new=old"; //new state = old state
		
		digitalWrite(RELAY_CC2, !state); //inverted, because the relay contact is NC
		digitalWrite(LED_CC2, state);
	} else returnStr = "circuit is not 1 or 2";

	gStates[circuit - 1] = state;
	if (returnStr == "") {
		//no error
		if (SHOW_DEBUG) Serial.println("[IO-changeState] OK - circuit " + String(circuit) + ": " + (gStates[circuit-1] ? "activé" : "désactivé"));
		appendStrToFile("[IO-changeState] OK - circuit " + String(circuit) + " => " + (gStates[circuit-1] ? "ACTIF(1)" : "DESACTIVE(0)"));
	} else {
		//error
		//TODO supprimer les references au retour de cette fonction (on le fait déjà ici)
		if (SHOW_DEBUG) Serial.println("[IO-changeState] error - circuit " + String(circuit) + " => erreur : " + String(returnStr));
		appendStrToFile("[IO-changeState] error - circuit " + String(circuit) + " => erreur : " + String(returnStr));
	}
	
	return returnStr;
}