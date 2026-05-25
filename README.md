# Projet Embarqué — Capteur BMP280 + XBee + Dashboard Web

> Projet réalisé dans le cadre du cursus ingénieur à l'ENIB (École Nationale d'Ingénieurs de Brest).

## Présentation générale

Ce projet met en œuvre une chaîne complète d'acquisition de données environnementales :

1. Un **capteur BMP280** (pression atmosphérique + température) connecté à une **STM32F411**
2. La STM32 transmet les données via **UART** à un **module XBee** émetteur
3. Un second **module XBee** récepteur est connecté à un PC
4. Un **serveur Python (Flask)** reçoit les trames, les stocke en **SQLite** et expose une **API REST**
5. Un **dashboard web** (HTML/JS/Chart.js) affiche les données en temps réel

```
[STM32 + BMP280] ──UART──► [XBee TX] ~~~radio~~~ [XBee RX] ──USB/UART──► [PC : server.py] ──► [Dashboard Web]
```

---

## Architecture du projet

```
Projet-embarque/
├── server.py                    # Serveur Flask : lecture série, BDD, API REST
├── sensor-dashboard.html        # IHM web (dashboard temps réel)
├── configure_xbee.py            # Script de configuration XBee (PC seul)
├── configure_xbee_stm32.py      # Script de configuration XBee via STM32
├── WORKSPACE_F411_HAL_STM32CUBE/ # Projet STM32CubeIDE (firmware STM32)
├── Composants.pdf               # Fiche technique des composants utilisés
├── requirements.txt             # Dépendances Python
└── README.md
```

---

## Matériel requis

| Composant | Rôle |
|---|---|
| STM32F411 (Nucleo ou custom) | Microcontrôleur principal |
| Capteur BMP280 | Mesure pression (hPa) et température (°C) via I²C |
| 2× Module XBee (série 1 ou 2) | Transmission radio sans fil |
| PC avec port USB/UART | Réception des trames + hébergement du serveur |

---

## Format des trames

La STM32 envoie une trame par mesure sur le port série, au format :

```
<compteur>;<pression_hPa>;<temperature_C>\r\n
```

Exemple :
```
2632847;1014.81;31.25
```

- `compteur` : valeur brute incrémentale (issu du timer STM32)
- `pression_hPa` : pression atmosphérique en hPa (ex. `1014.81`)
- `temperature_C` : température en °C (ex. `31.25`)

---

## Installation et lancement

### 1. Prérequis

- Python **3.10+**
- Modules XBee configurés en **mode transparent AT** au même baud rate
- Port série identifié (ex. `COM3` sous Windows, `/dev/ttyUSB0` sous Linux)

### 2. Installer les dépendances

```bash
pip install -r requirements.txt
```

### 3. Configurer le port série

Dans `server.py`, modifier les constantes en tête de fichier :

```python
SERIAL_PORT = "COM3"      # Windows : COMx  |  Linux : /dev/ttyUSB0
BAUD_RATE   = 115200
```

### 4. Lancer le serveur

```bash
python server.py
```

L'IHM est ensuite accessible sur **http://localhost:5000**

---

## API REST

| Méthode | Endpoint | Description |
|---|---|---|
| `GET` | `/` | Sert le dashboard HTML |
| `GET` | `/api/status` | État de la connexion série |
| `GET` | `/api/latest` | Dernière mesure reçue |
| `GET` | `/api/history?limit=N&offset=M` | Historique paginé (max 1000) |
| `GET` | `/api/stats` | MIN/MAX/AVG sur toute la base + fréquence/min |
| `POST` | `/api/reset` | Vide toutes les mesures (remet l'ID à 0) |

---

## Dashboard web

Le fichier `sensor-dashboard.html` est une application **single-page** sans dépendance serveur autre que Flask. Elle utilise :

- **Chart.js** (CDN) pour les graphiques temps réel
- **Google Fonts** (JetBrains Mono + Inter) pour la typographie
- Polling toutes les **1 seconde** vers l'API
- Thème **clair/sombre** avec bascule
- Bouton **Réinitialiser DB** avec modal de confirmation

---

## Problèmes rencontrés

### 1. Format de timestamp incompatible avec SQLite
**Problème** : `datetime.utcnow().isoformat()` produisait `2026-05-25T13:51:00` (avec `T`), alors que SQLite `datetime('now', '-1 minute')` compare avec le format `2026-05-25 13:51:00` (avec espace). La comparaison échouait silencieusement : toutes les lignes passaient le filtre, ce qui faisait afficher la **fréquence = total de mesures**.

**Solution** : utiliser `strftime("%Y-%m-%d %H:%M:%S")` pour l'insertion, et côté JS `.replace(' ','T')+'Z'` pour reconstruire un ISO valide pour `new Date()`.

### 2. Configuration des modules XBee
**Problème** : les deux modules XBee devaient être configurés avec les mêmes paramètres (PAN ID, canal, baud rate) pour communiquer. La configuration via XCTU + scripts Python a été nécessaire.

**Solution** : scripts `configure_xbee.py` et `configure_xbee_stm32.py` pour automatiser la configuration.

### 3. Données manquantes dans les stats (MIN/MAX/MOY)
**Problème** : l'API `/api/stats` calculait les statistiques uniquement sur les N dernières mesures au lieu de toute la base.

**Solution** : suppression du `LIMIT` dans la requête SQL pour travailler sur l'intégralité de la table.

### 4. Toast persistant après réinitialisation de la base
**Problème** : le toast de confirmation restait affiché indéfiniment en bas à droite après un reset, car le CSS utilisait `transform:translateY(120%)` sans `opacity:0`, laissant l'élément visible.

**Solution** : passage à `opacity:0; pointer-events:none` par défaut, et `opacity:1; pointer-events:auto` pour l'état `.show`.

---

## Firmware STM32 (WORKSPACE_F411_HAL_STM32CUBE)

Le firmware est développé avec **STM32CubeIDE** en utilisant la couche HAL.

Fonctionnalités implémentées :
- Lecture du capteur **BMP280** via **I²C** (adresse `0x76`)
- Compensation des données brutes (pression + température) selon la datasheet BMP280
- Transmission de la trame formatée via **UART** à 115 200 baud
- Envoi périodique (timer ou boucle principale)

---

## Auteur

Jérémy Georges-Durand — ENIB, 2025–2026
