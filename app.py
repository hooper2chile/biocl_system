#!/usr/bin/env python
# -*- coding: utf-8 -*-
from flask import Flask, render_template, session, request, Response #, send_from_directory
from flask_socketio import SocketIO, emit, disconnect

import os, sys, communication, reviewDB

# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
async_mode = None

#app = Flask(__name__)
app = Flask(__name__, static_url_path="")
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread1 = None

ph_set = [0,0,0,0]
od_set = [0,0,0,0]
temp_set = [0,0,0,0]
set_data = [0,0,0,0,0,1,1,1,1,1,0,0,0]

#CONFIGURACION DE PAGINAS WEB
@app.route('/', methods=['GET', 'POST'])
def index():
    return render_template("index.html", title_html="Configuracion de Procesos")


@app.route('/graphics')
def graphics():
    return render_template('graphics.html', title_html="Variables de Proceso")


@app.route('/dbase', methods=['GET', 'POST'])
def viewDB():
    return render_template('dbase.html', title_html="Data Logger")


@app.route('/calibrar', methods=['GET', 'POST'])
def calibrar():
    return render_template('calibrar.html', title_html="Calibrar")



@app.route('/descargar')
def descargar():
    return "<br>".join( os.listdir("./database") )

'''
@app.route('/descargar/<path:path>')
def descargar_csv(path):
    #convert path to path2:
    path2 = os.path.splitext(path)[0]+'.cvs'
    print path2
    os.system('sqlite3 -header -csv ./database/%s "select * from ph;" > ./database2/%s' % (path,path2) )
    return send_from_directory('./database2', path2)

@app.route('/descargar')
def csv_get():
    testa = []
    testb = []
    for i in range(1,len(APIRest)):
        testa.append(str(APIRest[i][0]))
        testb.append(str(APIRest[i][1]))

    csv = []
    for i in range(1, len(APIRest)):
        #csv[i] = testa[i] + testb[i]
        csv = '1\n'

    return Response(
                        csv,
                        mimetype="text/csv",
                        headers={"Content-disposition":
                                 "attachment; filename=myplot.csv"}
                    )
'''

#CONFIGURACION DE FUNCIONES SocketIO
#Connect to the Socket.IO server. (Este socket es OBLIGACION)
@socketio.on('connect', namespace='/biocl')
def function_thread():
    print "\n Cliente Conectado al Thread del Bioreactor\n"

    #Se emite durante la primera conexión de un cliente el estado actual de los setpoints
    emit('Setpoints',     {'set': set_data})
    emit('ph_calibrar',   {'set': ph_set})
    emit('od_calibrar',   {'set': od_set})
    emit('temp_calibrar', {'set': temp_set})

    global thread1
    if thread1 is None:
        thread1 = socketio.start_background_task(target=background_thread1)


N = None
APIRest = None
@socketio.on('my_json', namespace='/biocl')
def my_json(dato):

    dt  = int(dato['dt'])
    var = dato['var']

    try:
        f = open("window.txt","a+")
        f.write(dato['var'] + ' ' + dato['dt'] +'\n')
        f.close()

    except:
        print "no se logro escribir la ventana solicitada en el archivo window.txt"

    #Se buscan los datos de la consulta en database
    try:
        f = open("name_db.txt",'r')
        filedb = f.readlines()[-1][:-1]
        f.close()

    except:
        print "no se logro leer nombre de ultimo archivo en name_db.txt"

    global APIRest
    APIRest = reviewDB.window_db(filedb, var, dt)
    socketio.emit('my_json', {'data': APIRest, 'No': len(APIRest), 'var': var}, namespace='/biocl')



@socketio.on('Setpoints', namespace='/biocl')
def setpoints(dato):
    global set_data
    #se reciben los nuevos setpoints
    set_data = [ dato['alimentar'], dato['mezclar'], dato['ph'], dato['descarga'], dato['temperatura'], dato['alimentar_rst'], dato['mezclar_rst'], dato['ph_rst'], dato['descarga_rst'], dato['temperatura_rst'], dato['alimentar_dir'], dato['ph_dir'], dato['temperatura_dir'] ]

    #Con cada cambio en los setpoints, se vuelven a emitir a todos los clientes.
    socketio.emit('Setpoints', {'set': set_data}, namespace='/biocl', broadcast=True)

    #guardo set_data en un archivo para depurar
    try:
        settings = str(set_data)
        f = open("setpoints.txt","a+")
        f.write(settings + '\n')
        f.close()

    except:
        print "no se pudo guardar en set_data en setpoints.txt"


#Sockets de calibración de instrumentación
#CALIBRACION DE PH
@socketio.on('ph_calibrar', namespace='/biocl')
def calibrar_ph(dato):
    global ph_set
    #se reciben los parametros para calibración
    setting = [ dato['ph'], dato['iph'], dato['medx'] ]

    #ORDEN DE: ph_set:
    #ph_set = [ph1_set, iph1_set, ph2_set, iph2_set]
    try:
        if setting[2] == 'med1':
            ph_set[0] = float(dato['ph'])   #y1
            ph_set[1] = float(dato['iph'])  #x1

        elif setting[2] == 'med2':
            ph_set[2] = float(dato['ph'])   #y2
            ph_set[3] = float(dato['iph'])  #x2

    except:
        ph_set = [0,0,0,0]

    if (ph_set[3] - ph_set[1])!=0 and ph_set[0]!=0 and ph_set[1]!=0:
        m_ph = round(( ph_set[2] - ph_set[0] )/( ph_set[3] - ph_set[1] ), 2)
        n_ph = round(  ph_set[0] - ph_set[1]*(m_ph), 2)

    else:
        m_ph = 0
        n_ph = 0

    if ph_set[0]!=0 and ph_set[1]!=0 and ph_set[2]!=0 and ph_set[3]!=0 and m_ph!=0 and n_ph!=0:
        try:
            coef_ph_set = [m_ph, n_ph]
            f = open("coef_ph_set.txt","w")
            f.write(str(coef_ph_set) + '\n')
            f.close()

        except:
            print "no se pudo guardar en coef_ph_set en coef_ph_set.txt"

    #Con cada cambio en los parametros, se vuelven a emitir a todos los clientes.
    socketio.emit('ph_calibrar', {'set': ph_set}, namespace='/biocl', broadcast=True)

    #guardo set_data en un archivo para depurar
    try:
        ph_set_txt = str(ph_set)
        f = open("ph_set.txt","a+")
        f.write(ph_set_txt + '\n')
        f.close()

    except:
        print "no se pudo guardar parameters en ph_set.txt"

