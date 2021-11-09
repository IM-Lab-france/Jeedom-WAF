// Librairies 
#include <ArduinoJson.h>
#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <EEPROM.h>
#include "SimpleBLE.h"

// Declaration des constantes
#define CALIBRATION_FILE "/TouchCalData1"             // Enregistrement de la calibration
#define JEEDOM_KEY "/Jeedom.key"                      // Enregistrement de la coonfiguration Jeedom
#define JEEDOM_IP "/Jeedom.ip"                        // Enregistrement de la coonfiguration Jeedom
#define JEEDOM_PORT "/Jeedom.port"                    // Enregistrement de la coonfiguration Jeedom
#define WIFI_SSID "/Wifi.ssid"                        // Enregistrement de la coonfiguration Jeedom
#define WIFI_PASS "/Wifi.pass"                  // Enregistrement de la coonfiguration Jeedom
#define JSON_FILE "/JSON"                             // Enregistrement du fichier JSON
#define REPEAT_CAL false                              // mettre a true pour calibrer l'ecran
#define LABEL1_FONT &FreeSansOblique12pt7b            // Font 1
#define LABEL2_FONT &FreeSansBold12pt7b               // Font 2 
#define DISP_TCOLOR TFT_CYAN                          // Couleur affichage
#define KEY_TEXTSIZE 1                                // Multiplicateur de taille de Font
#define EEPROM_SIZE 64                                // Taille mémoire necessaire
#define TO_SCREENSAVER 15000                          // Timeout en milliseconde de l'extinction de l'écran
#define STATUS_X 120                                  // Centre du status X
#define STATUS_Y 300                                  // Centre du status Y
#define FORMAT_SPIFFS_IF_FAILED true
#define BUTTON_W 70
#define BUTTON_H 60

// déclaration des structures
struct Bouton {
  int X;
  int Y;
  int On;
  int Off;
  int etat;
  char texte[20];
  char btBgColor[8];
  char btFgColor[8];
  int padding;
  int isOn = 0;
};

struct Jeedom {
  char key[33];
  char ip[21];
  int port;
};

// declaration des variables
const int nbMaxButton = 12;                           // Nombre max de bouton par page
WiFiMulti WiFiMulti;                                  // Objet Wifi
WiFiClient client;                                    // Client Wifi
SimpleBLE ble;                                        // Objet BLEA
TFT_eSPI tft = TFT_eSPI();                            // Objet ecran TFT
TFT_eSPI_Button key[nbMaxButton];                     // Objet bouton tactiles
int bottomDraw = 0;                                   // Position de l'affichage suivant
int screenState = LOW;                                // Etat de l'ecran (LOW = Allumé, HIGH = Eteint)
unsigned long screenSaver = millis();                 // Initialisation delais ScreenSaver
unsigned long timerDisplayHeater = millis();          // Initialisation delais rafraichissement température
Bouton buttons[nbMaxButton];                          // Initialisation du tableau de structure bouton
String lastLine = "";                                 // Dernière ligne récupérée lors des requete par le client
String readString = "";

char JeedomKey[33] = "";
char JeedomIP[21] = "";
char JeedomPort[5] = "";

char WifiSSID[50] = "";
char WifiPassphrase[50] = "";

// void initialisePort() -> Ouverture du port COM pour la communication
void initialisePort() {
  Serial.begin(115200);
  Serial.println("Connection ouverte");
  Serial.setTimeout(5000);
}

