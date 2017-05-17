'''
    Google Drive Icloud for Bioreactor.
    Macos and rasbian version.
    Application for synchronization.
'''

import os, sys, time, datetime

TIME_SYNC = 3600 #sync for 3600 [s] = 1 [hr]
ID = '0Bxw8dfMyxUX2MF80cFlwSFF3S1E'

if(sys.platform=='darwin'):
    gdrive = './gdrive-osx-x64'
    DIR1 = '/Users/hooper/Dropbox/BIOCL/biocl_system/config/'
    DIR2 = '/Users/hooper/Dropbox/BIOCL/biocl_system/database2/'

else:
    gdrive = './gdrive-linux-rpi'
    DIR1 = '/home/pi/biocl_system/config/'
    DIR2 = '/home/pi/biocl_system/database2/'

while True:
    hora = time.strftime("Hora=%H:%M:%S__Fecha=%d-%m-%y")
    try:
        os.system(DIR1 + gdrive + ' sync upload ' + DIR2+'.' + ' ' + ID)
        print 'sincronizado: ' + hora
        time.sleep(TIME_SYNC)

    except:
        print 'Fallo al subir al cloud:' + hora
