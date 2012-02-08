#
# pylibssh2 - python bindings for libssh2 library
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
Abstraction for libssh2 L{Agent} object
"""

class AgentException(Exception):
    """
    Exception raised when L{Agent} actions fails.
    """
    pass

class Agent(object):
    """
    Agent object
    """
    def __init__(self, _agent):
        """
        Create a new Agent object.
        """
        self._agent = _agent

    def connect(self):
        """
        """
        self._agent.connect()

    def disconnect(self):
        """
        """
        self._agent.disconnect()

    def userauth(self, username):
        """
        """
        self._agent.userauth(username)
