from multiprocessing import Process, Queue, Event
import zmq, time, serial, sys

#5556: for listen data
#5557: for publisher data

tau_zmq_connect     = 0.5   # 0.3=300 [ms]
tau_zmq_while_write = 0.5   # 0.3=300 [ms]
tau_zmq_while_read  = 0.5   # 0.3=300 [ms]
tau_serial          = 0.02  #  0.01=10 [ms]

##### Queue data: q1 is for put data to   serial port #####
##### Queue data: q2 is for get data from serial port #####
def listen(q1):
    #####Listen part
    port_sub = "5556"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    time.sleep(tau_zmq_connect)

    string = ['','','','']
    while True:
        try:
            string= socket_sub.recv(flags=zmq.NOBLOCK).split()
            q1.put(string[1])

        except zmq.Again:
            pass

        time.sleep(tau_zmq_while_write)

    return True


##### Queue data: q1 is for put data to   serial port #####
##### Queue data: q2 is for get data from serial port #####
def speak(q1,q2):
    #####Publisher part
    port_pub = "5557"
    context_pub = zmq.Context()
    socket_pub = context_pub.socket(zmq.PUB)
    socket_pub.bind("tcp://*:%s" % port_pub)
    topic   = 'w'
    time.sleep(tau_zmq_connect)

    while True:
        q1.put("read")

        if not q2.empty():
            data = q2.get()
            socket_pub.send_string("%s %s" % (topic, data))

        time.sleep(tau_zmq_while_read) #Tiempo de muestreo menor para todas las aplicaciones que recogen datos por ZMQ.

    return True



def rs232(q1,q2):
    flag = False
    while not flag:
        try:
            if sys.platform=='darwin':
                ser = serial.Serial(port='/dev/cu.wchusbserial1420', baudrate=9600)
            else:
                ser = serial.Serial(port='/dev/ttyUSB0', baudrate=9600)

            #necesario para setear correctamente el puerto serial
            ser.setDTR(True)
            time.sleep(1)
            ser.setDTR(False)
            time.sleep(1)

            flag = ser.is_open
            print "CONEXION SERIAL EXITOSA"

        except serial.SerialException:
            print "conexion serial no realizada"
            flag = False
            time.sleep(2)


    if ser.is_open:
        #commanda start:  wph14.0feed100unload100mix100temp100rst111111dir111111
        ser.write('wph00.0feed000unload000mix0000temp000rst111111dir111111'+'\n')
        result = ser.readline().split()
        print result


        while True:
            try:
                if not q1.empty():
                    action = q1.get()

                    #Action for read measure from serial port
                    if action == "read":
                        try:
                            if ser.is_open:
                                ser.write('r'+'\n')
                                SERIAL_DATA = ser.readline()
                                q2.put(SERIAL_DATA)

                            else:
                                ser.open()
                        except:
                            print "no se pudo leer SERIAL_DATA del uc"

                    #Action for write command to serial port
                    else:
                        try:
                            ser.write(action+'\n')
                            result = ser.readline().split()
                            print result

                        except:
                            print "no se pudo escribir al uc"


                elif q1.empty():
                    time.sleep(tau_serial)

            except:
                print "se entro al while pero no se pudo revisar la cola"

            time.sleep(tau_serial)

        #return True



def main():
    q1 = Queue()
    q2 = Queue()

    p0 = Process(target=rs232, args=(q1,q2))
    p0.start()

    p1 = Process(target=listen, args=(q1,))
    p1.start()

    p2 = Process(target=speak, args=(q1, q2))
    p2.start()


if __name__ == "__main__":
    main()
