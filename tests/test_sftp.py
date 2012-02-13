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

class SFTPTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.socket)
        self.session.userauth_agent(self.username)
        self.assertEqual(self.session.userauth_authenticated(), 1)

    def test_connect(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        sftp.shutdown()

    def test_unlink(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        FILE = "/tmp/../tmp/test_sftp_test_unlink"
        open(FILE, "w").close()
        sftp.unlink(FILE)
        self.assertFalse(os.path.exists(FILE))
        sftp.shutdown()

    def test_rename(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE1 = "/tmp/../tmp/test_sftp_test_rename"
        FILE2 = "/tmp/../tmp/test_sftp_test_rename_newname"
        open(FILE1, "w").close()
        sftp.rename(FILE1, FILE2)
        self.assertFalse(os.path.exists(FILE1))
        self.assertTrue(os.path.exists(FILE2))
        os.remove(FILE2)
        #
        sftp.shutdown()

    def test_mkdir(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        DIR = "/tmp/test_sftp_test_mkdir"
        if os.path.exists(DIR) == False:
            sftp.mkdir(DIR, 0644)
        self.assertTrue(os.path.isdir(DIR))
        os.rmdir(DIR)
        #
        sftp.shutdown()

    def test_rmdir(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        DIR = "/tmp/test_sftp_test_rmdir"
        if os.path.exists(DIR) == False:
            os.mkdir(DIR)
        sftp.rmdir(DIR)
        self.assertFalse(os.path.isdir(DIR))
        #
        sftp.shutdown()

    def test_realpath(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        FILE = "/tmp/../tmp/test_sftp_test_realpath"
        open(FILE, "w").close()
        real = sftp.realpath(FILE)
        self.assertEqual(os.path.abspath(FILE), real)
        os.remove(FILE)
        #
        sftp.shutdown()

    def test_readlink(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        FILE = "/tmp/../tmp/test_sftp_test_readlink"
        SYM = "/tmp/../tmp/test_sftp_test_readlink_sym"
        open(FILE, "w").close()
        os.symlink(FILE, SYM)
        self.assertEqual(os.readlink(SYM), sftp.readlink(SYM))
        os.remove(SYM)
        os.remove(FILE)
        #
        sftp.shutdown()

    def test_symlink(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/test_sftp_test_symlink"
        SYM = "/tmp/test_sftp_test_symlink_sym"
        open(FILE, "w").close()
        sftp.symlink(FILE, SYM)
        self.assertTrue(os.path.islink(SYM))
        os.remove(SYM)
        os.remove(FILE)
        #
        sftp.shutdown()

    def test_get_stat(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/../tmp/test_sftp_test_get_stat"
        open(FILE, "w").close()
        s1 = sftp.get_stat(FILE)
        s2 = os.stat(FILE)
        self.assertEqual(s1['st_mode'] & 0777, s2.st_mode & 0777)
        self.assertEqual(s1['st_size'], s2.st_size)
        #self.assertEqual(s1['st_atime'], int(s2.st_atime))
        self.assertEqual(s1['st_mtime'], int(s2.st_mtime))
        self.assertEqual(s1['st_uid'], s2.st_uid)
        self.assertEqual(s1['st_gid'], s2.st_gid)
        os.remove(FILE)
        #
        sftp.shutdown()

    def test_set_stat(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/../tmp/test_sftp_test_set_stat"
        open(FILE, "w").close()
        s1 = {"st_mode" : 0400 }
        sftp.set_stat(FILE, s1)
        s2 = os.stat(FILE)
        self.assertEqual(s1['st_mode'] & 0777, s2.st_mode & 0777)
        os.remove(FILE)
        #
        sftp.shutdown()

    def test_file_read(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/../tmp/test_sftp_test_file_read"
        CONTENT = "0123456789"
        f = open(FILE, "w")
        f.write(CONTENT)
        f.close()
        #
        sftp_file = sftp.open_file(FILE)
        # Seek and tell test
        sftp_file.seek(0)
        self.assertEqual(sftp_file.tell(), 0)
        sftp_file.seek(2)
        self.assertEqual(sftp_file.tell(), 2)
        sftp_file.seek(12)
        self.assertEqual(sftp_file.tell(), len(CONTENT) + 2)
        #
        #
        # Read and tell
        at = 0
        sftp_file.seek(at)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(1)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(2)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(3)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read()
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)

        #
        #
        # Read and tell
        at = 0
        sftp_file.seek(at)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(1)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(2)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(3)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read()
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #
        #
        # Read and tell
        at = 0
        sftp_file.seek(at)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(1)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #
        #
        # Read and tell
        at = 4
        sftp_file.seek(at)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(2)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)
        #
        #
        # Read and tell
        at = 5
        sftp_file.seek(at)
        #self.assertEqual(sftp_file.tell(), at)
        out_file = sftp_file.read(3)
        self.assertEqual(out_file, CONTENT[at:at + len(out_file)])
        at += len(out_file)

        #
        os.remove(FILE)
        sftp.shutdown()

    def test_file(self):
        pass

    def tearDown(self):
        self.session.close()
        self.socket.close()

if __name__ == '__main__':
    unittest.main()