// int extractJSON(char* json)  -> Extraction des données JSON
int extractJSON(String json) {
  
  Serial.println("Parsage du Json");
  
  
  // Calculer la taille requise pour le JSON
  const int capacity =  (nbMaxButton * 3 * JSON_ARRAY_SIZE(4)        // Array double (position, couleur, code) 3*12
                        + nbMaxButton * 100                         // pour eviter l'out of memory (reste a trouver la regle de calcul)
                        + JSON_ARRAY_SIZE(nbMaxButton + 1)              // 12 bouton
                        + JSON_OBJECT_SIZE(4)                       // 3 item en racine (bgcolor, padding, objets)
                        + nbMaxButton*JSON_OBJECT_SIZE(8))*2;          // 12 boutons avec 7 objets

  Serial.print("capacity : ");
  Serial.println(capacity);

  Serial.print("Creation JsonDoc de taille : ");
  Serial.println(capacity);
  DynamicJsonDocument doc(capacity);

  // Deserialize the JSON document
  Serial.println("Deserialisation Json");
  DeserializationError error = deserializeJson(doc, string2char(json));

  Serial.println("Test Error Json");
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() erreur: "));
    Serial.println(error.f_str());
    return -1;
  }
  Serial.println("JSON Ok");
  
  const int nbButton = doc["nbButton"];
  const char* bgcolor = doc["bgcolor"];
  long padding = doc["padding"];
  //double latitude = doc["data"][0];
  //double longitude = doc["data"][1];


  Serial.print("Nombre de boutons : ");
  Serial.println(nbButton);
  JsonObject repo;
  JsonArray objets_item_code;

  Serial.println("Debut parsing JSON");
  
  
  for(int x = 0; x<nbButton;x++) {
    
    Serial.print("Boutons : ");
    Serial.println(x);

    repo = doc["objets"][x];
    buttons[x].X = repo["position"][0];
    buttons[x].Y = repo["position"][1];
    
    Serial.println("JsonObject");
    objets_item_code = repo["code"];
    buttons[x].On = objets_item_code[0];
    buttons[x].Off = objets_item_code[1];
    buttons[x].etat = objets_item_code[2];
    
    String repoTexte = repo["texte"];
    String repobtBgColor = repo["couleur"][0];
    String repobtFgColor = repo["couleur"][1];
    
    
    Serial.print("repoTexte:");
    Serial.println(repoTexte);
    if(repoTexte != NULL) {
      Serial.print("On fait repoTexte:");
      strncpy(buttons[x].texte,repo["texte"],sizeof(repo["texte"]) - 1);
    } else {
      Serial.println("repo['texte'] is NULL");
    }

    Serial.println("buttons[x].btBgColor");
    if(repobtBgColor != NULL) {
      strncpy(buttons[x].btBgColor,repo["couleur"][0],sizeof(repo["couleur"][0]) - 1);
    } else {
      Serial.println("repo['couleur'][0] is NULL");
    }  
    
    Serial.println("buttons[x].btFgColor");
    if(repobtFgColor != NULL) {
      strncpy(buttons[x].btFgColor,repo["couleur"][1],sizeof(repo["couleur"][1]) - 1);
    } else {
      Serial.println("repo['couleur'][0] is NULL");
    }  
    Serial.println("buttons[x].padding");
    buttons[x].padding = repo["padding"];
    Serial.println("buttons[x].padding");
    
  }
  
  Serial.println("Fin parsing JSON");

  return nbButton;
}

// void readValueJSON(int nbButton) -> Lecture des données extraites du JSON
void readValueJSON(int nbButton) {
  for(int x = 0; x<nbButton;x++) {
    Serial.print("Bouton : ");
    Serial.println(x);

    Serial.print("X : ");
    Serial.println(buttons[x].X);

    Serial.print("Y : ");
    Serial.println(buttons[x].Y);

    Serial.print("On : ");
    Serial.println(buttons[x].On);

    Serial.print("Off : ");
    Serial.println(buttons[x].Off);

    Serial.print("btBgColor : ");
    Serial.println(buttons[x].btBgColor);

    Serial.print("btFgColor : ");
    Serial.println(buttons[x].btFgColor);

    Serial.print("texte : ");
    Serial.println(buttons[x].texte);

    Serial.print("padding : ");
    Serial.println(buttons[x].padding);

  }

}

// void displayOn() -> Allume l'ecran TFT
void displayOn () {
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  screenState = LOW;
  delay(1);
  Serial.println("Ecran Allume");
}

// void displayOff() -> Eteint l'ecran TFT
void displayOff () {
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  screenState = HIGH;
  delay(1);
  Serial.println("Ecran Eteint");
  screenSaver = millis();
  //refreshElec();
}

// void listDir(fs::FS &fs, const char * dirname, uint8_t levels) -> Lister les fichiers
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

// void readFile(fs::FS &fs, const char * path) -> Lire un fichier
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

// void writeFile(fs::FS &fs, const char * path, const char * message) -> Ecrit un fichier
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

// void deleteFile(fs::FS &fs, const char * path) -> Efface les fichiers
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

