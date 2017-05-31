#!usr/bin/env python
# -*- coding: utf-8 -*-
'''
    Adquisición de datos por sockets (ZMQ) para la base de datos.
'''
import sys, time, datetime, sqlite3, sqlitebck, communication

TIME_MIN_BD = 1 # 1 [s]

def update_db(real_data, connector, c, first_time, BACKUP):
    #CREACION DE TABLAS PH, OD, TEMP. CADA ITEM ES UNA COLUMNA
    c.execute('CREATE TABLE IF NOT EXISTS   PH(ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')
    c.execute('CREATE TABLE IF NOT EXISTS   OD(ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')
    c.execute('CREATE TABLE IF NOT EXISTS TEMP(ID INTEGER PRIMARY KEY autoincrement, FECHA_HORA TIMESTAMP NOT NULL, MAGNITUD REAL)')

    #se guardan las tablas agregados en la db si no existian
    connector.commit()

    #INSERCION DE LOS DATOS MEDIDOS
    #ph=: real_data[1];  OD=: real_data[2], Temp=: real_data[3]
    try:
        c.execute("INSERT INTO   PH VALUES (NULL,?,?)", (datetime.datetime.now(), real_data[1]))
        c.execute("INSERT INTO   OD VALUES (NULL,?,?)", (datetime.datetime.now(), real_data[2]))
        c.execute("INSERT INTO TEMP VALUES (NULL,?,?)", (datetime.datetime.now(), real_data[3]))

    except:
        print "no se pudo insertar dato en db"
    #se guardan los datos agregados en la db
    connector.commit()

    #Backup DB in RAM to DISK SD
    if BACKUP:
        if(sys.platform=='darwin'):
            filedb='/Users/hooper/Dropbox/BIOCL/biocl_system/database/backup__' + first_time + '__.db'
            #filedb='/Users/hooper/Dropbox/BIOCL/biocl_system/database/backup.db'
        else:
            filedb='/home/pi/biocl_system/database/backup__' + first_time + '__.db'
            #filedb='/home/pi/biocl_system/database/backup.db'

        bck = sqlite3.connect(filedb)
        sqlitebck.copy(connector, bck)
        print "\n Backup REALIZADO \n"

        try:
            #Se guarda el nombre de la db para ser utilizado en app.py
            f = open("name_db.txt","w")
            f.write(filedb + '\n')
            f.close()

        except:
            print "no se pudo guardar el nombre de la DB para ser revisada en app.py"

        return True

def main():
    first_time = time.strftime("Hora__%H_%M_%S__Fecha__%d-%m-%y")
    TIME_BCK = 60
    connector = sqlite3.connect(':memory:', detect_types = sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES)
    c = connector.cursor()

    #Algoritmo de respaldo cada "TIME_BCK [s]"
    BACKUP = False
    start_time = time.time()
    end_time   = time.time()

    while True:
        #ZMQ connection for download data
        real_data = communication.zmq_client().split()

        update_db(real_data, connector, c, first_time, BACKUP)

        delta = end_time - start_time

        if delta <= TIME_BCK:
            BACKUP = False
            end_time = time.time()

        else:
            start_time = time.time()
            end_time   = time.time()
            BACKUP = True

        #Aqui se determina el tiempo con que guarda datos la BD.-
        time.sleep(TIME_MIN_BD)


if __name__ == "__main__":
    main()
