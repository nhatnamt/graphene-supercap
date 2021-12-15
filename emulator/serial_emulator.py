import time
import serial
import threading
import random

ser = serial.Serial('COM3',4800,timeout=0.1)
count = 0
running = 1
print('Connected to: ',ser.name, 'Press any key to exit')

def serialTX():
    global running
    while running:
        tx_len = random.randint(10,124)
        packet = bytearray()
        packet.append(0x19)     # Start flag MSB
        packet.append(0x94)     # Start flag LSB
        packet.append(0x00)     # Packet length MSB
        packet.append(tx_len)   # Packet length LSB
        packet.append(0x00)     # Protocol - ignored
        packet.append(0x08)     # Device ID MSB
        packet.append(0x86)     # Device ID LSB
        packet.append(0x00)     # Message ID - ignored
        packet.append(0x00)
        packet.append(0x00)
        packet.append(0x01)     # Transfer/Response Code - 0x01 for issure data

        # 11 bytes before actual data
        for i in range(tx_len-2):
            packet.append(random.randint(1,255))
        ser.write(packet)
        print("Packet sent: ",packet.hex())
        print("Random length: ",tx_len+11)
        time.sleep(1.5)
    ser.close()

def serialRX():
    global running
    while running:
        if ser.in_waiting:
            while ser.in_waiting:
                data_in = ser.readline().decode().strip()
                print (data_in)
            print(' ')

def exitProgram() :
    global running
    key = input()
    running = 0

tx = threading.Thread(target=serialTX)
rx = threading.Thread(target=serialRX)
ext = threading.Thread(target=exitProgram)

tx.start()
rx.start()
ext.start()