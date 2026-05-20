from pythonosc import dispatcher
from pythonosc import osc_server
from pythonosc import udp_client


class PureDataComm:
    def __init__(self, addr_in, addr_out):
        self.ip_in, self.port_in, self.path_in = addr_in[0], addr_in[1], addr_in[2]
        self.port_out, self.path_out = addr_out[0], addr_out[1]
        self.client = udp_client.SimpleUDPClient(self.ip_in, self.port_out)

    def talk2pd(self, path, message):
        self.client.send_message(path, message)

    def function1(self, value, motor_path="/motor"):
        if not 0 <= value <= 255:
            raise ValueError("motor value must be between 0 and 255")
        self.talk2pd(motor_path, int(value))

    def myfunction1(self, address, *message):
        print(address, message)
        b = message[0]
        self.talk2pd(address, b)

    def myfunction2(self, address, *message):
        print(address, message)
        b = message[0]
        self.talk2pd(address, (b, 1, b, 2, b, 3, b, 4))

    def main(self, path, *osc_arguments):
        print(path, osc_arguments)

    def listen2pd(self):
        disp = dispatcher.Dispatcher()
        disp.map(self.path_in, self.main)
        disp.map("/myfunction1", self.myfunction1)
        disp.map("/myfunction2", self.myfunction2)

        server = osc_server.ThreadingOSCUDPServer((self.ip_in, self.port_in), disp)
        print("listening on {}".format(server.server_address))
        server.serve_forever()
    