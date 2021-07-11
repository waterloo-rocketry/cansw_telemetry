import serial
import datetime
import sys
import os

ROOT_PATH = os.path.dirname(os.path.abspath(__file__))


def init_dev(port="COM1", baud_rate="9600"):
    dev = serial.Serial(port=port, baudrate=baud_rate)
    dev.flushInput()
    return dev


def main():
    file_name = sys.argv[3]
    FILE_PATH = os.path.join(ROOT_PATH, file_name)

    dev = init_dev(sys.argv[1], sys.argv[2])

    with open(FILE_PATH, "w") as data_file:
        while True:
            try:
                data_line = dev.readline()
                data_file.write(str(datetime.datetime.now()) + " " + data_line.decode("utf-8"))
                print(data_line)
            except KeyboardInterrupt:
                break


if __name__ == '__main__':
    main()
