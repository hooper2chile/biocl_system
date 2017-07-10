#!usr/bin/env python
# -*- coding: utf-8 -*-
'''
    Adquisición de datos por sockets (ZMQ) para la base de datos.
'''
import os, sys, time, datetime, sqlite3, sqlitebck, logging, communication

logging.basicConfig(filename='/home/pi/biocl_system/log/database.log', level=logging.INFO, format='%(asctime)s:%(levelname)s:%(message)s')
TIME_MIN_BD = 1 # 1 [s]
DIR="/home/pi/biocl_system/"
flag_database = "False"
flag_database_local = False

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
        #print "no se pudo insertar dato en db"
        logging.info("no se pudo insertar dato en db")

    #se guardan los datos agregados en la db
    connector.commit()

    #Backup DB in RAM to DISK SD
    if BACKUP:

        filedb='/home/pi/biocl_system/database/backup__' + first_time + '__.db'

        bck = sqlite3.connect(filedb)
        sqlitebck.copy(connector, bck)

        try:
            os.system('sqlite3 -header -csv %s "select * from ph;"   > /home/pi/biocl_system/csv/%s' % (filedb,filedb[31:-3])+'full_ph.csv' )
            os.system('sqlite3 -header -csv %s "select * from od;"   > /home/pi/biocl_system/csv/%s' % (filedb,filedb[31:-3])+'full_od.csv' )
            os.system('sqlite3 -header -csv %s "select * from temp;" > /home/pi/biocl_system/csv/%s' % (filedb,filedb[31:-3])+'full_temp.csv' )

            logging.info("\n Backup FULL REALIZADO \n")

        except:
            logging.info("\n Backup FULL NO REALIZADO, NO REALIZADO \n")


        try:
            #Se guarda el nombre de la db para ser utilizado en app.py
            f = open(DIR + "name_db.txt","w")
            f.write(filedb + '\n')
            f.close()

        except:
            #print "no se pudo guardar el nombre de la DB para ser revisada en app.py"
            logging.info("no se pudo guardar el nombre de la DB para ser revisada en app.py")

        return True

def main():
    i = 0
    first_time = time.strftime("Hora__%H_%M_%S__Fecha__%d-%m-%y")
    TIME_BCK = 30#120
    connector = sqlite3.connect(':memory:', detect_types = sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES)
    c = connector.cursor()


    #Algoritmo de respaldo cada "TIME_BCK [s]"
    BACKUP = False
    start_time = time.time()
    end_time   = time.time()

    global flag_database_local, flag_database
    #reviso constantemente
    while True:
        #reviso la primera vez si cambio el flag, y luego sirve para revisar cuando salga por que fue apagado el flag desde app.py
        try:
            f = open(DIR + "flag_database.txt","r")
            flag_database = f.readlines()[-1]
            f.close()

            logging.info("FLAG_DATABASE WHILE SUPERIOR:")

            if flag_database == "True":
                flag_database_local = True
            else:
                flag_database_local = False

        except:
            logging.info("no se pudo leer el flag en el while principal")

        time.sleep(5)

        #depuracion log
        if flag_database_local is False:
            if i is 10:
                try:
                    f = open(DIR + "db_log.txt","a+")
                    f.write("Grabando OFF\n")
                    f.close()

                    i = 0
                    logging.info("GRABANDO OFF")

                except:
                    logging.info("no se pudo leer grabar el log de grabar off")
                #depuracion log

        i += 1
        while flag_database_local:

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

            try:
                f = open(DIR + "flag_database.txt","r")
                flag_database = f.readlines()[-1][:-1]
                f.close()

                logging.info("FLAG_DATABASE WHILE SECUNDARIO:")


                if flag_database == "True":
                    flag_database_local = True
                else:
                    flag_database_local = False

            except:
                logging.info("no se pudo leer el flag en el while secundario")


            #log de grabacion: cada 10 seg (dado el time sleep: TIME_MIN_BD del while) se actualiza el log
            if i is 10:
                try:
                    f = open(DIR + "db_log.txt","a+")
                    f.write("Grabando ON\n")
                    f.close()

                    i = 0
                    logging.info("GRABANDO ON")

                except:
                    logging.info("no se pudo leer grabar el log de grabar on")
                #depuracion log

            i += 1


if __name__ == "__main__":
    main()
