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
void handleForceState(AsyncWebServerRequest *request, bool gStates[2]) { //URI : /ForceState?circuit=(1ou2)&state=(0ou1)

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
	//TODO il doit y avoir un moyen plus joli de le faire ?
	if (circuitNumber == 1) {
		digitalWrite(RELAY_CC1, !circuitState); //inversé car les contacts du relai sont en NF
	} else if (circuitNumber == 2) {
		digitalWrite(RELAY_CC2, !circuitState); //inversé car les contacts du relai sont en NF
	}

	//Stockage dans le tableau global
	gStates[circuitNumber - 1] = circuitState;
	
	Serial.println("Force state of CC" + String(circuitNumber) + " : " + String(gStates[circuitNumber - 1]));
	return request->send(200, "text/plain", String(gStates[circuitNumber - 1]));
}

//S'éxecute pour connaitre l'état du relai
void handleGetState(AsyncWebServerRequest *request, bool states[2]) { //URI : /GetState?circuit=(1ou2)

	//Vérif : Présence Param "circuit"
	if (!request->hasParam("circuit")) {
		return request->send(200, "text/plain", "Erreur : Missing Argument \"circuit\"");
	}

	//Vérif : Cohérence de la valeur du Param "circuit"
	uint8_t circuitNumber = request->getParam("circuit")->value().toInt();
	if (circuitNumber != 1 && circuitNumber != 2) {
		return request->send(400, "text/plain", "Erreur : Bad Value For \"Circuit\" Param (1 or 2)");
	}

	//Envoi de la valeur du circuit demandé (et print)
	request->send(200, "text/plain", String(states[circuitNumber - 1]));
	Serial.println("Ask state of CC" + String(circuitNumber) + " : " + String(states[circuitNumber - 1]));
}