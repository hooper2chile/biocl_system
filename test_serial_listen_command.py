import zmq, time

port_sub = "5556"
context_sub = zmq.Context()
socket_sub = context_sub.socket(zmq.SUB)
socket_sub.connect ("tcp://localhost:%s" % port_sub)
topicfilter = "w"
socket_sub.setsockopt(zmq.SUBSCRIBE, topicfilter)

time.sleep(0.1)

while True:
    string = socket_sub.recv().split()
    print string[1]
    time.sleep(0.1)
