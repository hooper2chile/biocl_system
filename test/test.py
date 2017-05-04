import zmq, time, serial, sys

#5556: for listen data
#5557: for publisher data


def rs232(q1,q2):

    if sys.platform=='darwin':
        ser = serial.Serial(port='/dev/cu.wchusbserial1420', baudrate=9600)
    else:
        ser = serial.Serial(port='/dev/ttyACM0', baudrate=9600)

    time.sleep(1)
    print "SERIAL OPEN: ", ser.is_open

    #####Listen part
    port_sub = "5556"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    string = ['','','','']

    #####Publisher part
    port_pub = "5557"
    context_pub = zmq.Context()
    socket_pub = context_pub.socket(zmq.PUB)
    socket_pub.bind("tcp://*:%s" % port_pub)
    topic   = 'w'
    action_read = "read"


    while True:
        try:
            string= socket_sub.recv(flags=zmq.NOBLOCK).split()

        except zmq.Again:
            pass




            socket_pub.send_string("%s %s" % (topic, data))










        while True:
            try:
                if actions == "read":
                    try:
                        if ser.is_open:
                            ser.write('r'+'\n')
                            SERIAL_DATA = ser.readline()
                        else:
                            ser.open()
                    except:
                        print "no se pudo leer SERIAL_DATA del uc"

                else:
                    try:
                        ser.write(actions+'\n')
                        result = ser.readline().split()
                        print result

                    except:
                        print "no se pudo escribir al uc"

            except:
                print "se entro al while pero no se pudo revisar la cola"


        return True






if __name__ == "__main__":
