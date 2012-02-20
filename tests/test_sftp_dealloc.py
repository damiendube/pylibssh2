import socket
import unittest
import os, pwd
import libssh2


class DeallocSftpTest(unittest.TestCase):
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

    def test_create_sftp(self):
        def func():
            sftp = self.session.sftp_init()
            del sftp
        func()

    def test_create_close_sftp(self):
        def func():
            sftp = self.session.sftp_init()
            sftp.shutdown()
            del sftp
        func()

    def test(self):
        sftp = self.session.sftp_init()
        def func():
            sftp_file = sftp.open_file("/dev/zero", "r")
            del sftp_file
        func()

if __name__ == '__main__':
    unittest.main()
