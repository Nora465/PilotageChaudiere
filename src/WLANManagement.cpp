/* Gestion de la partie WLAN :
 * WiFi (Connexion)
 * Serveur Web (Handles)
 *
 * Date de création : 24/08/2020
 * Créateur : Nora465
*/
#include "MainHeader.h"

void ConnectToAP() {

	WiFiManager wifiManager;
	//wifiManager.resetSettings(); //reset du réseau sauvegardé (DEBUG&TEST UNIQUEMENT)

	//Désactivation du debug (trop intrusif)
	wifiManager.setDebugOutput(false);

	//Réglage du portail captif    (IP					  , Passerelle			  , Masque Sous-Réseau)
	wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

	//Callback : s'execute quand aucun réseau n'est enregistré (active le portail captif)
	wifiManager.setAPCallback([](WiFiManager *wiFiManager) {
		Serial.print("\n[WiFi] Config \"Portail Captif\" en cours : " + String(wiFiManager->getConfigPortalSSID()));
	});

	//Tentative de connexion à un réseau enregistré
	//Si echec, créé un AP et demande à l'utilisateur de se connecter sur un réseau (bloquant)
	Serial.print("[WiFi] Connexion au réseau en cours...");
	wifiManager.autoConnect("HELP_CHAUDIERE_CONTROL_OFF"); //AutoConnectAP
	Serial.println("\n[WiFi] Connexion réussie !");

	//Affichage du nom du réseau et de l'@IP de l'ESP
	Serial.println();
	Serial.print("[WiFi] Connecté à \"");
	Serial.print(WiFi.SSID()); Serial.println("\"");
	Serial.print("[WiFi] @IP : ");
	Serial.println(WiFi.localIP());
}

// S'éxecute au changement d'état du relai
void HandleForceState(AsyncWebServerRequest *request, bool gStates[2]) { //URI : /ForceState?circuit=(1ou2)&state=(0ou1)

	//Vérif : Présence des paramètres
	if (!request->hasParam("circuit") || !request->hasParam("state")) {
		return request->send(400, "text/plain", "Erreur : Missing Argument(s)");
	}

	//Vérif : Cohérence de la valeur du Param "state"
	String stateValue = request->getParam("state")->value();
	if (stateValue != "1" && stateValue != "0") {
		return request->send(400, "text/plain", "Erreur : Bad Value For \"State\" Param (0 or 1)");
	}
	
	//Vérif : Cohérence de la valeur du Param "circuit"
	uint8_t circuitNumber = request->getParam("circuit")->value().toInt();
	if (circuitNumber != 1 && circuitNumber != 2) {
		return request->send(400, "text/plain", "Erreur : Bad Value For \"Circuit\" Param (1 or 2)");
	}

	//Ecriture sur la sortie correspondante
	bool circuitState = (stateValue == "1");
	ToggleCircuitState(circuitNumber, circuitState);

	//Stockage dans le tableau global
	gStates[circuitNumber - 1] = circuitState;
	
	Serial.println("Force state of CC" + String(circuitNumber) + " : " + String(gStates[circuitNumber - 1]));
	return request->send(200, "text/plain", String(gStates[0]) + " " + String(gStates[1]));
}

//S'éxecute pour connaitre l'état du relai
void HandleGetState(AsyncWebServerRequest *request, bool gStates[2]) { //URI : /GetStates

	//Envoi de la valeur des 2 circuits (et print)
	String stateValues = String(gStates[0]) + " " + String(gStates[1]);
	request->send(200, "text/plain", stateValues);
	Serial.println("Ask states : " + stateValues);
}

//Mode MANU / AUTO
void HandleChangeMode(AsyncWebServerRequest *request, bool gModeAuto) { //URI : /ChangeMode?mode=(0 ou 1)
	//Vérif : Présence du paramètre
	if (!request->hasParam("mode")) {
		return request->send(400, "text/plain", "Erreur : Missing Argument \"mode\"");
	}

	//Vérif : Cohérence de la valeur du Param "mode"
	String modeValue = request->getParam("mode")->value();
	if (modeValue != "0" && modeValue != "1") {
		return request->send(400, "text/plain", "Erreur : Bad Value For \"mode\" Param (0 or 1)");
	}

	//Vérif : Position AUTO - bouton "Mode" Physique
	if (!digitalRead(BP1_AUTOMANU)) {
		Serial.println("Can't change the mode : Physical SW enabled");
		return request->send(200, "text/plain", "Impossible de changer le mode : Le Mode est forcé physiquement");
	}

	//Assignation du nouveau mode dans la variable globale
	gModeAuto = (modeValue == "1");
	Serial.println("Mode Changé : " + String(gModeAuto));
	return request->send(200, "text/plain", "Mode Changé : " + String(gModeAuto));
	
}

void HandleModifySchedule(AsyncWebServerRequest *request, ScheduleDay *schedule) {

	//TODO Schedule : créer la liaison entre ESP et smartphone

	//get the new struct, put it in "schedule"

	WriteScheduleToEEPROM(schedule);
	//fonction : maj des alarmes ?
}