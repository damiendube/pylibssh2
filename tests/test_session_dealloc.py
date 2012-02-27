import socket
import unittest
import os, pwd
import libssh2


class DeallocSessionTest(unittest.TestCase):

    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))

    def tearDown(self):
        self.sock.close()

    def test_create_session(self):

        def func():
            self.session = libssh2.Session()
            self.session.startup(self.sock)
            self.session.userauth_agent(self.username)
            self.assertEqual(self.session.userauth_authenticated(), 1)
        func()

    def test_create_close_session(self):
        def func():
            self.session = libssh2.Session()
            self.session.startup(self.sock)
            self.session.userauth_agent(self.username)
            self.assertEqual(self.session.userauth_authenticated(), 1)
            self.session.close()
        func()

    def test_create_close_w_sftp_session(self):
        def func():
            self.session = libssh2.Session()
            self.session.startup(self.sock)
            self.session.userauth_agent(self.username)
            self.assertEqual(self.session.userauth_authenticated(), 1)
            sftp = self.session.sftp_init()
            self.assertTrue(sftp)
            self.session.close()
        func()

    def test_create_close_w_sftp_file_session(self):
        def func():
            self.session = libssh2.Session()
            self.session.startup(self.sock)
            self.session.userauth_agent(self.username)
            self.assertEqual(self.session.userauth_authenticated(), 1)
            sftp = self.session.sftp_init()
            self.assertTrue(sftp)
            sftp_file = sftp.open_file("/dev/zero", "r")
            self.assertTrue(sftp_file)
            self.session.close()
        func()

    def test_create_close_w_sftp_file_dir_session(self):
        def func():
            self.session = libssh2.Session()
            self.session.startup(self.sock)
            self.session.userauth_agent(self.username)
            self.assertEqual(self.session.userauth_authenticated(), 1)
            sftp = self.session.sftp_init()
            self.assertTrue(sftp)
            sftp_file = sftp.open_file("/dev/zero", "r")
            sftp_dir = sftp.open_dir("/tmp")
            self.assertTrue(sftp_file)
            self.assertTrue(sftp_dir)
            self.session.close()
        func()

if __name__ == '__main__':
    unittest.main()