// bool jeedomParam() -> Récupération des infos Jeedom
bool jeedomParam() {
  uint8_t jeedomKeyOK = 0;
  uint8_t jeedomIPOK = 0;
  uint8_t jeedomPortOK = 0;
  bool returnValue = false;
  //char* jeedomKey;
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return returnValue;
  }

  listDir(SPIFFS, "/", 0);

  if (SPIFFS.exists(JEEDOM_KEY)) {
    File f = SPIFFS.open(JEEDOM_KEY, "r");
    if (f) {
      if (f.readBytes((char *)JeedomKey, 32) == 32)
        jeedomKeyOK = 1;
      f.close();
    } else {
      status("Pas de clé Jeedom",STATUS_Y,TFT_DARKGREY);
    }
  } else {
    status("key=xxxxxxxxx",STATUS_Y,TFT_DARKGREY);
  }

  if (SPIFFS.exists(JEEDOM_IP)) {
    File f = SPIFFS.open(JEEDOM_IP, "r");
    if (f) {
      if (f.readBytes((char *)JeedomIP, 20) >= 7)
        jeedomIPOK = 1;
      f.close();
    } else {
      status("Pas d'ip Jeedom",STATUS_Y,TFT_DARKGREY);
    }
  } else {
    status("ip=x.x.x.x",STATUS_Y,TFT_DARKGREY);
  }

  if (SPIFFS.exists(JEEDOM_PORT)) {
    File f = SPIFFS.open(JEEDOM_PORT, "r");
    if (f) {
      if (f.readBytes((char *)JeedomPort, 5) >= 2)
        jeedomPortOK = 1;
      f.close();
    } else {
      status("Pas de port Jeedom",STATUS_Y,TFT_DARKGREY);
    }
  } else {
    status("port=xx",STATUS_Y,TFT_DARKGREY);
  }

  if (jeedomKeyOK && jeedomIPOK && jeedomPortOK) {
    Serial.print("Key:");
    Serial.println(JeedomKey);
    Serial.print("IP:");
    Serial.println(JeedomIP);
    Serial.print("Port:");
    Serial.println(JeedomPort);
    returnValue = true;
  } else {
    Serial.println("Pas de jeedom Key -> key=xxxxxxxxxxxxxxxxxxxxxxxx   ip=xxx.xxx.xxx.xxx   port=xx");
    status("Pas de clé Jeedom",STATUS_Y,TFT_DARKGREY);
  }

  return returnValue;
} 

// bool wifiParam() -> Récupération des infos Jeedom
bool wifiParam() {
  uint8_t wifiSSIDOK = 0;
  uint8_t wifiPassPhraseOK = 0;
  
  bool returnValue = false;
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return returnValue;
  }

  //listDir(SPIFFS, "/", 0);

  if (SPIFFS.exists(WIFI_SSID)) {
    File f = SPIFFS.open(WIFI_SSID, "r");
    if (f) {
      if (f.readBytes((char *)WifiSSID, 32) > 5)
        wifiSSIDOK = 1;
      f.close();
    } else {
      status("Pas de SSID Wifi",STATUS_Y,TFT_DARKGREY);
      Serial.println("Pas de SSID Wifi");
    }
  } else {
    status("ssid=xxxxxxxxx",STATUS_Y,TFT_DARKGREY);
    Serial.println("ssid=xxxxxxxxx");
  }

  if (SPIFFS.exists(WIFI_PASS)) {
    File f = SPIFFS.open(WIFI_PASS, "r");
    if (f) {
      if (f.readBytes((char *)WifiPassphrase, 49) >= 5)
        wifiPassPhraseOK = 1;
      f.close();
    } else {
      status("Pas de pass Wifi",STATUS_Y,TFT_DARKGREY);
      Serial.println("Pas de pass Wifi");
    }
  } else {
    status("pass=xxxxxxxxx",STATUS_Y,TFT_DARKGREY);
    Serial.println("pass=xxxxxxxxx");
  }
  
  if (wifiSSIDOK == 1 && wifiPassPhraseOK == 1) {
    returnValue = true;
  } else {
    Serial.println("Pas de configuration wifi -> ssid=xxxxxxxxxxxxxxxxxxxxxxxx   pass=xxxxxxxxxxxxxxxxxxx");
    status("Pas de conf Wifi",STATUS_Y,TFT_DARKGREY);
  }

  return returnValue;
} 

// void touch_calibrate() -> Calibration du TFT
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

// Initialisation de l'ecran et mise au noir de l'affichage.
void initScreen() {
  displayOn();                                        // Allumage de l'écran
  tft.init();                                         // Initialisation du TFT
  tft.setRotation(0);                                 // Rotation TFT (0 = Portrait)
  touch_calibrate();                                  // Lancement de la calibration de l'ecran
  tft.fillScreen(TFT_BLACK);                          // Vider l'ecran et mettre en noir
  tft.fillRect(0, 0, 240, 320, TFT_BLACK);            // Tracer un rectangle sur l'ensemble de l'afficheur
  tft.setFreeFont(&FreeMonoBold9pt7b);                // Fonte par defaut 
}

