import socket
import unittest
import os, pwd
import libssh2


class DeallocSftpSubTest(unittest.TestCase):

    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))

        self.session = libssh2.Session()
        self.session.startup(self.sock)
        self.session.userauth_agent(self.username)
        self.assertEqual(self.session.userauth_authenticated(), 1)

        self.sftp = self.session.sftp_init()

    def tearDown(self):
        self.sftp.shutdown()
        self.session.close()
        self.sock.close()

    def test_create_sftp_file(self):
        def func():
            sftp_file = self.sftp.open_file("/dev/zero", "r")
            del sftp_file
        func()

    def test_create_close_sftp_file(self):
        def func():
            sftp_file = self.sftp.open_file("/dev/zero", "r")
            self.sftp.close_file(sftp_file)
            del sftp_file
        func()

    def test_create_sftp_dir(self):
        def func():
            sftp_dir = self.sftp.open_dir("/tmp")
            del sftp_dir
        func()

    def test_create_close_sftp_dir(self):
        def func():
            sftp_dir = self.sftp.open_dir("/tmp")
            self.sftp.close_dir(sftp_dir)
            del sftp_dir
        func()


if __name__ == '__main__':
    unittest.main()
