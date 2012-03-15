import socket
import unittest
import os
import pwd
import libssh2
import shutil


class SSHTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.sock)
        self.session.userauth_agent(self.username)
        self.assertNotEqual(self.session.userauth_authenticated(), 0)

    def testBreakEveryThing(self):
        DIR = "/tmp/repo"
        SUB_DIR = "/tmp/repo/sub"
        FILE1 = os.path.join(DIR, "file1")
        FILE2 = os.path.join(SUB_DIR, "file2")
        FILE1_OUT = os.path.join(DIR, "file1.out")
        #
        try:
            shutil.rmtree(DIR)
        except:
            pass
        try:
            shutil.rmtree(SUB_DIR)
        except:
            pass
        #
        os.mkdir(DIR)
        os.mkdir(SUB_DIR)
        open(FILE1, "w").close()
        open(FILE2, "w").close()
        #

        def func1():
            sftp = self.session.sftp_init()

            def func2():
                sftp.exists(DIR)
                sftp_filex = sftp.open_file(FILE1, "r")
                self.assertTrue(sftp_filex)
                sftp.get_stat(DIR)
                sftp.exists(SUB_DIR)
                sftp_file1 = sftp.open_file(FILE1, "r")
                sftp.close_file(sftp_file1)
                sftp.get_stat(FILE1)
                sftp_file2 = sftp.open_file(FILE2, "r")
                sftp_dir = sftp.open_dir(DIR)
                sftp.exists(FILE2)
                sftp.close_file(sftp_file2)
                sftp.get_stat(FILE2)
                sftp_file1 = sftp.open_file(FILE1, "r")
                sftp_file2 = sftp.open_file(FILE2, "r")
                sftp.close_file(sftp_file1)
                sftp.close_file(sftp_file2)
                sftp.exists(FILE1)
                sftp.close_dir(sftp_dir)
            func2()
            func2()
            self.session.rmrf(SUB_DIR)
            os.mkdir(SUB_DIR)
            open(FILE2, "w").close()
            sftp.rename(FILE1, FILE1_OUT)
            sftp.rename(FILE1_OUT, FILE1)
            sftp.rename(FILE1, FILE1_OUT)
            sftp.rename(FILE1_OUT, FILE1)
            sftp.rename(FILE1, FILE1_OUT)
            sftp.rename(FILE1_OUT, FILE1)
            sftp.rename(FILE1, FILE1_OUT)
            sftp.rename(FILE1_OUT, FILE1)
            sftp.rename(FILE1, FILE1_OUT)
            sftp.rename(FILE1_OUT, FILE1)
            self.session.sftp_shutdown(sftp)
            try:
                func2()
            except:
                pass
        func1()
        func1()

    def tearDown(self):
        self.session.close()
        self.sock.close()

if __name__ == '__main__':
    unittest.main()
