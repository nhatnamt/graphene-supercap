import time
import serial
import threading

ser = serial.Serial('COM3',4800,timeout=0.1)
count = 0
running = 1
print('Connected to: ',ser.name, 'Press any key to exit')

def serialTX():
    global running
    while running:
        packet = bytearray()
        packet.append(0x19)
        packet.append(0x94)
        packet.append(0)
        packet.append(0x7C) #packet length
        packet.append(0)
        packet.append(0x08)
        packet.append(0x86)
        ser.write(packet)
        print("Packet sent: ",packet.hex())
        time.sleep(0.1)
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