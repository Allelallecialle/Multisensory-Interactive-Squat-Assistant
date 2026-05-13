import argparse
from email import message
from pythonosc import dispatcher 
from pythonosc import osc_server 
from pythonosc import udp_client


input_port = 5005
output_port = 5006

class PureDataComm:
    def __init__(self, send_func):
        
        self.send_func = send_func
        self.input_port = input_port
        self.output_port = output_port

    def send2pd(self, message):
        
        self.send_func(message)
        
        
    def talk2pd(self, ip, port, path, mymove):
        client = udp_client.SimpleUDPClient(ip, port)
        #print (type(mymove))
        client.send_message(path, mymove)

    def myfunction1(self, address, *message):
        print (address, message)
        a = message
        b = (a[0]**2)
        self.talk2pd(self.input_port, self.output_port, address, b)
        
    def myfunction2(self, address, *message):
        print (address, message)
        a = message
        b = a[0]+1
        self.talk2pd(self.input_port, self.output_port, address, (b,1,b, 2,b,3,b,4))
        
    def listen2pd(self, addrIN, addrOUT):
        ipIN   = addrIN[0]
        portIN = addrIN[1]
        pathIN = addrIN[2]
        disp = dispatcher.Dispatcher()
        disp.map(pathIN, self.main, ipIN)
        
        ###############################################################
        ############ call functions that use data from pd ############
        ###############################################################
        # handlers are functions we want to run with pd information
        disp.map(address = "/myfunction1", handler = self.myfunction1) 
        disp.map(address = "/myfunction2", handler = self.myfunction2) 
        ###############################################################

        # server to listen
        server = osc_server.ThreadingOSCUDPServer((ipIN,portIN), disp)
        print("listening on {}".format(server.server_address))
        server.serve_forever()
        
    def main(self, path: str, *osc_arguments):
        msg = osc_arguments[-1]
