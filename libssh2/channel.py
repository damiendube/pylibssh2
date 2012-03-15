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
"""
Abstraction for libssh2 L{Channel} object
"""


class ChannelException(Exception):
    """
    Exception raised when L{Channel} actions fails.
    """
    pass


class Channel(object):
    """
    Channel object
    """
    def __init__(self, _channel):
        """
        Creates a new channel object with the given _channel.

        @param _channel: low level channel object
        @type _channel: L{_libssh2.Channel}
        """
        self._channel = _channel

    def wait_closed(self):
        """
        Wait for remote channel to ack closing.

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.wait_closed")
        return self._channel.wait_closed()

    def eof(self):
        """
        Checks if the remote host has sent a EOF status.

        @return: 1 if the remote host has sent EOF otherwise 0
        @rtype: int
        """
        logging.debug("Channel.eof")
        return self._channel.eof()

    def execute(self, command):
        """
        Executes command on the channel.

        @param command: message data
        @type command: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.execute")
        return self._channel.execute(command)

    def exit_status(self):
        """
        Gets the exit code raised by the process running on the remote host.

        @return: the exit status reported by remote host or 0 on failure
        @rtype: int
        """
        logging.debug("Channel.exit_status")
        return self._channel.exit_status()

    def flush(self):
        """
        Flushs the read buffer on the channel.

        @return: 0 on sucess or negative on failure
        @rtype: int
        """
        logging.debug("Channel.flush")
        return self._channel.flush()

    def poll_read(self, extended=None):
        """
        Checks if data is available on the channel.

        @param extended: if message channel datas is extended
        @type extended: int

        @return: 1 when data is available or 0 otherwise
        @rtype: int
        """
        logging.debug("Channel.poll_read")
        if extended:
            return self._channel.poll_read(extended)
        else:
            return self._channel.poll_read()

    def pty(self, term="vt100"):
        """
        Requests a pty with term type on the channel.

        @param term: terminal emulation type (vt100, ansi, etc...)
        @type term: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.pty")
        return self._channel.pty(term)

    def pty_resize(self, width, height, pixelwidth=0, pixelheight=0):
        """
        Requests a pty resize of the channel with given width and height.

        @param width: terminal width in characters
        @type width: int
        @param height: terminal height in characters
        @type height: int
        @param pixelwidth: terminal width in pixels
        @type pixelwidth: int
        @param pixelheight: terminal height in pixels
        @type pixelheight: int

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.pty_resize")
        return self._channel.pty_resize(width, height, pixelwidth, pixelheight)

    def read(self, size=1024):
        """
        Reads size bytes on the channel.

        @param size: size of the buffer storage
        @type size: int

        @return: bytes readed or negative on failure
        @rtype: str
        """
        logging.debug("Channel.read")
        if size == -1:
            buf = ""
            while True:
                try:
                    new_buf = self._channel.read()
                except Exception:
                    new_buf = ""

                if new_buf and len(new_buf) > 0:
                    buf += new_buf
                else:
                    break
            return buf
        else:
            return self._channel.read(size)

    def read_ex(self, size=1024, stream_id=0):
        """
        Reads size bytes on the channel with given stream_id.

        @param size: size of the buffer storage
        @type size: int

        @param stream_id: stream_id of the stream upon which to read
        @type size: int

        @return: bytes readed or negative on failure
        @rtype: str
        """
        logging.debug("Channel.read_ex")
        return self._channel.read_ex(size, stream_id)

    def send_eof(self):
        """
        Sends EOF status on the channel to remote server.

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.send_eof")
        self._channel.send_eof()

    def wait_eof(self):
        """
        Checks if the remote host has sent a EOF status.
        """
        logging.debug("Channel.wait_eof")
        self._channel.wait_eof()

    def setblocking(self, mode=1):
        """
        Sets blocking mode on the channel. Default mode is blocking.

        @param mode: blocking (1) or non blocking (0) mode
        @rtype: int
        """
        logging.debug("Channel.setblocking")
        return self._channel.setblocking(mode)

    def setenv(self, name, value):
        """
        Sets envrionment variable on the channel.

        @param name: envrionment variable name
        @type name: str
        @param value: envrionment variable value
        @type value: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.setenv")
        return self._channel.setenv(name, value)

    def shell(self):
        """
        Requests a shell on the channel.

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.shell")
        return self._channel.shell()

    def window_read(self):
        """
        Checks the status of the read window on the channel.

        @param read_avail: window limit to read
        @type read_avail: int
        @param window_size_initial: window initial size defined
        @type window_size_initial: int

        @return: the number of bytes which the remote end may send
        without overflowing the window limit
        @rtype: int
        """
        logging.debug("Channel.window_read")
        return self._channel.window_read()

    def window_write(self):
        """
        window_write() -> (int, int)

        check the status of the write window.


        @return windows: the number of bytes which may be safely writen on the
        channel without blocking.
        @return window_size_initial: as defined by the channel_open request
        @rtype  (int, int)
        """
        logging.debug("Channel.window_write")
        return self._channel.window_write()

    def write(self, message):
        """
        Writes data on the channel.

        @param message: data to write
        @type message: str

        @return: number of bytes writter
        @rtype: int
        """
        logging.debug("Channel.write")
        return self._channel.write(message)

    def x11_req(self, single_connection, auth_proto, auth_cookie, display):
        """
        Requests a X11 Forwarding on the channel.

        @param single_connection: forward a single connection
        @type single_connection: int
        @param auth_proto: X11 authentication protocol to use
        @type auth_proto: str
        @param auth_cookie: the cookie
        @type auth_cookie: str
        @param display: screen number
        @type display: int

        @return: 0 on success or negative on failure
        @rtype: int
        """
        logging.debug("Channel.x11_req")
        return self._channel.x11_req(single_connection, auth_proto, auth_cookie, display)
