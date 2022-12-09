import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Listen on port 8080

s.bind(('' , 8080))

s.listen(5)

con, addr = s.accept()

while True:
    data = con.recv(1024)
    print(data)