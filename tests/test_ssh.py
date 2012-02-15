import socket
import unittest
import os, pwd
import libssh2

class SSHTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.sock)
        self.session.userauth_agent(self.username)
        self.assertEqual(self.session.userauth_authenticated(), 1)

    def tearDown(self):
        self.session.close()
        self.sock.close()

if __name__ == '__main__':
    unittest.main()
