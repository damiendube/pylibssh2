#!/usr/bin/env python
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
import socket, sys, os

import libssh2

usage = """Do a SCP recv <file> with username@hostname:/remote_path/
Usage: %s <hostname> <username> <password> <local_in_file> <remote_out_file>""" % __file__[__file__.rfind('/') + 1:]

class MySCPClient:
    def __init__(self, hostname, username, password, port=22):
        self.hostname = hostname
        self.username = username
        self.password = password
        self.port = port
        self._prepare_sock()

    def _prepare_sock(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.hostname, self.port))
            self.sock.setblocking(1)
        except Exception, e:
            print "SockError: Can't connect socket to %s:%d" % (self.hostname, self.port)
            print e

        try:
            self.session = libssh2.Session()
            self.session.set_banner()
            self.session.startup(self.sock)
            self.session.userauth_password(self.username, self.password)
        except Exception, e:
            print "SSHError: Can't startup session"
            print e

    def send(self, local_in_path, remote_out_path, mode=0644):
        write_len = 4096
        f = file(local_in_path, "rb")
        channel = self.session.scp_send(remote_out_path, mode, stat.st_size)
        if not channel:
            print "Failed to open channel"
            return
        buf = f.read(write_len)
        while True:
            if len(buf) > 0:
                written = channel.write(buf)
                if written == 0:
                    continue
                elif written == write_len:
                    buf = f.read(write_len)
                else:
                    buf = buf[written:-1]
            else:
                break
        channel.flush()
        channel.send_eof()
        channel.wait_eof()
        try:
            channel.wait_closed()
            self.session.channel_close(channel)
        except Exception, detail:
            print "Failed to close %s" % (detail)

    def __del__(self):
        self.session.close()
        self.sock.close()

if __name__ == '__main__' :
    if len(sys.argv) == 1:
        print usage
        sys.exit(1)
    myscp = MySCPClient(
        hostname=sys.argv[1],
        username=sys.argv[2],
        password=sys.argv[3]
    )
    import time
    startTime = time.time()
    myscp.send(sys.argv[4], sys.argv[5])
    endTime = time.time()
    print "Speed: %sMB/s" % (os.path.getsize(sys.argv[4]) / (endTime - startTime) / 1024 / 1024)