import socket
import libssh2
import os, pwd
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("localhost", 22))
sock.setblocking(1)

session = libssh2.Session()
session.set_banner()
session.startup(sock)
username = pwd.getpwuid(os.getuid())[0]


agent = session.agent()
agent.connect()
agent.userauth(username)

