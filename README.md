# Projet Embarqué — Acquisition de données BMP280 via XBee + Dashboard Web

> Projet réalisé dans le cadre du cursus ingénieur à l'**ENIB** (École Nationale d'Ingénieurs de Brest) — 2025/2026.

---

## Présentation générale

Ce projet met en œuvre une chaîne complète d’acquisition et de visualisation de données environnementales (pression atmosphérique et température) transmises **sans fil** jusqu’à un dashboard web.

### Chaîne de transmission

```
[BMP280]
   | I²C
   ▾
[STM32F411]  ── UART ──►  [XBee émetteur]
                               |
                           ~~~ radio 2.4 GHz ~~~
                               |
                          [XBee récepteur]  ── USB/UART ──►  [PC]
                                                              |
                                                        [server.py]
                                                              |
                                                     [Dashboard Web]
                                                    http://localhost:5000
```

### Détail étape par étape

1. **BMP280 → STM32 (I²C)**
   Le capteur BMP280 est branché sur le bus **I²C** de la STM32F411 (adresse `0x76`). La STM32 envoie une requête de lecture, récupère les registres bruts de pression et de température, puis applique les formules de **compensation de Bosch** (calibration embarquée dans le capteur) pour obtenir des valeurs en hPa et °C.

2. **STM32 → XBee émetteur (UART)**
   La STM32 formate les données compensées en une trame ASCII séparée par des points-virgules et la transmet via son **port UART** à 115 200 baud au module XBee émetteur connecté en mode **transparent AT**.

3. **XBee émetteur → XBee récepteur (radio 2.4 GHz)**
   Les deux modules XBee (même série, même PAN ID, même canal) communiquent en radio. Le module émetteur reçoit les octets UART de la STM32 et les transmet sans modification au module récepteur. Le mode transparent rend la liaison radio **invisible** pour la STM32 et le PC : ils voient simplement un lien série point à point.

4. **XBee récepteur → PC (USB/UART)**
   Le module récepteur est connecté au PC via un adaptateur USB-UART. Le système d’exploitation l’expose comme un port série virtuel (`COM3` sous Windows, `/dev/ttyUSB0` sous Linux).

5. **server.py (Flask + SQLite)**
   Le script Python ouvre le port série, lit les trames en continu dans un thread dédié, parse chaque ligne, et insère les données dans une base **SQLite** locale. Une **API REST** Flask expose les données au dashboard.

6. **Dashboard web (HTML/JS)**
   Le fichier `sensor-dashboard.html` est servi directement par Flask. Il interroge l’API toutes les **secondes** et affiche en temps réel : valeurs instantanées, MIN/MAX/MOY, graphiques temporels, tableau des dernières mesures et fréquence de trame.

---

## Architecture du dépôt

```
Projet-embarque/
├── server.py                      # Serveur Flask : lecture série, BDD SQLite, API REST
├── sensor-dashboard.html          # IHM web temps réel (Chart.js)
├── configure_xbee.py              # Configuration XBee depuis le PC (mode commande AT)
├── configure_xbee_stm32.py        # Configuration XBee via la STM32
├── WORKSPACE_F411_HAL_STM32CUBE/  # Projet STM32CubeIDE — firmware de la STM32
├── Composants.pdf                 # Fiches techniques des composants utilisés
├── requirements.txt               # Dépendances Python
└── README.md
```

---

## Matériel utilisé

| Composant | Interface | Rôle |
|---|---|---|
| **STM32F411** (Nucleo-64) | — | Microcontrôleur principal |
| **Capteur BMP280** | I²C (`0x76`) | Mesure pression (hPa) + température (°C) |
| **Module XBee émetteur** | UART 115200 baud | Reçoit les trames STM32, émet en radio |
| **Module XBee récepteur** | USB/UART | Reçoit les trames radio, les passe au PC |
| **PC** | USB | Héberge le serveur Flask et le dashboard |

Les fiches techniques détaillées sont disponibles dans [`Composants.pdf`](Composants.pdf).

---

## Format des trames

Chaque mesure est envoyée sous la forme d’une ligne ASCII :

```
<compteur>;<pression_hPa>;<temperature_C>\r\n
```

Exemple réel :
```
2632847;1014.81;31.25
```

| Champ | Description | Exemple |
|---|---|---|
| `compteur` | Valeur incrémentale du timer STM32 | `2632847` |
| `pression_hPa` | Pression atmosphérique compensée en hPa | `1014.81` |
| `temperature_C` | Température compensée en °C | `31.25` |

---

## Installation et lancement

### 1. Prérequis

- Python **3.10+**
- Modules XBee configurés en **mode transparent AT**, même PAN ID, même canal, 115 200 baud
- Port série du XBee récepteur identifié (`COM3` sous Windows, `/dev/ttyUSB0` sous Linux)
- Firmware STM32 flashé et fonctionnel

