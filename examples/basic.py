import socket
import libssh2


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("localhost", 22))

session = libssh2.Session()
session.set_banner()
session.startup(sock)

print session.userauth_authenticated()
agent = session.agent()

agent.connect()
print agent.identities()
agent.userauth("ddube")
agent.disconnect()
agent = None

print session.userauth_authenticated()
