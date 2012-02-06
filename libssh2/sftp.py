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

from sftpfile import SftpFile
from sftpdir import SftpDir

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

    def close(self):
        """
        """
        self._sftp.close()

    def open_dir(self, path):
        """
        """
        return SftpDir(self._sftp.open_dir(path))

    def open_file(self, path):
        """
        """
        return SftpFile(self._sftp.open_file(path))

    def shutdown(self):
        """
        """
        self._sftp.close()

    def unlink(self, path):
        """
        """
        self._sftp.unlink(path)

    def rename(self, old_path, new_path):
        """
        """
        self._sftp.rename(old_path, new_path)

    def mkdir(self, path, mode):
        """
        """
        self._sftp.mkdir(path, mode)

    def rmdir(self, path):
        """
        """
        self._sftp.rmdir()

    def realpath(self, path, pathlen, type):
        """
        """
        return self._sftp.realpath(path, pathlen, type)

    def symlink(self, path, target):
        """
        """
        self._sftp.symlink()

    def getstat(self, path, pathlen, type):
        """
        """
        return self._sftp.opendir(path, pathlen, type)

    def setstat(self, path, attrs):
        """
        """
        self._sftp.setstat(path, attrs)
