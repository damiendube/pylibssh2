import socket
import unittest
import os
import pwd
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
        self.assertNotEqual(self.session.userauth_authenticated(), 0)

    def tearDown(self):
        self.session.close()
        self.sock.close()

    def test_create_sftp(self):
        def func():
            self.session.sftp_init()
        func()

    def test_create_close_sftp(self):
        def func():
            sftp = self.session.sftp_init()
            self.session.sftp_shutdown(sftp)
        func()

    def test_delete_sftp_w_file(self):
        sftp = self.session.sftp_init()

        def func(sftp):
            sftp_file = sftp.open_file("/dev/zero", "r")
            self.assertTrue(sftp_file)
        func(sftp)

    def test_delete_sftp_w_dir(self):
        sftp = self.session.sftp_init()

        def func(sftp):
            sftp_dir = sftp.open_dir("/tmp")
            self.assertTrue(sftp_dir)
        func(sftp)

    def test_delete_sftp_w_file_dir(self):
        sftp = self.session.sftp_init()

        def func(sftp):
            sftp_file = sftp.open_file("/dev/zero", "r")
            self.assertTrue(sftp_file)
            sftp_dir = sftp.open_dir("/tmp")
            self.assertTrue(sftp_dir)
        func(sftp)

if __name__ == '__main__':
    unittest.main()
