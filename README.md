# Jeedom WAF

# Matériel
- AZ touch MOD (https://www.az-delivery.de/fr/products/az-touch-wandgehauseset-mit-touchscreen-fur-esp8266-und-esp32)
- ESP32 VROOM (https://www.amazon.fr/AZ-Delivery-NodeMCU-d%C3%A9veloppement-d%C3%A9nergie-successeur/dp/B071P98VTG/ref=dp_prsubs_1?pd_rd_i=B071P98VTG&th=1)

![Afficheur](https://quand-il.fr/16358389415457983550308449060332.jpg)

 # Objectif
 Création d'un boitier de controle pour piloter Jeedom avec création automatique des boutons via fichier JSON.
 Fonctionne en Wifi.
 Ceci d'ajouter un interrupteur de controle type WAF.

## Paramètres

### Jeedom
| Nom | Clé |
| -------- |---------|
| Clé API Jeedom | key=xxxxxxxxxxxxxxxxxxxxxxxx |
| IP Jeedom | ip=xxx.xxx.xxx.xxx|
| Port Jeedom | port=xx|

### Wifi
| Nom | Clé |
| -------- |---------|
| SSID Wifi | ssid=xxxxxxxxxxxxxxxxxxxxxxx|
| Passphrase | pass=xxxxxxxxxxxxxxxxxxxxxxx|

 ## Fichier JSON
 Accessible via une variable Jeedom (JSONPilote)
 
 https://**#jeedom#**/core/api/jeeApi.php?apikey=**#Jeedom Key#**&type=variable&name=**#JSONPilote#**
 
```json
 {
  "bgcolor": "#OOOOOO",
  "padding": 2,
  "objets": [
    {
      "type": "button",
      "position": [
        1,
        1
      ],
      "code": [
        1018,
        1019,
        1017
      ],
      "texte": "Scanner",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        2,
        1
      ],
      "code": [
        1010,
        1011,
        1009
      ],
      "texte": "Atelier",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        3,
        1
      ],
      "code": [
        591,
        592,
        593
      ],
      "texte": "lampadaire",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        1,
        2
      ],
      "code": [
        1003,
        1004,
        1002
      ],
      "texte": "Ecran",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        2,
        2
      ],
      "code": [
        1055,
        1054,
        1055
      ],
      "texte": "PC",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        3,
        2
      ],
      "code": [
        1428,
        1432,
        1430
      ],
      "texte": "Bureau",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        1,
        3
      ],
      "code": [
        300,
        301,
        299
      ],
      "texte": "Prise1",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        2,
        3
      ],
      "code": [
        300,
        301,
        299
      ],
      "texte": "Prise2",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        3,
        3
      ],
      "code": [
        300,
        301,
        299
      ],
      "texte": "Prise3",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        1,
        4
      ],
      "code": [
        425,
        426,
        427
      ],
      "texte": "Salon",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        2,
        4
      ],
      "code": [
        777,
        778,
        776
      ],
      "texte": "Esteban",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    },
    {
      "type": "button",
      "position": [
        3,
        4
      ],
      "code": [
        94,
        95,
        96
      ],
      "texte": "VMC",
      "couleur": [
        "#BE03FC",
        "#FFFFFF"
      ],
      "forme": "carre",
      "padding": 2
    }
  ],
  "nbButton": 12
}
```

 
