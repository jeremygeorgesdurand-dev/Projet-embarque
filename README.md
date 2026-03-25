# Projet-embarque

## Description

Ce dépôt contient le projet d'embarqué réalisé dans le cadre du module **Objets Connectés / Projet embarqué ENIB**.

L’objectif est de mettre en place une chaîne complète de mesure et de communication :

- Capteur de pression / température **BMP280** connecté en **I²C** à une carte **STM32 Nucleo‑F411RE**.
- Transmission des mesures vers un PC via :
  - l’UART relié au **ST‑LINK** (port série virtuel USB),
  - un module **Zigbee XBee Pro XBP24‑AWI** monté sur un shield XBee (liaison sans fil).
- Affichage et exploitation des données sur PC via une **IHM** :
  - affichage numérique (2 décimales),
  - affichage graphique temps réel,
  - stockage en **base de données** (SQLite),
  - interrogation de la base depuis l’IHM.

Le développement embarqué est fait en C avec STM32 HAL, piloté depuis **VS Code**, et le projet est intégralement versionné sur **GitHub**.

## Matériel

- Carte de développement **STM32 Nucleo‑F411RE** (Cortex‑M4F, 512 Ko Flash, 128 Ko RAM, connecteurs Arduino + Morpho, ST‑LINK intégré).
- Capteur de pression / température **BMP280** (interface numérique I²C).
- Shield **XBee** compatible Arduino, avec module **XBee Pro XBP24‑AWI** (802.15.4 / Zigbee, interface UART 3,3 V).
- PC de développement :
  - VS Code + toolchain ARM (`arm-none-eabi-gcc`, Make/CMake, OpenOCD ou STM32CubeProgrammer),
  - Python 3 avec PySerial, PyQt5/PySide6, SQLite3, Matplotlib ou PyQtGraph.

## Architecture générale

```text
[BMP280] --(I²C)--> [STM32 Nucleo F411RE]
                      |         |
                      |         +--(UART)--> [XBee Pro XBP24-AWI] ~~~ (sans fil) ~~~> [XBee USB -> PC]
                      |
                      +--(UART ST-LINK)--> [PC via USB]

[PC] : IHM graphique + base de données (SQLite)
```

Le STM32 joue le rôle de nœud capteur :

- acquisition périodique du BMP280 via I²C,
- formatage des mesures dans une trame texte (`TS;P;T`),
- envoi des trames sur l’UART ST‑LINK et sur l’UART connecté au module XBee.

Le PC reçoit ces trames sur un port série (ST‑LINK, XBee, ou plus tard Bluetooth) et les traite dans l’IHM.

## Objectifs du projet

Objectifs principaux (résumé de la feuille d’objectifs ENIB) :

- Récupérer la donnée du capteur dans le STM32.
- Afficher la donnée du capteur dans un terminal via l’UART ST‑LINK.
- Afficher la donnée du capteur sur une IHM côté PC.
- Afficher la donnée avec **2 décimales**.
- Afficher la donnée sous forme **graphique** sur l’IHM.
- Stocker les données dans une **base de données** côté PC.
- Pouvoir **interroger la base** depuis l’IHM.
- Mettre en œuvre une **liaison Zigbee** (avec IHM côté PC).
- Mettre en œuvre une **liaison Bluetooth** (avec IHM côté PC, si réalisée).
- Utiliser des outils de **machine / deep learning** pour classifier la valeur du capteur (objectif avancé).

Livrables attendus supplémentaires :

- Chronogramme obtenu avec un **analyseur logique** pour observer les trames entre le capteur et le microcontrôleur (I²C, éventuellement UART).
- **Présentation orale** de 15–20 minutes présentant l’architecture, la réalisation et les résultats.

## Organisation du dépôt

Proposition d’arborescence (évolutive) :

```text
.
├── WORKSPACE_F411_HAL_STM32CUBE/   # Projet CubeIDE / HAL pour Nucleo F411RE
│   └── ...
├── pc_ihm/                         # Application PC (Python)
│   ├── main.py
│   ├── serial_worker.py
│   ├── db.py
│   ├── plot_widget.py
│   └── requirements.txt
├── doc/
│   ├── objectifs.pdf               # Feuille d’objectifs ENIB
│   ├── architecture.md             # Schémas bloc, notes d’architecture
│   ├── chronogrammes/              # Captures analyseur logique
│   └── presentation/               # Slides de la soutenance
└── README.md
```

## Plan de travail (roadmap)

1. **Bring‑up BMP280**
   - Configuration I²C sur la Nucleo‑F411RE.
   - Intégration d’une librairie BMP280 pour STM32 (HAL I²C).
   - Lecture de l’ID du capteur, des registres de calibration, puis obtention de valeurs de pression / température cohérentes.

2. **Trames UART ST‑LINK**
   - Formatage des mesures sous forme de chaîne texte (`TS;P;T`).
   - Envoi périodique sur l’UART relié au ST‑LINK.
   - Validation avec un terminal série sur PC.

3. **Script PC minimal**
   - Script Python qui lit le port série, parse les trames et affiche les valeurs dans la console.

4. **IHM PC + base de données**
   - Fenêtre graphique (PyQt) avec :
     - affichage des dernières valeurs (2 décimales),
     - graphe temps réel de la pression / température.
   - Stockage automatique dans une base SQLite.
   - Onglet pour interroger la base (plage de dates, source, etc.) et afficher les résultats.

5. **Intégration Zigbee (XBee)**
   - Configuration d’un second UART sur le STM32 vers le module XBee.
   - Tests de bout en bout STM32 ↔ XBee ↔ PC.
   - IHM capable de choisir la source (ST‑LINK ou XBee).

6. **Bluetooth et ML (optionnel)**
   - Ajout d’un module Bluetooth série traité comme un port COM supplémentaire.
   - Script de classification simple (scikit‑learn, etc.) s’appuyant sur les données enregistrées.

7. **Chronogrammes et soutenance**
   - Capture des échanges I²C BMP280 <-> STM32 et éventuellement des trames UART.
   - Intégration des chronogrammes commentés dans la documentation.
   - Préparation de la présentation orale (15–20 min) : contexte, architecture, démos, bilan.

## Auteur

- Jérémy Georges-Durand – Étudiant ENIB
