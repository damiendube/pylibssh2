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
import logging
import sys
"""
Abstraction for libssh2 L{Sftp} object
"""


class SftpDirException(Exception):
    """
    Exception raised when L{Sftp} actions fails.
    """
    pass


class SftpDir(object):
    """
    Sftp object
    """
    def __init__(self, _handle):
        """
        Create a new Sftp object.
        """
        self._handle = _handle

    def read(self):
        """
        """
        logging.debug("Sftpdir." + sys._getframe(0).f_code.co_name)
        return self._handle.read()

    def list_files(self):
        """
        """
        logging.debug("Sftpdir." + sys._getframe(0).f_code.co_name)
        return self._handle.list_files()
