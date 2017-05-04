import communication

while True:
    msg = communication.zmq_client()
    lista = msg.split()

    print(lista[1],lista[2],lista[3])
