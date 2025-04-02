import RPi.GPIO as GPIO
import time
import serial  

# Configuración de UART
try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)  
except serial.SerialException:
    print(" No se pudo abrir el puerto UART. Verifica la conexión.")
    exit()


TRIG = 23  
ECHO = 24  

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(TRIG, GPIO.OUT)
    GPIO.setup(ECHO, GPIO.IN)

def medir_distancia():
    GPIO.output(TRIG, 0)
    time.sleep(0.00002)
    GPIO.output(TRIG, 1)
    time.sleep(0.00001)
    GPIO.output(TRIG, 0)

    while GPIO.input(ECHO) == 0:
        pass
    time1 = time.time()

    while GPIO.input(ECHO) == 1:
        pass
    time2 = time.time()    

    duracion = time2 - time1
    distancia = round((duracion * 343) / 2 * 100, 2)
    return int(distancia)

def enviar_estado(distancia):
    if distancia <= 7 :
        estado = 0
    else:
        estado = 1
    
    # Envía el estado (0 o 1) como byte
    ser.write(str(estado).encode())
    print(f"Enviando: {estado} (Distancia: {distancia} cm)")

def loop():
    while True:
        dis = medir_distancia()
        enviar_estado(dis)
        time.sleep(0.8)

def destroy():
    GPIO.cleanup()
    ser.close()

if __name__ == "__main__":
    setup()
    try:
        loop()
    except KeyboardInterrupt:
        print("\nSaliendo del programa...")
        destroy()