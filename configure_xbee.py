import serial
import time

port = "COM5"  # ← ton port STM32/shield
baud = 9600

with serial.Serial(port, baud, timeout=2) as ser:
    time.sleep(1)
    
    # Entrée en mode AT
    ser.write(b'+++')
    time.sleep(1.5)
    print(ser.read_all().decode('ascii', errors='ignore'))  # OK
    
    commandes = [
        b'ATID1234\r',
        b'ATDL0000FFFF\r',
        b'ATDH00000000\r',
        b'ATBD3\r',
        b'ATWR\r',
        b'ATCN\r',
    ]
    
    for cmd in commandes:
        ser.write(cmd)
        time.sleep(0.5)
        reponse = ser.read_all().decode('ascii', errors='ignore')
        print(f"{cmd.decode().strip()} → {reponse.strip()}")