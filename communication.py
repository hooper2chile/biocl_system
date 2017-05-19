#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys, zmq, time
#5556: for download data
#5557: for upload data

#emit command for actuation zmq server (publisher): 'wph14.0feed100unload100mix100temp100rst111111dir111111\n'
#set_data = [ dato['alimentar'], dato['mezclar'], dato['ph'], dato['descarga'], dato['temperatura'], dato['alimentar_rst'], dato['mezclar_rst'], dato['ph_rst'], dato['descarga_rst'], dato['temperatura_rst'], dato['alimentar_dir'], dato['ph_dir'], dato['temperatura_dir'] ]

temp_save_set_data = None
tau_zmq_connect = 0.3 #0.3 [s]: no ha funcionado con menos
SPEED_MAX_MIX = 1500
SPEED_MAX = 100
TEMP_MAX = 100
PH_MIN = 0
PH_MAX = 14

#download data measures with client zmq
def published_setpoint(set_data):
    #Publisher set_data commands
    port = "5556"
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:%s" % port)
    topic   = 'w'
    #espero y envio valor
    time.sleep(tau_zmq_connect)
    socket.send_string("%s %s" % (topic, set_data))

    return True


def zmq_client():
    #####Listen measures
    port_sub = "5557"
    context_sub = zmq.Context()
    socket_sub = context_sub.socket(zmq.SUB)
    socket_sub.connect ("tcp://localhost:%s" % port_sub)
    topicfilter = "w"
    socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)
    #espero y retorno valor
    time.sleep(tau_zmq_connect)
    return socket_sub.recv()


def send_setpoint(set_data):
    #format string

    for i in range(5,13):
        if set_data[i] is True:
            set_data[i] = 1
        else:
            set_data[i] = 0


    string_rst = str(set_data[5]) + str(set_data[6]) + str(set_data[7]) + str(set_data[8]) + str(set_data[9]) + '1'
    string_dir = str(set_data[10])+ str(set_data[11])+ str(set_data[12]) + '111'


    global temp_save_set_data

    try:
        set_data[0] = int(set_data[0])   #alimentar
        set_data[1] = int(set_data[1])   #mezclar
        set_data[2] = float(set_data[2]) #ph
        set_data[3] = int(set_data[3])   #descarga
        set_data[4] = int(set_data[4])   #temperatura

        #rst setting
        set_data[5] = int(set_data[5])  #alimentar_rst
        set_data[6] = int(set_data[6])  #mezclar_rst
        set_data[7] = int(set_data[7])  #ph_rst
        set_data[8] = int(set_data[8])  #descarga_rst
        set_data[9] = int(set_data[9])  #temperatura_rst

        #dir setting
        set_data[10]= int(set_data[10]) #alimentar_dir
        set_data[11]= int(set_data[11]) #ph_dir
        set_data[12]= int(set_data[12]) #temperatura_dir

        temp_save_set_data = set_data

    except ValueError:
        set_data = temp_save_set_data #esto permite reenviar el ultimo si hay una exception
        print "exception de set_data"
        #set_data = [10,10,10,10,10,1,1,1,1,1,1,1,1]


    #threshold setting:
    #alimentar
    if set_data[0] > SPEED_MAX_MIX:
        set_data[0] = SPEED_MAX_MIX

    elif set_data[0] < 0:
        set_data[0] = 0

    #Mezclar
    if set_data[1] > SPEED_MAX:  #cambiar a SPEED_MAX_MIX cuando este listo el protocolo que tambien hay que modificar
        set_data[1] = SPEED_MAX

    elif set_data[1] < 0:
        set_data[1] = 0

    #ph
    if set_data[2] > PH_MAX:
        set_data[2] = PH_MAX

    elif set_data[2] < PH_MIN:
        set_data[2] = PH_MIN

    #descarga
    if set_data[3] > SPEED_MAX:
        set_data[3] = SPEED_MAX

    elif set_data[3] < 0:
        set_data[3] = 0

    #temperatura
    if set_data[4] > TEMP_MAX:
        set_data[4] = TEMP_MAX

    elif set_data[4] < 0:
        set_data[4] = 0




    #format seetting:
    if set_data[0] < 10:
        string_feed = '00' + str(set_data[0])
    elif set_data[0] < 100:
        string_feed = '0'  + str(set_data[0])
    else:
        string_feed = str(set_data[0])



    if set_data[1] < 10:
        string_mix = '000' + str(set_data[1])
    elif set_data[1] < 100:
        string_mix = '00'  + str(set_data[1])
    elif set_data[1] < 1000:
        string_mix = '0'   + str(set_data[1])
    else:
        string_mix = str(set_data[1])



    ph_dec = str(set_data[2]-int(set_data[2]))[1:]
    if ph_dec == '.0':
        if set_data[2] < 10:
            string_ph = '0' + str(set_data[2])
        else:
            string_ph = str(set_data[2])

    else:
        if set_data[2] < 10:
            string_ph = '0' + str(set_data[2])
        else:
            string_ph = str(set_data[2])



    if set_data[3] < 10:
        string_unload = '00' + str(set_data[3])
    elif set_data[3] < 100:
        string_unload = '0'  + str(set_data[3])
    else:
        string_unload = str(set_data[3])



    if set_data[4] < 10:
        string_temp = '00' + str(set_data[4])
    elif set_data[4] < 100:
        string_temp = '0'  + str(set_data[4])
    else:
        string_temp = str(set_data[4])



    command = 'wph' + string_ph + 'feed' + string_feed + 'unload' + string_unload + 'mix' + string_mix + \
             'temp' + string_temp + 'rst' + string_rst + 'dir' + string_dir + '\n'

    print('\n' + command + '\n')

    #write for serial port
    published_setpoint(command)

    try:
        f = open("command.txt","a+")
        f.write(command)
        f.close()

    except OSError:
        print("no se pudo guardar el command en el archivo de texto")

    return True



def main():
    while True:
        print("communication.py")


if __name__ == '__main__':
    main()
