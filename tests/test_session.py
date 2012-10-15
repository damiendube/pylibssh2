#
# Copyright (c) 2011 WALLIX, SAS. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: pylibssh2
# Author(s) : Sofian Brabez <sbz@wallix.com>
# Id: $Id: test_session.py 145 2011-01-11 11:12:38Z sbrabez $
# URL: $URL: svn+ssh://ohervieu@zoo.ifr.lan/srv/svn/pylibssh2/trunk/tests/test_session.py $
# Module description:
# Compile with:
#
"""
Unit tests for Session
"""

import socket
import unittest
import os
import pwd
import shutil
try:
    import libssh2
except:
    # Let test_import fail
    pass


class SessionTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = socket.gethostname()
        self.publickey = os.path.expanduser("~%s/.ssh/id_dsa.pub" % (self.username))
        self.privatekey = os.path.expanduser("~%s/.ssh/id_dsa" % (self.username))
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))

    def test_import(self):
        ok = False
        try:
            import libssh2  # pyflakes:ignore
            ok = True
        except ImportError:
            ok = False

        self.assertTrue(ok)

    def test_session_create(self):
        session = libssh2.Session()
        self.assertTrue(isinstance(session, libssh2.session.Session))

    def test_session_startup(self):
        session = libssh2.Session()
        session.set_banner()
        self.assertEqual(session.userauth_authenticated(), 0)
        session.startup(self.sock)

    def test_session_userauth_publickey(self):
        session = libssh2.Session()
        session.startup(self.sock)
        self.assertEqual(session.userauth_authenticated(), 0)
        session.userauth_publickey_fromfile(self.username, self.publickey, self.privatekey)
        self.assertEqual(session.userauth_authenticated(), 1)

    def test_session_userauth_hostbased(self):
        session = libssh2.Session()
        session.startup(self.sock)
        self.assertEqual(session.userauth_authenticated(), 0)
        session.userauth_hostbased_fromfile(self.username, self.publickey, self.privatekey, self.hostname)
        self.assertEqual(session.userauth_authenticated(), 1)

    def test_session_userauth_agent(self):
        session = libssh2.Session()
        session.startup(self.sock)
        self.assertEqual(session.userauth_authenticated(), 0)
        username = pwd.getpwuid(os.getuid())[0]
        session.userauth_agent(username)
        self.assertEqual(session.userauth_authenticated(), 1)
        session.close()

    def test_rmtree(self):
        DIR = "/tmp/repo/"
        SUB_DIR = "/tmp/repo/sub_dir"
        FILE1 = os.path.join(DIR, "file1")
        FILE2 = os.path.join(SUB_DIR, "file2")
        try:
            shutil.rmtree(DIR)
        except Exception:
            pass
        os.mkdir(DIR)
        os.mkdir(SUB_DIR)
        open(FILE1, "w").close()
        open(FILE2, "w").close()
        self.assertTrue(os.path.exists(FILE1))
        self.assertTrue(os.path.exists(FILE2))
        #
        session = libssh2.Session()
        session.startup(self.sock)
        username = pwd.getpwuid(os.getuid())[0]
        session.userauth_agent(username)
        self.assertEqual(session.userauth_authenticated(), 1)
        #
        session.rmrf(DIR + "/*")
        self.assertTrue(os.path.exists(DIR))
        self.assertFalse(os.path.exists(SUB_DIR))
        self.assertFalse(os.path.exists(FILE1))
        self.assertFalse(os.path.exists(FILE2))
        #
        session.close()

    def test_mv(self):
        DIR1 = "/tmp/repo"
        DIR2 = "/tmp/repo2"
        FILE1 = os.path.join(DIR1, "file1")
        FILE2 = os.path.join(DIR1, "file2")
        FILE3 = os.path.join(DIR1, "file3")
        FILE4 = os.path.join(DIR1, "file4")
        FILE5 = os.path.join(DIR1, "file5")
        OUT_FILE1 = os.path.join(DIR2, "file1")
        OUT_FILE2 = os.path.join(DIR2, "file2")
        OUT_FILE3 = os.path.join(DIR2, "file3")
        OUT_FILE4 = os.path.join(DIR2, "file4")
        OUT_FILE5 = os.path.join(DIR2, "file5")
        try:
            shutil.rmtree(DIR1)
            shutil.rmtree(DIR2)
        except Exception:
            pass
        os.mkdir(DIR1)
        os.mkdir(DIR2)
        open(FILE1, "w").close()
        open(FILE2, "w").close()
        open(FILE3, "w").close()
        open(FILE4, "w").close()
        open(FILE5, "w").close()
        self.assertTrue(os.path.exists(FILE1))
        self.assertTrue(os.path.exists(FILE2))
        self.assertTrue(os.path.exists(FILE3))
        self.assertTrue(os.path.exists(FILE4))
        self.assertTrue(os.path.exists(FILE5))
        self.assertFalse(os.path.exists(OUT_FILE1))
        self.assertFalse(os.path.exists(OUT_FILE2))
        self.assertFalse(os.path.exists(OUT_FILE3))
        self.assertFalse(os.path.exists(OUT_FILE4))
        self.assertFalse(os.path.exists(OUT_FILE5))
        #
        session = libssh2.Session()
        session.startup(self.sock)
        username = pwd.getpwuid(os.getuid())[0]
        session.userauth_agent(username)
        self.assertEqual(session.userauth_authenticated(), 1)
        #
        session.mv(FILE1, DIR2)
        session.mv([FILE2, FILE3], DIR2)
        session.mv([FILE4], DIR2)
        session.mv((FILE5), DIR2)
        self.assertTrue(os.path.exists(OUT_FILE1))
        self.assertTrue(os.path.exists(OUT_FILE2))
        self.assertTrue(os.path.exists(OUT_FILE3))
        self.assertTrue(os.path.exists(OUT_FILE4))
        self.assertTrue(os.path.exists(OUT_FILE5))
        #
        session.close()
    
    def tearDown(self):
        self.sock.close()

if __name__ == '__main__':
    unittest.main()
