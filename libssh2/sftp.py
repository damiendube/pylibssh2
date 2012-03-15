#
# pylibssh2 - python bindings for libssh2 library
#
# Copyright (C) 2010 Wallix Inc.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation; either version 2.1 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

from sftpdir import SftpDir
from sftpfile import SftpFile
import errno
import logging
import sys

"""
Abstraction for libssh2 L{Sftp} object
"""


class SftpException(Exception):
    """
    Exception raised when L{Sftp} actions fails.
    """
    pass


class Sftp(object):
    """
    Sftp object
    """
    def __init__(self, _sftp):
        """
        Create a new Sftp object.
        """
        self._sftp = _sftp

    def open_dir(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        return SftpDir(self._sftp.open_dir(path))

    def close_dir(self, sftp_dir):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        if not isinstance(sftp_dir, SftpDir):
            raise Exception("Bad instance type")
        self._sftp.close_dir(sftp_dir._handle)

    def open_file(self, path, flags, mode=None):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        if mode:
            return SftpFile(self._sftp.open_file(path, flags, mode))
        else:
            return SftpFile(self._sftp.open_file(path, flags))

    def close_file(self, sftp_file):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        if not isinstance(sftp_file, SftpFile):
            raise Exception("Bad instance type")
        self._sftp.close_file(sftp_file._handle)

    def exists(self, path):
        try:
            self.realpath(path)
            return True
        except IOError, detail:
            if detail.errno == errno.ENOENT:
                return False
            else:
                raise

    def unlink(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self._sftp.unlink(path)

    def remove(self, path):
        """
        Alias
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self.unlink(path)

    def rename(self, src, dst):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        try:
            self._sftp.rename(src, dst)
        except IOError, detail:
            if detail.errno == 5:
                self._sftp.unlink(dst)
                self._sftp.rename(src, dst)
            else:
                raise

    def move(self, src, dst):
        """
        Alias
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self.rename(src, dst)

    def copy_file(self, src, dst):
        src_file = self.open_file(src, "r")
        dst_file = self.open_file(dst, "w")
        dst_file.write(src_file.read(-1))
        self.close_file(src_file)
        self.close_file(dst_file)

    def mkdir(self, path, mode=0755):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self._sftp.mkdir(path, mode)

    def rmdir(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self._sftp.rmdir(path)

    def realpath(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        return self._sftp.realpath(path)

    def readlink(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        return self._sftp.readlink(path)

    def symlink(self, path, target):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self._sftp.symlink(path, target)

    def get_stat(self, path):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        return self._sftp.get_stat(path)

    def set_stat(self, path, attrs):
        """
        """

        logging.debug("Sftp." + sys._getframe(0).f_code.co_name)
        self._sftp.set_stat(path, attrs)
