from pythonosc.udp_client import SimpleUDPClient

# Send msg on port 80
client = SimpleUDPClient("127.0.0.1", 80)

def send_to_puredata(msg, value):
    client.send_message(msg, value)
    # prova
    #client.send_message("/SQUATSTATE", 1)