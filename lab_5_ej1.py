import serial
import time

# Configuración del puerto serial
SERIAL_PORT = '/dev/serial0'  # Puerto serial en la Raspberry Pi
BAUD_RATE = 9600              # Baud rate (debe coincidir con el de la Tiva)

try:
    # Inicializar el puerto serial
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1  # Tiempo de espera para leer datos
    )

    print(f"Conectado al puerto serial {SERIAL_PORT} a {BAUD_RATE} baudios.")

    # Bucle principal para recibir datos
    while True:
        if ser.in_waiting > 0:  # Verificar si hay datos disponibles
            data = ser.readline().decode('utf-8').strip()  # Leer y decodificar los datos
            print(f"Dato recibido: {data}")

            # Aquí puedes agregar lógica para procesar los datos
            if data == "motor_1":
                print("Acción: Motor 1 activado")
            elif data == "motor_2":
                print("Acción: Motor 2 activado")

except serial.SerialException as e:
    print(f"Error en la comunicación serial: {e}")

finally:
    # Cerrar el puerto serial al finalizar
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Puerto serial cerrado.")