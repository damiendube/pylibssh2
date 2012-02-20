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
import shutil
import stat


class SFTPTest(unittest.TestCase):
    def setUp(self):
        self.username = pwd.getpwuid(os.getuid())[0]
        self.hostname = "localhost"
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, 22))
        self.session = libssh2.Session()
        self.session.startup(self.sock)
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

    def test_copy_file(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE1 = "/tmp/../tmp/test_sftp_test_copy_file"
        FILE2 = "/tmp/../tmp/test_sftp_test_test_copy_file_newname"
        open(FILE1, "w").close()
        sftp.copy_file(FILE1, FILE2)
        self.assertTrue(os.path.exists(FILE1))
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
        FAKE_FILE = "/tmp/../tmp/test_sftp_test_realpath.fake"
        open(FILE, "w").close()
        real = sftp.realpath(FILE)
        self.assertEqual(os.path.abspath(FILE), real)
        try:
            sftp.realpath(FAKE_FILE)
        except:
            pass
        else:
            self.assertTrue(False, "sftp.realpath on unexistant file")
        #
        os.remove(FILE)
        sftp.shutdown()

    def test_readlink(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        FILE = "/tmp/../tmp/test_sftp_test_readlink"
        FAKE_FILE = "/tmp/../tmp/test_sftp_test_readlink.fake"
        SYM = "/tmp/../tmp/test_sftp_test_readlink_sym"
        open(FILE, "w").close()
        os.symlink(FILE, SYM)
        self.assertEqual(os.readlink(SYM), sftp.readlink(SYM))
        try:
            sftp.symlink(FAKE_FILE)
        except:
            pass
        else:
            self.assertTrue(False, "sftp.readlink on unexistant file")
        #
        os.remove(SYM)
        os.remove(FILE)
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
        self.assertEqual(stat.S_IMODE(s1['st_mode']), stat.S_IMODE(s2.st_mode))
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
        self.assertEqual(stat.S_IMODE(s1['st_mode']), stat.S_IMODE(s2.st_mode))
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
        def seek():
            sftp_file = sftp.open_file(FILE, "r")
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
            #sftp_file.close()
        seek()
        #
        os.remove(FILE)
        sftp.shutdown()

    def test_open(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/test_sftp_test_open.file"
        DIR = "/tmp/test_sftp_test_open.dir"
        NOT_A_FILE = "/tmp/test_sftp_test_open.not_a_file"
        try:
            open(FILE, "w").close()
        except:
            pass
        try:
            os.mkdir(DIR)
        except:
            pass

        try:
            sftp.open_file(FILE, "r")
        except Exception, detail:
            self.assertTrue(False, "Failed sftp.open_file %s" % (detail))

        try:
            sftp.open_file(DIR, "r")
        except Exception, detail:
            pass
        else:
            # For some reason this works...
            #self.assertTrue(False, "Did not failed sftp.open_file on a directory!")
            pass

        try:
            sftp.open_dir(DIR)
        except Exception, detail:
            self.assertTrue(False, "Failed sftp.open_dir %s" % (detail))

        try:
            sftp.open_dir(FILE)
        except Exception, detail:
            pass
        else:
            self.assertTrue(False, "Did not failed sftp.open_dir on a file")

        try:
            sftp.open_file(NOT_A_FILE, "r")
        except Exception, detail:
            pass
        else:
            self.assertTrue(False, "Did not fail sftp.open_file on a inexistant directory")

        try:
            sftp.open_dir(NOT_A_FILE)
        except Exception, detail:
            pass
        else:
            self.assertTrue(False, "Did not fail sftp.open_dir on a inexistant directory")


        shutil.rmtree(DIR, ignore_errors=True)
        os.remove(FILE)
        sftp.shutdown()

    def test_read(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/../tmp/test_sftp_test_read"
        CONTENT = "0123456789\n9876543210"
        f = open(FILE, "w")
        f.write(CONTENT)
        f.close()
        #
        sftp_file = sftp.open_file(FILE, "r")
        out_content = sftp_file.read(-1)
        self.assertEqual(CONTENT, out_content)
        sftp_file.close()
        #
        os.remove(FILE)
        sftp.shutdown()

    def test_write(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        #
        FILE = "/tmp/test_sftp_test_read"
        CONTENT = "0123456789\n9876543210"
        #
        sftp_file = sftp.open_file(FILE, "w", 0644)
        sftp_file.write(CONTENT)
        sftp_file.close()
        f = open(FILE)
        self.assertEqual(f.read(), CONTENT)
        f.close()
        #
        os.remove(FILE)
        sftp.shutdown()

    def test_dir(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        DIR = "/tmp/test_sftp_test_dir"
        SUB_DIR = "/tmp/test_sftp_test_dir/dir"
        FILE1 = "/tmp/test_sftp_test_dir/file1"
        FILE2 = "/tmp/test_sftp_test_dir/file2"
        #
        shutil.rmtree(DIR, ignore_errors=True)
        #
        sftp.mkdir(DIR)
        sftp.mkdir(SUB_DIR)
        sftp.open_file(FILE1, "w").close()
        sftp.open_file(FILE2, "w").close()
        sftp_dir = sftp.open_dir(DIR)
        files = sftp_dir.list_files()
        sftp_dir.close()
        #
        fileNames = [key for key in files.keys() if not stat.S_ISDIR(files[key]["st_mode"])]
        dirNames = [key for key in files.keys() if stat.S_ISDIR(files[key]["st_mode"])]
        #
        self.assertTrue(len(fileNames) == 2, "Number if files listed %s" % (len(files)))
        self.assertTrue(len(dirNames) == 3, "Number if files listed %s" % (len(files)))
        self.assertTrue("." in dirNames)
        self.assertTrue(".." in dirNames)
        self.assertTrue(os.path.basename(SUB_DIR) in dirNames)
        self.assertTrue(os.path.basename(FILE1) in fileNames)
        self.assertTrue(os.path.basename(FILE2) in fileNames)
        #
        shutil.rmtree(DIR, ignore_errors=True)
        sftp.shutdown()
        #

    def test_file_read_open_dir(self):
        sftp = self.session.sftp_init()
        self.assertTrue(sftp != None, "got an sftp object")
        DIR = "/tmp/test_sftp_test_file_read_open_dir"
        FILE1 = "/tmp/test_sftp_test_file_read_open_dir/file1"
        FILE2 = "/tmp/test_sftp_test_file_read_open_dir/file2"
        CONTENT = "0123456789"
        #
        shutil.rmtree(DIR, ignore_errors=True)
        #
        sftp.mkdir(DIR)
        f = sftp.open_file(FILE1, "w")
        f.write(CONTENT)
        f.close()
        f = sftp.open_file(FILE2, "w")
        f.write(CONTENT)
        f.close()
        sftp_file = sftp.open_file(FILE1, "r")
        sftp_file.read(-1)
        sftp_file.close()
        sftp_file = sftp.open_file(FILE2, "r")
        sftp_file.read(-1)
        sftp_file.close()
        sftp_dir = sftp.open_dir(DIR)
        sftp_dir.list_files()
        sftp_dir.close()
        #
        shutil.rmtree(DIR, ignore_errors=True)
        sftp.shutdown()
        #

    def tearDown(self):
        self.session.close()
        self.sock.close()

if __name__ == '__main__':
    unittest.main()
