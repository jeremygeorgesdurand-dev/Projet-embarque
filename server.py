#!/usr/bin/env python3
"""
server.py — Lecture XBee UART → SQLite + API REST Flask
Trame attendue : <raw>;pression_hPa;temperature_C\r\n
Ex.            : 219861;1014.87;30.20
Lancer : python server.py
IHM    : http://localhost:5000
"""

import serial
import sqlite3
import threading
import time
import re
from datetime import datetime
from flask import Flask, jsonify, send_from_directory, request
import os

# ── CONFIG ──────────────────────────────────────────────────────────────────
SERIAL_PORT = "COM3"        # Linux: /dev/ttyUSB0
BAUD_RATE   = 115200
DB_PATH     = "sensor_data.db"
HOST        = "0.0.0.0"
PORT        = 5000
STATIC_DIR  = os.path.dirname(os.path.abspath(__file__))

# ── BASE DE DONNÉES ──────────────────────────────────────────────────────────
def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS measurements (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp   TEXT    NOT NULL,
            raw         TEXT    NOT NULL,
            pressure    REAL,
            temperature REAL
        )
    """)
    conn.commit()
    conn.close()

def insert_measurement(raw: str, pressure: float | None, temperature: float | None):
    conn = sqlite3.connect(DB_PATH)
    conn.execute(
        "INSERT INTO measurements (timestamp, raw, pressure, temperature) VALUES (?, ?, ?, ?)",
        (datetime.utcnow().isoformat(), raw, pressure, temperature)
    )
    conn.commit()
    conn.close()

# ── PARSING ──────────────────────────────────────────────────────────────────
def parse_line(line: str) -> tuple[float | None, float | None]:
    """
    Format attendu : anything;pression;temperature
    Ex.            : 219861;1014.87;30.20
    Retourne (pressure, temperature) ou (None, None) si invalide.
    """
    line = line.strip()
    parts = line.split(";")
    if len(parts) >= 3:
        try:
            pressure    = float(parts[1])
            temperature = float(parts[2])
            return pressure, temperature
        except ValueError:
            pass
    return None, None

# ── THREAD SÉRIE ─────────────────────────────────────────────────────────────
serial_status = {"connected": False, "last_error": ""}

def serial_reader():
    while True:
        try:
            with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2) as ser:
                serial_status["connected"] = True
                serial_status["last_error"] = ""
                print(f"[SERIAL] Connecté sur {SERIAL_PORT} @ {BAUD_RATE} baud")
                while True:
                    raw = ser.readline().decode("utf-8", errors="ignore").strip()
                    if not raw:
                        continue
                    pressure, temperature = parse_line(raw)
                    insert_measurement(raw, pressure, temperature)
                    print(f"[DATA] {raw}  →  P={pressure} hPa  T={temperature}°C")
        except serial.SerialException as e:
            serial_status["connected"] = False
            serial_status["last_error"] = str(e)
            print(f"[SERIAL] Erreur : {e} — nouvelle tentative dans 5 s")
            time.sleep(5)

# ── FLASK API ────────────────────────────────────────────────────────────────
app = Flask(__name__, static_folder=STATIC_DIR)

@app.route("/")
def index():
    return send_from_directory(STATIC_DIR, "sensor-dashboard.html")

@app.route("/api/status")
def api_status():
    return jsonify(serial_status)

@app.route("/api/latest")
def api_latest():
    conn = sqlite3.connect(DB_PATH)
    row = conn.execute(
        "SELECT id, timestamp, raw, pressure, temperature FROM measurements ORDER BY id DESC LIMIT 1"
    ).fetchone()
    conn.close()
    if row:
        return jsonify({"id": row[0], "timestamp": row[1], "raw": row[2],
                        "pressure": row[3], "temperature": row[4]})
    return jsonify(None)

@app.route("/api/history")
def api_history():
    limit  = min(int(request.args.get("limit",  100)), 1000)
    offset = int(request.args.get("offset", 0))
    conn = sqlite3.connect(DB_PATH)
    rows = conn.execute(
        "SELECT id, timestamp, raw, pressure, temperature FROM measurements "
        "ORDER BY id DESC LIMIT ? OFFSET ?",
        (limit, offset)
    ).fetchall()
    total = conn.execute("SELECT COUNT(*) FROM measurements").fetchone()[0]
    conn.close()
    data = [{"id": r[0], "timestamp": r[1], "raw": r[2],
             "pressure": r[3], "temperature": r[4]} for r in rows]
    return jsonify({"total": total, "data": data})

@app.route("/api/stats")
def api_stats():
    n = int(request.args.get("n", 100))
    conn = sqlite3.connect(DB_PATH)
    def stat(col):
        row = conn.execute(f"""
            SELECT MIN({col}), MAX({col}), AVG({col}), COUNT(*)
            FROM (SELECT {col} FROM measurements
                  WHERE {col} IS NOT NULL ORDER BY id DESC LIMIT ?)
        """, (n,)).fetchone()
        return row
    rp = stat("pressure")
    rt = stat("temperature")
    conn.close()
    return jsonify({
        "pressure":    {"min": rp[0], "max": rp[1], "avg": round(rp[2], 3) if rp[2] else None, "count": rp[3]},
        "temperature": {"min": rt[0], "max": rt[1], "avg": round(rt[2], 3) if rt[2] else None, "count": rt[3]}
    })

if __name__ == "__main__":
    init_db()
    t = threading.Thread(target=serial_reader, daemon=True)
    t.start()
    print(f"[WEB] IHM disponible sur http://localhost:{PORT}")
    app.run(host=HOST, port=PORT, debug=False)
