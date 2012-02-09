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
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.publickey = os.path.expanduser("~/.ssh/id_rsa.pub")
        self.privatekey = os.path.expanduser("~/.ssh/id_rsa")
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.hostname, 22))

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

    def test_session_startup(self):
        session = libssh2.Session()
        session.set_banner()
        self.assertEqual(session.userauth_authenticated(), 0)
        session.startup(self.socket)

    def test_session_userauth_publickey(self):
        session = libssh2.Session()
        session.startup(self.socket)
        self.assertEqual(session.userauth_authenticated(), 0)
        session.userauth_publickey_fromfile(self.username, self.publickey, self.privatekey)
        self.assertEqual(session.userauth_authenticated(), 1)

    def test_session_userauth_hostbased(self):
        session = libssh2.Session()
        session.startup(self.socket)
        self.assertEqual(session.userauth_authenticated(), 0)
        session.userauth_hostbased_fromfile(self.username, self.publickey, self.privatekey, self.hostname)
        self.assertEqual(session.userauth_authenticated(), 1)

    def test_session_userauth_agent(self):
        session = libssh2.Session()
        session.startup(self.socket)
        self.assertEqual(session.userauth_authenticated(), 0)
        username = pwd.getpwuid(os.getuid())[0]
        session.userauth_agent(username)
        self.assertEqual(session.userauth_authenticated(), 1)
        session.close()

    def tearDown(self):
        self.socket.close()

if __name__ == '__main__':
    unittest.main()
