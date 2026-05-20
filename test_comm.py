import puredata_comm

if __name__ == "__main__":
    outputAddress = [5006, "/filter"]
    inputAddress = ["127.0.0.1", 5005, "/motor"]
    comm = puredata_comm.PureDataComm(inputAddress, outputAddress)

    while True:
        raw = input("motor and value (e.g. 'motor1 128', q to quit): ").strip()
        if raw.lower() in ("q", "quit", "exit"):
            break
        try:
            motor_str, value_str = raw.split()
            comm.function1(int(value_str), f"/{motor_str}")
        except ValueError as e:
            print(f"invalid input: {e}")
