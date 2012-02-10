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
import libssh2
import math

class SCPTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.socket)
        self.session.userauth_agent(self.username)
        self.assertEqual(self.session.userauth_authenticated(), 1)


    def test_send(self):
        # Initialize file that will be used
        IN_FILE_PATH = "/tmp/test_scp_send_file_in"
        OUT_FILE_PATH = "/tmp/test_scp_send_file_out"
        FILE_CONTENT = "CONTENT"
        if os.path.exists(IN_FILE_PATH):
            os.remove(IN_FILE_PATH)
        if os.path.exists(OUT_FILE_PATH):
            os.remove(OUT_FILE_PATH)
        f = open(IN_FILE_PATH, "w")
        f.write(FILE_CONTENT)
        f.close()
        in_file_state = os.stat(IN_FILE_PATH)
        self.session.scp_send_file(IN_FILE_PATH, OUT_FILE_PATH)

        if os.path.exists(OUT_FILE_PATH):
            out_file_state = os.stat(OUT_FILE_PATH)
            self.assertEqual(int(in_file_state.st_mode / 4096), int(out_file_state.st_mode / 4096))
            self.assertEqual(int(in_file_state.st_atime), int(out_file_state.st_atime))
            self.assertEqual(int(in_file_state.st_mtime), int(out_file_state.st_mtime))
            self.assertEqual(in_file_state.st_size, out_file_state.st_size)
            f = open(OUT_FILE_PATH, "r")
            self.assertEqual(f.read(), "CONTENT")
            f.close()
            os.remove(OUT_FILE_PATH)
        else:
            self.assertTrue(False, "File was not sent")

        if os.path.exists(IN_FILE_PATH):
            os.remove(IN_FILE_PATH)

    def a_test_recv(self):
        # Initialize file that will be used
        IN_FILE_PATH = "/tmp/test_scp_send_file_in"
        OUT_FILE_PATH = "/tmp/test_scp_send_file_out"
        FILE_CONTENT = "CONTENT"
        if os.path.exists(IN_FILE_PATH):
            os.remove(IN_FILE_PATH)
        if os.path.exists(OUT_FILE_PATH):
            os.remove(OUT_FILE_PATH)
        f = open(IN_FILE_PATH, "w")
        f.write(FILE_CONTENT)
        f.close()
        in_file_state = os.stat(IN_FILE_PATH)
        self.session.scp_recv_file(IN_FILE_PATH, OUT_FILE_PATH)

        if os.path.exists(OUT_FILE_PATH):
            out_file_state = os.stat(OUT_FILE_PATH)
            self.assertEqual(in_file_state, out_file_state)
            f = open(OUT_FILE_PATH, "r")
            self.assertEqual(f.read(), "CONTENT")
            os.remove(OUT_FILE_PATH)
        else:
            self.assertTrue(False, "File was not recv")

        if os.path.exists(IN_FILE_PATH):
            os.remove(IN_FILE_PATH)

    def tearDown(self):
        self.session.close()
        self.socket.close()

if __name__ == '__main__':
    unittest.main()