// void status(const char *msg, int position, uint16_t bgcolor) -> Affichage du status
void status(const char *msg, int position, uint16_t bgcolor) { 
  tft.setTextPadding(240);                            // Padding du texte
  tft.setTextColor(TFT_WHITE, bgcolor);               // Couleur blanche
  tft.setTextFont(0);                                 // Font 0
  tft.setTextDatum(TC_DATUM);                         // Type texte
  tft.setTextSize(2);                                 // Taille font
  tft.drawString(msg, STATUS_X, position);            // Trace la chaine de caractère
}

// void connectWifiAP() -> initialisation du Wifi
void connectWifiAP() {
  status("Connexion au Wifi...",STATUS_Y,TFT_DARKGREY);         // Connexion au Wifi
  WiFi.begin(WifiSSID, WifiPassphrase);                // Initialisation de la connexion Wifi
  status("En attente du WiFi. ",STATUS_Y,TFT_DARKGREY);         // Status en attente Wifi

  while(WiFi.status() != WL_CONNECTED) {                      // Tant que Wifi non connecté
    Serial.print(".");                                              // Affiche un point
    delay(500);                                                     // Tite pause
  }
    
  status("WiFi OK",STATUS_Y,TFT_DARKGREY);                      // Wifi connecté
  Serial.println("IP address: ");                                   
  Serial.println(WiFi.localIP());                               // Affichage IP 
}

// void onButton() -> si un bouton est activé
void onButton(){
    String out = "Jeedom Controle";
    out += String(millis() / 1000);
    Serial.println("jj");
    Serial.println(out);
    ble.begin(out);
}

// void refreshBLE() -> 
void refreshBLE() {
  static uint8_t lastPinState = 1;
  uint8_t pinState = digitalRead(0);
  if(!pinState && lastPinState){
      onButton();
  }
  lastPinState = pinState;
  while(Serial.available()) Serial.write(Serial.read());
}

// void callToActionUrl(int CmdId) -> Appel l'URL Jeedom correspondant à l'ID demandé
void callToActionUrl(int CmdId, int toOn) {
  lastLine = "";
  String url = String("/core/api/jeeApi.php?apikey=") + JeedomKey + "&type=cmd&id=";
  if(toOn == 1) {
    url += String(buttons[CmdId].On);
  } else if(toOn == 0) {
    url += String(buttons[CmdId].Off);
  } else {
    url += String(buttons[CmdId].etat);
  }

  Serial.print("Requête URL : ");
  Serial.println(url);

  if (client.connect(JeedomIP, atoi(JeedomPort))) {                             // Connexion au serveur Jeedom
      Serial.println("connecté");
    
    // This will send the request to the server
    client.println(String("GET ") + url + " HTTP/1.1\r\n" +      // Envoie de l'URL vers jeedom
                "Host: " + "jeedom.fozzy.fr" + "\r\n" +
                "Connection: close\r\n\r\n");
    unsigned long timeout = millis();                           // Configuration du Timeout de connexion
    while (client.available() == 0) {                           // Tant que la connexion n'est pas fermée
      if (millis() - timeout > 5000) {                              // Si TimeOut passé  
          Serial.println(">>> Client Timeout !");                   // Affichage erreur
          client.stop();                                            // Arret du client
          return;                                               // return TimeOut
      }
    }
  } else {
    Serial.println("Pas de connexion au serveur");            // Connexion au serveur impossible
  }

  screenSaver = millis();                                     // Initialisation du compteur screenSaver

  while(client.available()) {                                 // Tant que le client est ouvert
    lastLine = client.readStringUntil('\r');                      // Sauvegarder la dernière ligne de retour de Jeedom
    //lastLine = lastLine.trim('\n');
    //lastLine = lastLine.trim('\r');
    //lastLine = lastLine.trim();
  }

  Serial.println("Fermeture de la connexion");                // Message de fermeture de la connexion
}

// char* string2char(String command) -> String vers Char*
char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

