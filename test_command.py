import serial, time, sys

if sys.platform=='darwin':
  ser = serial.Serial(port='/dev/cu.wchusbserial1410', baudrate=9600)
else:
  ser = serial.Serial(port='/dev/ttyUSB0', baudrate=9600)

time.sleep(1) #necesario para setear correctamente el puerto serial
print "SERIAL OPEN: ", ser.is_open



ser.write('wph00.0feed100unload100mix1500temp100rst111111dir111111'+'\n')
result = ser.readline().split()
print result

time.sleep(1)


ser.write('wph00.0feed050unload050mix1000temp050rst000000dir000000'+'\n')
result = ser.readline().split()
print result
