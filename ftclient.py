import sys
from socket import *
from time import sleep
from os import path


# Function to # initiate contact with server, code from lecture
# Precondition: Known server name and port
# Post-condition: client socket
def initiate_contact(server, port):
    client_socket = socket(AF_INET, SOCK_STREAM)
    client_socket.connect((server, port))
    # print "now connected to server..."
    return client_socket


# Function to startup the data connection
# Preconditions: port number provided
# Post conditions: data socket
def start_data_connection(port):
    server_socket = socket(AF_INET, SOCK_STREAM)  # create TCP socket
    server_socket.bind(('', int(port)))
    server_socket.listen(1)

    print "**********The client is now ready to receive data***********"
    return server_socket


# Function to send command request to server
# Precondition: connection setup
# Post condition: message sent
def make_request():
        if command == "-l":
            message = command + " " + str(data_port)
        elif command == "-g":
            message = command + " " + file_name + " " + str(data_port)
        else:
            message = command
        # print "message to be sent to server is", message
        connection.send(message)


# Function to receive response data from client
# Precondition: connection setup
# Post condition: message received
def receive_data():
    if command == "-l":
        # set up data connection
        data_socket = start_data_connection(data_port)
        # server waits on for incoming requests, new socket created on return
        data_connection, address = data_socket.accept()
        print("\nReceiving directory structure from {}: {}".format(server_name, data_port))
        file_list = data_connection.recv(1024)
        print file_list
        data_connection.shutdown(1)
        data_connection.close()

    # If the command is g, get file from server and save
    elif command == "-g":
        validation = connection.recv(1024)
        # print "\"" + validation + "\""
        if "FILE NOT FOUND!" in validation:
            print("\n{}: {} says {}".format(server_name, server_port, validation))
        else:
            # set up data connection
            data_socket = start_data_connection(data_port)
            # server waits on for incoming requests, new socket created on return
            data_connection, address = data_socket.accept()
            print("\nReceiving \"{}\" from {}: {}".format(file_name, server_name, data_port))

            # handle duplicate file name by add postfix to the name,
            # https://www.guru99.com/python-check-if-file-exists.html
            new_file_name = file_name
            while path.exists(new_file_name):
                new_file_name = new_file_name.replace(".txt", "_1.txt")
            fp = open(new_file_name, "w")
            content = data_connection.recv(1024)
            # keep receiving until the reach the end of file marked
            while "@@" not in content:
                content += data_connection.recv(1024)

            # remove @@ from file and save
            content = content.replace("@@", "")
            # print content
            fp.write(content)
            print("File transfer complete!")
            data_connection.shutdown(1)
            data_connection.close()

    # receive error message for wrong command
    else:
        message_in = connection.recv(1024)
        print("\n{}: {} says {}".format(server_name, server_port,  message_in))


# main function
if __name__ == "__main__":
    # check usage and arguments are valid
    if len(sys.argv) != 5 and len(sys.argv) != 6:
        print "Wrong arguments, please check requirement!"
        exit(1)

    if ((sys.argv[3] == '-l' and len(sys.argv) != 5) or
            (sys.argv[3] == '-g' and len(sys.argv) != 6)):
        print "Wrong arguments, please check requirement!"
        exit(1)

    server_name = sys.argv[1]
    server_port = int(sys.argv[2])
    command = sys.argv[3]
    data_port = 0
    if command == "-l":
        data_port = int(sys.argv[4])
        # validate data port
        if data_port > 65536 or data_port < 1024:
            print "Wrong port number, please check!"
            exit(1)
    elif command == "-g":
        file_name = sys.argv[4]
        data_port = int(sys.argv[5])
        # validate data port
        if data_port > 65536 or data_port < 1024:
            print "Wrong port number, please check!"
            exit(1)

    # initialize connection
    connection = initiate_contact(server_name, server_port)

    make_request()
    receive_data()

    connection.shutdown(1)
    connection.close()








