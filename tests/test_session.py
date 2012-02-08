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
import os, pwd
try:
    import libssh2
except:
    # Let test_import fail
    pass

class SessionTest(unittest.TestCase):
    def setUp(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect(("localhost", 22))

    def test_import(self):
        ok = False
        try:
            import libssh2 # pyflakes:ignore
            ok = True
        except ImportError:
            ok = False

        self.assertTrue(ok)

    def test_session_create(self):
        session = libssh2.Session()
        self.assertTrue(isinstance(session, libssh2.session.Session))

    def test_session_handshake(self):
        session = libssh2.Session()
        session.set_banner()
        self.assertEqual(session.userauth_authenticated(), 0)
        session.handshake(self.socket)

    def test_session_password_login(self):
        session = libssh2.Session()
        session.set_banner()
        session.handshake(self.socket)
        self.assertEqual(session.userauth_authenticated(), 0)

        session.userauth_password("ddube", "D1fference")
        self.assertEqual(session.userauth_authenticated(), 1)

    def test_session_host_login(self):
        session = libssh2.Session()
        session.set_banner()
        session.handshake(self.socket)
        self.assertEqual(session.userauth_authenticated(), 0)

        username = pwd.getpwuid(os.getuid())[0]

        session.userauth_publickey_fromfile(username, os.path.expanduser("~%s/.ssh/id_rsa.pub" % (username)), os.path.expanduser("~%s/.ssh/id_rsa" % (username)), "D1fference")

        self.assertEqual(session.userauth_authenticated(), 1)

    def tearDown(self):
        self.socket.close()

if __name__ == '__main__':
    unittest.main()
