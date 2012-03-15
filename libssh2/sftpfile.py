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


class SftpFileException(Exception):
    """
    Exception raised when L{Sftp} actions fails.
    """
    pass


class SftpFile(object):
    """
    Sftp object
    """
    def __init__(self, _handle):
        """
        Create a new Sftp object.
        """
        self._handle = _handle

    def read(self, maxlen=4096):
        """
        """
        logging.debug("Sftpfile." + sys._getframe(0).f_code.co_name)
        if maxlen == -1:
            buf = ""
            while True:
                try:
                    new_buf = self._handle.read()
                except Exception:
                    new_buf = ""

                if new_buf and len(new_buf) > 0:
                    buf += new_buf
                else:
                    break
            return buf
        else:
            return self._handle.read(maxlen)

    def write(self, message):
        """
        """
        logging.debug("Sftpfile." + sys._getframe(0).f_code.co_name)
        return self._handle.write(message)

    def tell(self):
        """
        """
        logging.debug("Sftpfile." + sys._getframe(0).f_code.co_name)
        return self._handle.tell()

    def seek(self, offset):
        """
        """
        logging.debug("Sftpfile." + sys._getframe(0).f_code.co_name)
        self._handle.seek(offset)
