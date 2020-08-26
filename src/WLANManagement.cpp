/* Gestion de la partie WLAN :
 * WiFi (Connexion)
 * Serveur Web (Handles)
 *
 * Date de création : 24/08/2020
 * Créateur : Nora465
*/

#include "MainHeader.h"

void ConnectToAP() {
	//TODO ne pas commit sur git si le mot de passe est visible (utiliser wifimanager ?)

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
bool handleModifyRelayChange(AsyncWebServerRequest *request) {
	const String GETParamName = "RelayState";

	if (!request->hasParam(GETParamName)) {
		request->send(200, "text/plain", "Erreur : Missing Argument");
		return 0; //TODO revoir ça (si l'arg manque, on veut renvoyer une erreur, pas 0)
	}
	
	bool newState = (request->getParam(GETParamName)->value() == "1");

	digitalWrite(RELAY_CC1, !newState); //inversé car les contacts du relai sont en NF

	request->send(200, "text/plain", String(newState));
	return newState;
}

//S'éxecute pour connaitre l'état du relai
void handleGetRelayState(AsyncWebServerRequest *request, bool CC1State) {
	request->send(200, "text/plain", String(CC1State));
	Serial.print("Ask state: ");
	Serial.println(CC1State);
}