// String callJSON() -> Recupération du JSON de paramétrage
String callJSON() {
  
  String url = String("/core/api/jeeApi.php?apikey=") + JeedomKey + "&type=variable&name=JSONPilote";
  
  Serial.print("Requête URL : ");
  Serial.println(url);
  
  if (client.connect(JeedomIP, atoi(JeedomPort))) {                             // Connexion au serveur Jeedom
      Serial.println("connecté");
    
    // This will send the request to the server
    client.println(String("GET ") + url + " HTTP/1.1\r\n" +      // Envoie de l'URL vers jeedom
                "Host: " + "jeedom.fozzy.fr" + "\r\n" +
                "Connection: close\r\n\r\n");
    unsigned long timeout = millis();                           // Configuration du Timeout de connexion
    while (client.available() == 0) {                           // Tant que la connexion n'est pas fermée
      if (millis() - timeout > 5000) {                              // Si TimeOut passé  
          Serial.println(">>> Client Timeout !");                   // Affichage erreur
          client.stop();                                            // Arret du client
          return "";                                         // return TimeOut
      }
    }
  } else {
    Serial.println("Pas de connexion au serveur");            // Connexion au serveur impossible
  }

  screenSaver = millis();                                     // Initialisation du compteur screenSaver

  while(client.available()) {                                 // Tant que le client est ouvert
    lastLine = client.readStringUntil('\r');                      // Sauvegarder la dernière ligne de retour de Jeedom
  }
  
  Serial.println("Fermeture de la connexion");                // Message de fermeture de la connexion


  char c;
  char no = '\\'; //character I want removed.

  for (int i=0; i<lastLine.length()-1;++i){
    c = lastLine.charAt(i);
    if(c==no){
      lastLine.remove(i, 1);
    }
  }

  Serial.println(lastLine);

  return lastLine;
}

// unsigned int hexToDec(String hexString) -> Conversion HEX vers DEC
unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}

// void drawButton(int initrow, int nbCol, int X, int Y, int W,  int H , int keySpac) -> Tracage des boutons 
void drawButton(int idButton) {
  String BgRGBHex = String(buttons[idButton].btBgColor);
  String FgRGBHex = String(buttons[idButton].btFgColor);
  int BgR = hexToDec(BgRGBHex.substring(1, 3));
  int BgG = hexToDec(BgRGBHex.substring(3, 5));
  int BgB = hexToDec(BgRGBHex.substring(5, 7));
  int FgR = hexToDec(FgRGBHex.substring(1, 3));
  int FgG = hexToDec(FgRGBHex.substring(3, 5));
  int FgB = hexToDec(FgRGBHex.substring(5, 7));

  
  uint16_t BgRGB = ((BgB >> 3) | ((BgG & 0xfc) << 5) | (BgR & 0xf8) << 11);
  uint16_t FgRGB = ((FgB >> 3) | ((FgG & 0xfc) << 5) | (FgR & 0xf8) << 11);

  key[idButton].initButton(&tft,                                                              // Prépare tracage du bouton
                    (BUTTON_W / 2) + buttons[idButton].X * (BUTTON_W + 5) - BUTTON_W ,        // x
                    (BUTTON_H / 2) +(buttons[idButton].Y * (BUTTON_H + 5)) - BUTTON_H,        // y
                    BUTTON_W,                                                                 // w
                    BUTTON_H,                                                                 // h
                    TFT_WHITE,                                                                // outline
                    BgRGB, FgRGB,                                                             // fill
                    buttons[idButton].texte, KEY_TEXTSIZE);                                   // text
  
  if(buttons[idButton].isOn) {
    key[idButton].drawButton(true);   
  } else {
    key[idButton].drawButton();   
  }
  
                                                                // affichage du bouton
}

// void inputParam() -> gestion des paramètres
void inputParam() {
  if(Serial.available()) 
  {
    Serial.println("aquisition");
  }
  while (Serial.available()) 
  {
    delay(3);  
    if (Serial.available() >0) 
    {
      char c = Serial.read();  
      readString += c; 
    } 
  }
 
  char *p = (char*)readString.c_str();
  char *str;
  if(readString != "") { 

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      Serial.println("SPIFFS Mount Failed"); 
      return;
    }

    Serial.println(readString);
    if(readString.substring(0, 3) == "key") {  
      Serial.println("la clé");
      writeFile(SPIFFS, JEEDOM_KEY, string2char(readString.substring(4, readString.length()-1)));
    }
    if(readString.substring(0, 3) == "ip=") {
      Serial.println("l'IP");
      writeFile(SPIFFS, JEEDOM_IP, string2char(readString.substring(3, readString.length()-1)));
    }
    if(readString.substring(0, 3) == "por") {
      Serial.println("le port");
      writeFile(SPIFFS, JEEDOM_PORT, string2char(readString.substring(5, readString.length()-1)));
    }
    if(readString.substring(0, 3) == "ssi") {
      Serial.println("le SSID");
      writeFile(SPIFFS, WIFI_SSID, string2char(readString.substring(5, readString.length()-1)));
    }
    if(readString.substring(0, 3) == "pas") {
      Serial.println("le pass Wifi");
      writeFile(SPIFFS, WIFI_PASS, string2char(readString.substring(5, readString.length()-1)));
    }
    if(readString.substring(0, 3) == "del") {
      Serial.println("On efface tout");
      deleteFile(SPIFFS, JEEDOM_KEY);
      deleteFile(SPIFFS, JEEDOM_IP);
      deleteFile(SPIFFS, JEEDOM_PORT);
      deleteFile(SPIFFS, WIFI_SSID);
      deleteFile(SPIFFS, WIFI_PASS);
      ESP.restart();
    }
    if(readString.substring(0, 3) == "res") {
      Serial.println("Redemarrage");
      ESP.restart();
    }
    readString = "";
  }
  

  
}

