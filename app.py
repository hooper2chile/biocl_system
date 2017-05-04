#!/usr/bin/env python
# -*- coding: utf-8 -*-
from flask import Flask, render_template, session, request, Response
from flask_socketio import SocketIO, emit, disconnect

import sys, communication, user_list, reviewDB

# Set this variable to "threading", "eventlet" or "gevent" to test the
# different async modes, or leave it set to None for the application to choose
# the best option based on installed packages.
async_mode = None

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread1 = None


set_data = [0,0,0,0,0,1,1,1,1,1,0,0,0]


#CONFIGURACION DE PAGINAS WEB
@app.route('/', methods=['GET', 'POST'])
def index():
    error = None
    if request.method == 'POST':
        if user_list.auth_login(request.form['username'],request.form['password']):
            return render_template('index.html', title_html="Bioreactor Software")
        else:
            return render_template("login.html", error="Credencial Invalida")

    else:
        return render_template("login.html", error="Requiere Credencial")


@app.route('/graphics')
def graphics():
    return render_template('graphics.html', title_html="Variables de Proceso")


@app.route('/dbase', methods=['GET', 'POST'])
def viewDB():
    return render_template('dbase.html', title_html="Data Logger")


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


#CONFIGURACION DE FUNCIONES SocketIO
#Connect to the Socket.IO server. (Este socket es OBLIGACION)
@socketio.on('connect', namespace='/biocl')
def function_thread():
    print "\n Cliente Conectado al Thread del Bioreactor\n"

    #Se emite durante la primera conexión de un cliente el estado actual de los setpoints
    emit('Setpoints', {'set': set_data})

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


#CONFIGURACION DE LOS THREADS
def background_thread1():
    measures = [0,0,0]
    save_set_data = [0,0,0,0,0,1,1,1,1,1,0,0,0]

    while True:
        global set_data
        #se emiten las mediciones y setpoints para medir y graficar
        socketio.emit('Medidas', {'data': measures, 'set': set_data}, namespace='/biocl')

        #ZMQ DAQmx download data from micro controller
        temp = communication.zmq_client().split()

        measures[0] = temp[1]
        measures[1] = temp[2]
        measures[2] = temp[3]

        for i in range(0,len(set_data)):
            if save_set_data[i] != set_data[i]:
                communication.send_setpoint(set_data)
                save_set_data = set_data

                print "\n Se ejecuto Thread 1 emitiendo %s\n" % set_data

        socketio.sleep(0.25)


if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=False)