#CALIBRACION OXIGENO DISUELTO
@socketio.on('od_calibrar', namespace='/biocl')
def calibrar_od(dato):
    global od_set
    #se reciben los parametros para calibración
    setting = [ dato['od'], dato['iod'], dato['medx'] ]

    #ORDEN DE: od_set:
    #ph_set = [od1_set, iod1_set, od2_set, iod2_set]
    try:
        if setting[2] == 'med1':
            od_set[0] = float(dato['od'])
            od_set[1] = float(dato['iod'])

        elif setting[2] == 'med2':
            od_set[2] = float(dato['od'])
            od_set[3] = float(dato['iod'])
    except:
        od_set = [0,0,0,0]


    if od_set[3] - od_set[1]!=0 and od_set[0]!=0 and od_set[1]!=0:
        m_od = round(( od_set[2] - od_set[0] )/( od_set[3] - od_set[1] ), 2)
        n_od = round(  od_set[0] - od_set[1]*(m_od), 2)

    else:
        m_od = 0
        n_od = 0

    if od_set[0]!=0 and od_set[1]!=0 and od_set[2]!=0 and od_set[3]!=0 and m_od!=0 and n_od!=0:
        try:
            coef_od_set = [m_od, n_od]
            f = open("coef_od_set.txt","w")
            f.write(str(coef_od_set) + '\n')
            f.close()

        except:
            print "no se pudo guardar en coef_ph_set en coef_od_set.txt"


    #Con cada cambio en los parametros, se vuelven a emitir a todos los clientes.
    socketio.emit('od_calibrar', {'set': od_set}, namespace='/biocl', broadcast=True)

    #guardo set_data en un archivo para depurar
    try:
        od_set_txt = str(od_set)
        f = open("od_set.txt","a+")
        f.write(od_set_txt + '\n')
        f.close()

    except:
        print "no se pudo guardar parameters en od_set.txt"


#CALIBRACIÓN TEMPERATURA
@socketio.on('temp_calibrar', namespace='/biocl')
def calibrar_temp(dato):
    global temp_set
    #se reciben los parametros para calibración
    setting = [ dato['temp'], dato['itemp'], dato['medx'] ]

    #ORDEN DE: od_set:
    #ph_set = [od1_set, iod1_set, od2_set, iod2_set]
    try:
        if setting[2] == 'med1':
            temp_set[0] = float(dato['temp'])
            temp_set[1] = float(dato['itemp'])

        elif setting[2] == 'med2':
            temp_set[2] = float(dato['temp'])
            temp_set[3] = float(dato['itemp'])

    except:
        temp_set = [0,0,0,0]

    if temp_set[3] - temp_set[1]!=0 and temp_set[0]!=0 and temp_set[1]!=0:
        m_temp = round(( temp_set[2] - temp_set[0] )/( temp_set[3] - temp_set[1] ), 2)
        n_temp = round(  temp_set[0] - temp_set[1]*(m_temp), 2)

    else:
        m_temp = 0
        n_temp = 0

    if temp_set[0]!=0 and temp_set[1]!=0 and temp_set[2]!=0 and temp_set[3]!=0 and m_temp!=0 and n_temp!=0:
        try:
            coef_temp_set = [m_temp, n_temp]
            f = open("coef_temp_set.txt","w")
            f.write(str(coef_temp_set) + '\n')
            f.close()

        except:
            print "no se pudo guardar en coef_ph_set en coef_od_set.txt"


    #Con cada cambio en los parametros, se vuelven a emitir a todos los clientes.
    socketio.emit('temp_calibrar', {'set': temp_set}, namespace='/biocl', broadcast=True)

    #guardo set_data en un archivo para depurar
    try:
        temp_set_txt = str(temp_set)
        f = open("temp_set.txt","a+")
        f.write(temp_set_txt + '\n')
        f.close()

    except:
        print "no se pudo guardar parameters en temp_set.txt"





#CONFIGURACION DE THREADS
def background_thread1():
    measures = [0,0,0,0,0,0]
    save_set_data = [0,0,0,0,0,1,1,1,1,1,0,0,0]

    while True:
        global set_data
        #se emiten las mediciones y setpoints para medir y graficar
        socketio.emit('Medidas', {'data': measures, 'set': set_data}, namespace='/biocl')

        #ZMQ DAQmx download data from micro controller
        temp_ = communication.zmq_client().split()

        measures[0] = temp_[1]  #ph
        measures[1] = temp_[2]  #oD
        measures[2] = temp_[3]  #Itemp1
        measures[3] = temp_[4]  #Iph
        measures[4] = temp_[5]  #Iod
        measures[5] = temp_[6]  #Itemp1-Itemp2

        for i in range(0,len(set_data)):
            if save_set_data[i] != set_data[i]:
                communication.send_setpoint(set_data)
                save_set_data = set_data

                print "\n Se ejecuto Thread 1 emitiendo %s\n" % set_data

        socketio.sleep(0.1)


if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