### 2. Installer les dépendances Python

```bash
pip install -r requirements.txt
```

### 3. Configurer le port série

Dans `server.py`, adapter les constantes en tête de fichier :

```python
SERIAL_PORT = "COM3"      # Windows : COMx  |  Linux : /dev/ttyUSB0
BAUD_RATE   = 115200
```

### 4. Lancer le serveur

```bash
python server.py
```

L’IHM est ensuite accessible sur **[http://localhost:5000](http://localhost:5000)**.

---

## API REST

| Méthode | Endpoint | Description |
|---|---|---|
| `GET` | `/` | Sert le dashboard HTML |
| `GET` | `/api/status` | État de la connexion série (`connected`, `last_error`) |
| `GET` | `/api/latest` | Dernière mesure enregistrée |
| `GET` | `/api/history?limit=N&offset=M` | Historique paginé (max 1000 entrées) |
| `GET` | `/api/stats` | MIN / MAX / MOY sur toute la base + fréquence de trame (trames/min) |
| `POST` | `/api/reset` | Vide toutes les mesures et remet l’auto-incrément à zéro |

---

## Dashboard web

Le fichier `sensor-dashboard.html` est une application **single-page** entièrement servie par Flask, sans build ni dépendance supplémentaire. Fonctionnalités :

- Affichage en **temps réel** de la pression et de la température (polling 1 s)
- Statistiques globales **MIN / MAX / MOY** calculées sur toute la base
- **Graphiques temporels** interactifs (Chart.js) — 50, 100 ou 200 derniers points
- **Tableau** des 50 dernières mesures avec trame brute
- Indicateur de **fréquence** de réception (trames/min sur la dernière minute)
- **Bascule thème clair/sombre**
- **Réinitialisation de la base** via modal de confirmation

---

## Problèmes rencontrés

### 1. Format de timestamp incompatible avec SQLite
**Problème** : `datetime.utcnow().isoformat()` génère `2026-05-25T13:51:00` (séparateur `T`), alors que la fonction SQLite `datetime('now', '-1 minute')` produit `2026-05-25 13:51:00` (séparateur espace). La comparaison échouait silencieusement, laissant passer **toutes les lignes** dans le filtre de la dernière minute — la **fréquence affichée était donc égale au total de mesures** et ne faisait qu’augmenter.

**Solution** : adoption de `strftime("%Y-%m-%d %H:%M:%S")` pour l’insertion des timestamps côté Python, et `.replace(' ','T')+'Z'` côté JavaScript pour reconstruire un ISO 8601 valide pour `new Date()`.

### 2. Configuration des modules XBee
**Problème** : les deux modules XBee doivent être configurés avec exactement les mêmes paramètres (PAN ID, canal RF, baud rate UART) pour établir la liaison. La procédure via XCTU était longue et source d’erreurs.

**Solution** : écriture des scripts `configure_xbee.py` (configuration directe PC → XBee) et `configure_xbee_stm32.py` (configuration en passant par la STM32) pour automatiser et reproductibiliser le paramétrage.

### 3. Statistiques calculées sur un sous-ensemble seulement
**Problème** : l’endpoint `/api/stats` contenait initialement une clause `LIMIT N` héritée d’une version précédente. Les MIN / MAX / MOY affichés ne reflétaient que les N dernières mesures au lieu de l’ensemble de la base.

**Solution** : suppression de la clause `LIMIT` dans les requêtes SQL de calcul de statistiques.

### 4. Notification persistante après réinitialisation
**Problème** : le toast « Base de données réinitialisée » restait affiché en permanence en bas à droite car le CSS utilisait `transform:translateY(120%)` sans `opacity:0`, laissant l’élément rendu et visible.

**Solution** : remplacement par `opacity:0; pointer-events:none` à l’état caché et `opacity:1; pointer-events:auto` à l’état `.show`.

---

## Firmware STM32 (WORKSPACE_F411_HAL_STM32CUBE)

Le firmware est développé avec **STM32CubeIDE** en utilisant la couche HAL.

### Fonctionnement

1. **Initialisation I²C** — configuration du bus I²C et détection du BMP280 à l’adresse `0x76`
2. **Lecture des coefficients de calibration** — le BMP280 embarque des coefficients de trim uniques par capteur, lus une seule fois au démarrage
3. **Acquisition périodique** — lecture des registres bruts de pression (`0xF7–0xF9`) et de température (`0xFA–0xFC`)
4. **Compensation** — application des formules de la datasheet Bosch BMP280 pour convertir les valeurs ADC en hPa et °C
5. **Formatage et envoi UART** — la trame `compteur;pression;temperature\r\n` est envoyée à **115 200 baud** vers le module XBee émetteur

---

## Auteurs

**Nicolas Cipresso**, **Fabien Struillou**, **Jérémy Georges-Durand** — ENIB, 2025–2026