void getStatus(int x) {
  // récupération de l'état
    
  callToActionUrl(x, 2);
  
  lastLine = lastLine.substring(1, lastLine.length());
  
  Serial.print("#");
  Serial.print(lastLine);
  Serial.println("#");


  if(lastLine != "") {

    if(lastLine.toInt() > 0) {
      buttons[x].isOn = 1;
    } else {
      buttons[x].isOn = 0;
    }
  } else {
      Serial.println("Pas de retour");
  }
    delay(50);
}

// setup () -> Le setup
void setup() {

  initialisePort();                                   // Ouverture port COM
  ble.begin("Jeedom");                                // Activation BLEA
  initScreen();                                       // Initialisation de l'ecran 
  if(wifiParam()) {
    connectWifiAP();                                    // Connexion au WIFI
    if(jeedomParam()) {

      int nbButton = extractJSON(callJSON());             // Extraction des données JSON
    
      for(int x = 0; x<nbButton;x++) {
        getStatus(x);
        drawButton(x);
      }
    } else {
      status("Pb conf Jeedom",STATUS_Y,TFT_DARKGREY);
    }
  } else {
    status("Pb conf Wifi",STATUS_Y,TFT_DARKGREY);
  }
}

// loop () -> Et sinon...
void loop() {
  inputParam();
  refreshBLE();
  if (millis() - screenSaver > TO_SCREENSAVER) {                               // Si le TimeOut de l'ecran est arrivé
    displayOff ();                                                        // Extinction de l'écran
  }

  uint16_t t_x = 0, t_y = 0;                                              // Initialisation des variable de pression bouton
  bool pressed = tft.getTouch(&t_x, &t_y);                                // récupération de la position de la pression
  if(pressed && screenState == HIGH) {
    Serial.println("pressed");
    displayOn ();                                                             // Allume l'ecran
    delay(100); 
  } else {
    for (uint8_t b = 0; b < 12; b++) {                                      // Vérifie si les coordonnées de pression sont contenue dans une boite de pression
      if (pressed && key[b].contains(t_x, t_y)) {                           // Si c'est contenue dans une boite de pression
        if (screenState == LOW)  {
          key[b].press(true);                                                     // Indiqué que le bouton est pressé
        } else {
          key[b].press(false); 
          displayOn (); 
        }
      } else {
        key[b].press(false);                                                    // Indiqué que le bouton n'est pas pressé
      }
    }
    
    for (uint8_t b = 0; b < 12; b++) {                                       // Parcours de tous les boutons
      if (screenState == LOW && key[b].justReleased()) {                                              // Si le bouton est libéré à l'instant
        if(!buttons[b].isOn) {
          key[b].drawButton(true);                                                    // Trace le bouton
          buttons[b].isOn = 1;
        } else {
          key[b].drawButton();
          buttons[b].isOn = 0;
        }
        
        if (screenState == LOW)                                                 // Si l'ecran est allumé
        {
          callToActionUrl(b,buttons[b].isOn);                                       // Appel de Jeedom
          status(buttons[b].texte,STATUS_Y,TFT_DARKGREY);                           // Afficher le status du bouton
        } else {
          displayOn ();                                                             // Allume l'ecran
          delay(100);                                                               // Tite pause
        }
      }   
    }
    /*
    if (key[b].justPressed()) {                                               // Si le bouton est pressé à l'instant
      key[b].drawButton(true);                                                    // Inverse le bouton
    }
    */
  }
}

