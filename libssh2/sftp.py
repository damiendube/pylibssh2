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
        _sftp.close()

    def opendir(self, path):
        """
        """
        return SftpDir(_sftp.opendir(path))

    def open(self, path):
        """
        """
        return SftpFile(_sftp.open(path))

    def shutdown(self):
        """
        """
        _sftp.close()

    def unlink(self, path):
        """
        """
        _sftp.unlink(path)

    def rename(self, old_path, new_path):
        """
        """
        _sftp.rename(old_path, new_path)

    def mkdir(self, path, mode):
        """
        """
        _sftp.mkdir(path, mode)

    def rmdir(self, path):
        """
        """
        _sftp.rmdir()

    def realpath(self, path, pathlen, type):
        """
        """
        return _sftp.realpath(path, pathlen, type)

    def symlink(self, path, target):
        """
        """
        _sftp.symlink()

    def getstat(self, path, pathlen, type):
        """
        """
        return _sftp.opendir(path, pathlen, type)

    def setstat(self, path, attrs):
        """
        """
        _sftp.setstat(path, attrs)
