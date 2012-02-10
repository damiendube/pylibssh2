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

"""
Abstraction for libssh2 L{Session} object
"""

from channel import Channel
from sftp import Sftp
import _libssh2
import os
import math

class SessionException(Exception):
    """
    Exception raised when L{Session} actions fails.
    """
    pass

class Session(object):
    """
    Session object
    """
    def __init__(self):
        """
        Create a new session object.
        """
        self._session = _libssh2.Session()

    def callback_set(self, callback_type, callback):
        """
        Sets callback on the session.

        @param callback_type: value of libssh2.LIBSSH2_CALLBACK_* constant
        @type callback_set: int
        @param callback: a callback Python object
        @type callback: function
        """
        self._session.callback_set(callback_type, callback)

    def close(self, reason=None):
        """
        Closes the session.

        @param reason: human readable reason for disconnection
        @type reason: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        if reason:
            return self._session.close(reason)
        else:
            return self._session.close()

    def direct_tcpip(self, host, port, shost, sport):
        """
        Tunnels a TCP connection through the session.

        @param host: remote host
        @type host: str
        @param port: remote port
        @type port: int
        @param shost: local host
        @type shost: str
        @param sport: local port
        @type sport: int

        @return: new opened L{Channel}
        @rtype: L{Channel}
        """
        return self._session.direct_tcpip(host, port, shost, sport)

    def forward_listen(self, host, port, bound_port, queue_maxsize):
        """
        Forwards a TCP connection through the session.

        @param host: remote host
        @type host: str
        @param port: remote port
        @type port: int
        @param bound_port: populated with the actual port on the remote host
        @type bound_port: int
        @param queue_maxsize: maxium number of pending connections
        @type int

        @return: new L{Listener} instance on success or None on failure
        @rtype: L{Listener}
        """
        return self._session.forward_listen(
            host, port, bound_port, queue_maxsize
        )

    def hostkey_hash(self, hashtype):
        """
        Returns the computed digest of the remote host's key.

        @param hashtype: values possible are 1 (HASH_MD5) or 2 (HASH_SHA1)
        @type hashtype: int

        @return: string representation of the computed hash value
        @rtype: str
        """
        return self._session.hostkey_hash(hashtype)

    def last_error(self):
        """
        Returns the last error in tuple format (code, message).

        @return: error tuple (int, str)
        @rtype: tuple
        """
        return self._session.last_error()

    def open_session(self):
        """
        Allocates a new L{Channel} for the session.

        @return: new channel opened
        @rtype: L{Channel}
        """
        return Channel(self._session.open_session())


    def set_trace(self, bitmask):
        """
        Sets trace level on the session.

        @param bitmask: bitmask on libssh2.LIBSSH2_TRACE_* constant
        """
        self._session.set_trace(bitmask)


    def scp_recv(self, remote_path):
        """
        Gets a remote file via SCP Protocol.

        @param remote_path: absolute path of remote file to transfer
        @type remote_path: str

        @return: new channel opened, mode, size
        @rtype: L{Channel}, mode, size
        """
        _channel, mode, size = self._session.scp_recv(remote_path)
        return (Channel(_channel), mode, size)

    def scp_send(self, path, mode, size, mtime=0, atime=0):
        """
        Sends a file to remote host via SCP protocol.

        @param path: absolute path of file to transfer
        @type path: str
        @param mode: file access mode to create file
        @type mode: int
        @param size: size of file being transmitted
        @type size: int

        @return: new channel opened
        @rtype: L{Channel}
        """
        return Channel(self._session.scp_send(path, int(mode), size, int(mtime), int(atime)))

    def scp_send_file(self, in_file_path, out_file_path):
        write_len = 4096
        f = open(in_file_path, "rb")
        file_stat = os.stat(in_file_path)
        channel = self.scp_send(out_file_path, file_stat.st_mode & 0777, file_stat.st_size, int(file_stat.st_mtime), int(file_stat.st_atime))
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
            channel.close()
        except Exception, detail:
            print "Failed to close %s" % (detail)

    def scp_recv_file(self, in_file_path, out_file_path):
        read_len = 4096
        f = open(in_file_path, "wb")
        channel, mode, file_size = self.scp_recv(in_file_path)
        if not channel:
            print "Failed to open channel"
            return

        got = 0
        while True:
            to_read = min(read_len, file_size - got)
            buf = channel.read(to_read)
            if len(buf) > 0:
                f.write(buf)
                got += len(buf)

            if got >= file_size:
                break
            print "got %i remain %i" % (got, file_size - got)

        #try:
        #    channel.close()
        #except Exception, detail:
        #    print "Failed to close %s" % (detail)

        f.close()
        os.chmod(out_file_path, mode)

    def session_method_pref(self, method_type, pref):
        """
        Sets preferred methods to be negociated. Theses preferences must be
        prior to calling L{startup}.

        @param method_type: the method type constants
        @type method_type: int
        @param pref: coma delimited list of preferred methods
        @type pref: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        self._session.session_methods(method_type, pref)

    def session_methods(self):
        """
        Returns dictionnary with the current actives algorithms.
        CS keys is Client to Server and SC keys is Server to Client.

        @return: dictionnary with actual method negociated
        @rtype: dict
        """
        return self._session.session_methods()

    def set_banner(self, banner=_libssh2.DEFAULT_BANNER):
        """
        Sets the banner that will be sent to remote host. This is optional, the
        banner L{_libssh2.DEFAULT_BANNER} will be sent by default.

        @param banner: an user defined banner
        @type banner: str
        
        @return: 0 on success or negative on failure
        @rtype: int
        """
        self._session.set_banner(banner)

    def sftp_init(self):
        """
        Open an Sftp Channel.

        @return: new Sftp channel opened
        @rtype: L{Sftp}
        """
        return Sftp(self._session.sftp_init())

    def startup(self, sock):
        """
        Starts up the session form a socket created by a socket.socket() call.

        @param sock: a connected socket object
        @type sock: socket._socketobject

        @return: 0 on success or negative on failure
        @rtype: int
        """
        self._session.startup(sock)

    def userauth_authenticated(self):
        """
        Returns authentification status for the given session.

        @return: non-zero if authenticated or 0 if not
        @rtype: int
        """
        return self._session.userauth_authenticated()

    def userauth_list(self, username):
        """
        Lists the authentification methods supported by a server.

        @param username: username which will be used while authenticating
        @type username: str

        @return: string containing a comma-separated list of authentication 
        methods
        @rtype: str
        """
        return self._session.userauth_list(username)

    def userauth_password(self, username, password):
        """
        Authenticates a session with the given username and password.

        @param username: user to authenticate
        @type username: str
        @param password: password to use for the authentication
        @type password: str

        @return: 0 on success or negative on failure
        @rtype: int
        """
        return self._session.userauth_password(username, password)

    def userauth_publickey_fromfile(
            self, username, publickey, privatekey, passphrase=None):
        """
        Authenticates a session as username using a key pair found in the 
        pulickey and privatekey files, and passphrase if provided.

        @param username: user to authenticate
        @type username: str
        @param publickey: path and name of public key file
        @type publickey: str
        @param privatekey: path and name of private key file
        @type privatekey: str
        @param passphrase: passphrase to use when decoding private file
        @type passphrase: str
        """
        if passphrase == None or len(passphrase) == 0:
            self._session.userauth_publickey_fromfile(username, publickey, privatekey)
        else:
            self._session.userauth_publickey_fromfile(username, publickey, privatekey, passphrase)

    def userauth_hostbased_fromfile(
            self, username, publickey, privatekey, hostname, passphrase=None):
        if passphrase == None or len(passphrase) == 0:
            self._session.userauth_hostbased_fromfile(username, publickey, privatekey, hostname)
        else:
            self._session.userauth_hostbased_fromfile(username, publickey, privatekey, hostname, passphrase)

    def userauth_keyboardinteractive(self, username, password):
        """
        Authenticates a session with the given username using a
        challenge-response authentication.

        @param username: user to authenticate
        @type username: str
        @param password: password using to fake keyboard method
        @type: str
        
        @return: 0 on success or negative on failure
        @rtype: int
        """
        return self._session.userauth_keyboardinteractive(username, password,
                                                   len(password))

    def userauth_agent(self, username):
        """
        Initialize an ssh-agent handle
        """
        return self._session.userauth_agent(username)
