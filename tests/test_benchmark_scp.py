
import libssh2
import os
import pwd
import shutil
import socket
import time
import unittest


def sizeof_fmt(num):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            return "%3.1f %s" % (num, x)
        num /= 1024.0


class SCPBenchmarkTest(unittest.TestCase):

    SRC_PATH = "/tmp/repo-src"
    DST_PATH = "/tmp/repo-dst"

    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.sock)
        self.session.userauth_agent(self.username)
        self.assertNotEqual(self.session.userauth_authenticated(), 0)

    def create_files(self, count, size):
        try:
            shutil.rmtree(SCPBenchmarkTest.SRC_PATH)
        except:
            pass
        try:
            shutil.rmtree(SCPBenchmarkTest.DST_PATH)
        except:
            pass
        os.mkdir(SCPBenchmarkTest.SRC_PATH)
        os.mkdir(SCPBenchmarkTest.DST_PATH)
        #
        for i in range(0, count):
            filename = os.path.join(SCPBenchmarkTest.SRC_PATH, "%s" % (i))
            with open(filename, "w+") as f:
                f.truncate(size)
            self.assertEquals(os.stat(filename).st_size, size)

    def do_test(self, count, size):
        self.create_files(count, size)

        start_time = time.time()
        #
        for i in range(0, count):
            src = os.path.join(SCPBenchmarkTest.SRC_PATH, "%s" % (i))
            dst = os.path.join(SCPBenchmarkTest.DST_PATH, "%s" % (i))
            self.session.scp_recv_file(src, dst)
            self.assertTrue(os.path.exists(dst))
        end_time = time.time()

        print "%s Files of %s took %ssec speed %s/sec" % (count, sizeof_fmt(size), end_time - start_time, sizeof_fmt(count * size / (end_time - start_time)))

    @staticmethod
    def dl(arg_tuple):
        session = arg_tuple[0]
        src = arg_tuple[1]
        dst = arg_tuple[2]
        print "session %s src %s dst %s" % (session, src, dst)
        session.scp_recv_file(src, dst)

    #def test_100_1MB(self):
    #    self.do_test(10, 1024 * 1024)

    #def test_1024_512K(self):
    #    self.do_test(100, 1024 * 512)

    #def test_1024_10K(self):
    #    self.do_test(100, 1024 * 10)

    def test_100_1K(self):
        self.do_test(100, 1024)

    #def test_1024_512B(self):
    #    self.do_test(100, 512)

    def tearDown(self):
        self.session.close()
        self.sock.close()

if __name__ == '__main__':
    unittest.main()

