Pour téléverser un nouveau prog dans l'esp, ou le démarrer, mettre les 2 SW sur 0, sinon freeze de l'esp (pourquoi ?)

**Versions ESP** : (fichier binaire : .elf ou .bin)

\- DONE	V2.1  	Refonte de la V1 : piloter un relai (revoir les webhandles)<br>
\- DONE	V2.2  	Piloter les 2 relais manuellement(=forcer l'activation) (modifier les handles !)<br>
\- DONE	V2.2,5	Modif du port Web (50500) // Standardisation des réponses du serveur aux handles (voir paraph V2 Refonte Handles)<br>
\- DONE	V2.3    Faire la partie "programmation journalière"<br>

**Versions Android** : (fichier source : .iai // setup compilé : .apk)

\- (non accessible)	V2.1	Permet de piloter un seul relai (refonte de la V1) - j'ai oublié de sauvegarder le code source
\- DONE 	V2.2,5	Pilotage des 2 relais manuellement + Modif du port Web // standard des réponses du serveur
\- DONE   V3      Charger et modifier les périodes hebdomadaires (sur l'ESP)

**V2 Refonte Handles :**
  \* /GetStates (la réponse du serveur (esp) doit etre : <CC1State> <CC2State> (avec l'espace))<br>
  \* /ForceState ?circuit=1ou2 & state=0ou1
  \* /ChangeMode ?mode=0ou1
  \* /GetSchedule ?range=1ou2 (la réponse du serveur doit etre un tableau (json) des périodes ON-OFF relative à plage (la taille dépend du mode d'affichage))
  \* /ModifySchedule (POST Param: Json)