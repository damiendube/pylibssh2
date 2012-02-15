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

    def open_dir(self, path):
        """
        """
        return SftpDir(self._sftp.open_dir(path))

    def open_file(self, path, flags, mode=None):
        """
        """
        if mode:
            return SftpFile(self._sftp.open_file(path, flags, mode))
        else:
            return SftpFile(self._sftp.open_file(path, flags))

    def exists(self, path):
        try:
            real_path = self.realpath(path)
            if real_path and len(real_path):
                return True
            else:
                return False
        except IOError:
            return False

    def shutdown(self):
        """
        """
        self._sftp.shutdown()

    def unlink(self, path):
        """
        """
        self._sftp.unlink(path)

    def remove(self, path):
        """
        Alias
        """
        self.unlink(path)

    def rename(self, src, dst):
        """
        """
        self._sftp.rename(src, dst)

    def move(self, src, dst):
        """
        Alias
        """
        self.rename(src, dst)

    def copy_file(self, src, dst):
        src_file = self.open_file(src, "r")
        dst_file = self.open_file(dst, "w")
        dst_file.write(src_file.read(-1))
        src_file.close()
        dst_file.close()

    def mkdir(self, path, mode=0755):
        """
        """
        self._sftp.mkdir(path, mode)

    def rmdir(self, path):
        """
        """
        self._sftp.rmdir(path)

    def realpath(self, path):
        """
        """
        return self._sftp.realpath(path)

    def readlink(self, path):
        """
        """
        return self._sftp.readlink(path)

    def symlink(self, path, target):
        """
        """
        self._sftp.symlink(path, target)

    def get_stat(self, path):
        """
        """
        return self._sftp.get_stat(path)

    def set_stat(self, path, attrs):
        """
        """
        self._sftp.set_stat(path, attrs)
