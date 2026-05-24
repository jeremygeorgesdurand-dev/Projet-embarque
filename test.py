import serial

port = "COM5"   # ← ton XBee PC
baud = 9600

ser = serial.Serial(port, baud, timeout=2)
print(f"En écoute sur {port}...")

while True:
    line = ser.readline().decode('ascii', errors='ignore').strip()
    if line:
        parts = line.split(';')
        if len(parts) == 3:
            print(f"Tick: {parts[0]} | Pression: {parts[1]} hPa | Temp: {parts[2]} °C")
        else:
            print(f"Reçu: {line}")