import csv, sqlite3, sys

#python aer2.py testa.db 10
def csv_file(FILE_DB, dt):

    db = sqlite3.connect( FILE_DB )
    c  = db.cursor()

    FILE_CSV = '/home/pi/biocl_system/csv/' + FILE_DB[31:-3] + 'T=' + str(dt) + '.csv'

    #################  PH  ########################
    c.execute('SELECT * FROM ph')
    j = 1
    temporal = {}
    #dt = int(sys.argv[2])

    for i in c:
        temporal[j] = [ i[1][:-7], i[2] ]
        j+=1

    ############################
    i=1
    temp = {}
    while i<=len(temporal)/dt:
        temp[i]=temporal[i*dt]
        i+=1
    ############################

    with open(FILE_CSV, 'a+') as fp:
        a = csv.writer(fp)
        k=1
        data = []
        a.writerow( ['date hour', 'pH'] )
        for k in range(1,len(temp)):
            data += [temp[k]]
        a.writerows(data)




    #################  TEMP  ########################
    c.execute('SELECT * FROM temp')
    j = 1
    temporal = {}
    #dt = int(sys.argv[2])

    for i in c:
        temporal[j] = [ i[1][:-7], i[2] ]
        j+=1

    ############################
    i=1
    temp = {}
    while i<=len(temporal)/dt:
        temp[i]=temporal[i*dt]
        i+=1
    ############################

    with open(FILE_CSV, 'a+') as fp:
        a = csv.writer(fp)
        k=1
        data = []
        a.writerow( [''] )
        a.writerow( ['date hour', 'Temp_pH'] )
        for k in range(1,len(temp)):
            data += [temp[k]]
        a.writerows(data)




    #################  OD  ########################
    c.execute('SELECT * FROM od')
    j = 1
    temporal = {}
    #dt = int(sys.argv[2])

    for i in c:
        temporal[j] = [ i[1][:-7], i[2] ]
        j+=1

    ############################
    i=1
    temp = {}
    while i<=len(temporal)/dt:
        temp[i]=temporal[i*dt]
        i+=1
    ############################

    with open(FILE_CSV, 'a+') as fp:
        a = csv.writer(fp)
        k=1
        data = []
        a.writerow( [''] )
        a.writerow( ['date hour', 'OD'] )
        for k in range(1,len(temp)):
            data += [temp[k]]
        a.writerows(data)



    c.close()
    db.close()
