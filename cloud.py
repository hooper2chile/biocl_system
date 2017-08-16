'''
    Google Drive cloud for Bioreactor.
    Macos and rasbian version.
    Application for synchronization.
'''

import os, sys, time, datetime, logging

logging.basicConfig(filename='/home/pi/biocl_system/log/cloud.log', level=logging.INFO, format='%(asctime)s:%(levelname)s:%(message)s')



TIME_SYNC = 5 #3600 #sync for 3600 [s] = 1 [hr]
ID = '0B3jT9_WfcyT9SXg3eExUdWoxRUU'

if(sys.platform=='darwin'):
    gdrive = './gdrive-osx-x64'
    DIR1 = '/Users/hooper/Dropbox/BIOCL/biocl_system/config/'
    DIR2 = '/Users/hooper/Dropbox/BIOCL/biocl_system/csv/'

else:
    time.sleep(15)
    gdrive = '/home/pi/biocl_system/config/./gdrive-linux-rpi'
    DIR1   = ' ' #'/home/pi/biocl_system/config/'
    DIR2   = '/home/pi/biocl_system/csv/'


while True:
    hora = time.strftime("Hora=%H:%M:%S__Fecha=%d-%m-%y")
    try:
        os.system(DIR1 + gdrive + ' sync upload ' + DIR2 + '.' + ' ' + ID)
        logging.info('sincronizado: ' + hora)
        f = open(DIR2+'gdrive_sync.txt','a+')
        f.write('sincronizado: ' + hora + ' \n')
        f.close()
        time.sleep(TIME_SYNC)

    except:
        logging.info('Fallo al subir a cloud:' + hora)
