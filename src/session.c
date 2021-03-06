/*-
 * pylibssh2 - python bindings for libssh2 library
 *
 * Copyright (C) 2005 Keyphrene.com.
 * Copyright (C) 2010 Wallix Inc.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <Python.h>
#define PYLIBSSH2_MODULE
#include "pylibssh2.h"


/* {{{ PYLIBSSH2_Session_startup
 */
static char PYLIBSSH2_Session_startup_doc[] = "\
    startup(socket)\n\
    \n\
    Perform the SSH startup.\n\
    \n\
    @param  socket: an open socket to the remote host\n\
    @type   socket: object\n";
static PyObject*
PYLIBSSH2_Session_startup(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int fd;

    PyObject *socket;
    if (!PyArg_ParseTuple(args, "O:startup", &socket)) {
        return NULL;
    }

    Py_XINCREF(socket);
    self->socket = socket;
    fd = PyObject_AsFileDescriptor(self->socket);

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_session_startup(self->session, fd);
    Py_END_ALLOW_THREADS

    if(rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_SOCKET_NONE:
                PyErr_Format(PYLIBSSH2_Error, "The socket is invalid: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_BANNER_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send banner to remote host: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_KEX_FAILURE:
                PyErr_Format(PYLIBSSH2_Error, "Encryption key exchange with the remote host failed: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SOCKET_DISCONNECT:
                PyErr_Format(PYLIBSSH2_Error, "The socket was disconnected: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_PROTO:
                PyErr_Format(PYLIBSSH2_Error, "An invalid SSH protocol response was received on the socket: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block: %s", errmsg);
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Failure establishing startup %i: %s", rc, errmsg);
                return NULL;
        }
    }

    self->opened = 1;

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_set_banner
 */
static char PYLIBSSH2_Session_set_banner_doc[] = "\
set_banner(banner)\n\
\n\
Sets the banner that will be sent to remote host.\n\
This is optional, the banner libssh2.DEFAULT_BANNER will be sent by default.\n\
\n\
@param  banner: an user defined banner\n\
@type   banner: str\n";
static PyObject *
PYLIBSSH2_Session_set_banner(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *banner = LIBSSH2_SSH_DEFAULT_BANNER"_Python";

    if (!PyArg_ParseTuple(args, "|s:set_banner", &banner)) {
        return NULL;
    }

    rc = libssh2_banner_set(self->session, banner);
    if(rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_SetString(PYLIBSSH2_Error, " An internal memory allocation call failed.");
                return NULL;
            case LIBSSH2_ERROR_EAGAIN:
                PyErr_SetString(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block.");
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i", rc);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ session_close
 */
int
Session_close(PYLIBSSH2_SESSION *self)
{
    PRINTFUNCNAME
    int rc;
    char *reason = "end";
    PyObject *item;

    if(self->opened) {
        while(PySet_Size(self->sftps)) {
            item = PySet_Pop(self->sftps);
            Sftp_shutdown((PYLIBSSH2_SFTP*)item);
            Py_XDECREF(item);
        }

        while(PySet_Size(self->channels)) {
            item = PySet_Pop(self->channels);
            Channel_close((PYLIBSSH2_CHANNEL*)item);
            Py_XDECREF(item);
        }
        while(PySet_Size(self->listeners)) {
            item = PySet_Pop(self->listeners);
            Py_XDECREF(item);
        }

        Py_BEGIN_ALLOW_THREADS
        rc = libssh2_session_disconnect(self->session, reason);
        Py_END_ALLOW_THREADS

        self->opened = 0;

        return rc;
    }
    return 0;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_close
 */
static char PYLIBSSH2_Session_close_doc[] = "\
close([reason])\n\
\n\
Closes the session.\n\
\n\
@param  reason: human readable reason for disconnection\n\
@type   reason: str\n";

static PyObject*
PYLIBSSH2_Session_close(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *reason = "end";

    if (!PyArg_ParseTuple(args, "|s:close", &reason)) {
        return NULL;
    }

    rc = Session_close(self);
    if (rc < 0) {
        /* CLEAN: PYLIBSSH2_SESSION_CLOSE_MSG */
        PyErr_SetString(PYLIBSSH2_Error, "SSH close error.");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_userauth_authenticated
 */
static char PYLIBSSH2_Session_userauth_authenticated_doc[] = "\
userauth_authenticated() -> int\n\
\n\
Returns authentification status for the given session.\n\
\n\
@return non-zero if authenticated or 0 if not\n\
@rtype  int";

static PyObject *
PYLIBSSH2_Session_userauth_authenticated(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    return Py_BuildValue("i", libssh2_userauth_authenticated(self->session));
}
/* }}} */

/* {{{ PYLIBSSH2_Session_userauth_list
 */
static char PYLIBSSH2_Session_userauth_list_doc[] = "\
userauth_list(username) -> str\n\
\n\
Lists the authentification methods supported by a server.\n\
\n\
@param  username: username which will be used while authenticating\n\
@type   username: str\n\
\n\
@return a string containing a comma-separated list of authentication methods\n\
@rtype  str";

static PyObject *
PYLIBSSH2_Session_userauth_list(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    char *username;
    char *auth_list;
    int rc;
    if (!PyArg_ParseTuple(args, "s:userauth_list", &username)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    auth_list = libssh2_userauth_list(self->session, username, strlen(username));
    Py_END_ALLOW_THREADS

    if (auth_list == NULL) {
        char *errmsg;
        rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_SetString(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block.");
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown error %i: %s", rc, errmsg);
                return NULL;

        }
    }

    return PyString_FromString(auth_list);
}
/* }}} */

/* {{{ PYLIBSSH2_Session_hostkey_hash
 */
static char PYLIBSSH2_Session_hostkey_hash_doc[] = "\n\
hostkey_hash([hashtype]) -> str\n\
\n\
Returns the computed digest of the remote host's key.\n\
\n\
@param  hashtype: values possible are 1 (HASH_MD5) or 2 (HASH_SHA1)\n\
@type   hashtype: str\n\
\n\
@return string representation of the computed hash value\n\
@rtype  str";

static PyObject *
PYLIBSSH2_Session_hostkey_hash(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int hashtype = LIBSSH2_HOSTKEY_HASH_MD5;
    const char *hash;
    char buff[20+1];
    size_t len;

    if (!PyArg_ParseTuple(args, "|i:hostkey_hash", &hashtype)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    hash = libssh2_hostkey_hash(self->session, hashtype);
    Py_END_ALLOW_THREADS

    if (hash == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    switch (hashtype) {
        case LIBSSH2_HOSTKEY_HASH_MD5:
            len = 16;
            break;
        case LIBSSH2_HOSTKEY_HASH_SHA1:
            len = 20;
            break;
        default:
            len = 0;
            break;
    }

    memcpy(buff, hash, len);
    buff[len] = '\0';

    return PyString_FromString(buff);
}
/* }}} */

/* {{{ PYLIBSSH2_Session_userauth_password
 */
static char PYLIBSSH2_Session_userauth_password_doc[] = "\n\
userauth_password(username, password)\n\
\n\
Authenticates a session with the given username and password.\n\
\n\
@param  username: user to authenticate\n\
@type   username: str\n\
@param  password: password to use for the authentication\n\
@type   password: str\n";

static PyObject*
PYLIBSSH2_Session_userauth_password(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *username;
    char *password;

    if (!PyArg_ParseTuple(args, "ss:userauth_password", &username, &password)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_userauth_password(self->session, username, password);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_PASSWORD_EXPIRED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_PASSWORD_EXPIRED: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
                PyErr_Format(PYLIBSSH2_Error, "failed, invalid username/password or public/private key: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_userauth_publickey_fromfile
 */
static char PYLIBSSH2_Session_userauth_publickey_fromfile_doc[] = "\n\
userauth_publickey_fromfile(username, publickey, privatekey, passphrase)\n\
\n\
Authenticates a session as username using a key pair found in the pulickey and\n\
privatekey files, and passphrase if provided.\n\
\n\
@param  username: user to authenticate\n\
@type   username: str\n\
@param  publickey: path and name of public key file\n\
@type   publickey: str\n\
@param  privatekey: path and name of private key file\n\
@type   privatekey: str\n\
@param  passphrase: passphrase to use when decoding private file\n\
@type   passphrase: str\n";

static PyObject*
PYLIBSSH2_Session_userauth_publickey_fromfile(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *username;
    char *publickey;
    char *privatekey;
    char *passphrase = NULL;

    if (!PyArg_ParseTuple(args, "sss|s:userauth_publickey_fromfile", &username,
                          &publickey, &privatekey, &passphrase)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_userauth_publickey_fromfile(self->session, username, publickey,
                                             privatekey, passphrase);
    Py_END_ALLOW_THREADS

    if (rc) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED:
                PyErr_Format(PYLIBSSH2_Error, "The username/public key combination was invalid: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
                PyErr_Format(PYLIBSSH2_Error, "Authentication using the supplied public key was not accepted: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_SetString(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block.");
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_userauth_publickey_fromfile
 */
static char PYLIBSSH2_Session_userauth_hostbased_fromfile_doc[] = "\n\
userauth_hostbased_fromfile(username, publickey, privatekey, passphrase, hostname)\n\
\n";

static PyObject*
PYLIBSSH2_Session_userauth_hostbased_fromfile(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *username;
    char *publickey;
    char *privatekey;
    char *hostname;
    char *passphrase = NULL;

    if (!PyArg_ParseTuple(args, "ssss|s:userauth_publickey_fromfile", &username, &publickey, &privatekey, &hostname, &passphrase)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_userauth_hostbased_fromfile(self->session, username, publickey,privatekey, passphrase, hostname);
    Py_END_ALLOW_THREADS

    if (rc) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED:
                PyErr_Format(PYLIBSSH2_Error, "The username/public key combination was invalid: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
                PyErr_Format(PYLIBSSH2_Error, "Authentication using the supplied public key was not accepted: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_SetString(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block.");
                return NULL;

            case LIBSSH2_ERROR_FILE:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_FILE: %s", errmsg);
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */


/* {{{ PYLIBSSH2_Session_userauth_publickey_fromfile
 */
static char PYLIBSSH2_Session_userauth_agent_doc[] = "\n\
userauth_agent(username)\n";

static PyObject*
PYLIBSSH2_Session_userauth_agent(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *username;
    struct libssh2_agent_publickey *store = NULL;
    LIBSSH2_AGENT * agent = NULL;

    if (!PyArg_ParseTuple(args, "s:userauth_agent", &username)) {
        return NULL;
    }

    agent = libssh2_agent_init(self->session);
    if(agent == NULL) {
        PyErr_SetString(PYLIBSSH2_Error, "Unable to initialize agent");
        return NULL;

    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_agent_connect(agent);
    Py_END_ALLOW_THREADS

    if (rc == 0) {
        rc = libssh2_agent_list_identities(agent);
        if(rc == 0) {
            while(rc == 0) {
                rc = libssh2_agent_get_identity(agent, &store, store);
                if(rc < 0) {
                    PyErr_SetString(PYLIBSSH2_Error, "Unable to get identity");
                    break;
                }
                if(libssh2_agent_userauth(agent, username, store) == 0) {
                    break;
                }
                if(rc == 1) {
                    PyErr_SetString(PYLIBSSH2_Error, "Unable userauth using agent identities");
                }
            }
        }
        else {
            PyErr_SetString(PYLIBSSH2_Error, "Unable to list identities");
        }

        Py_BEGIN_ALLOW_THREADS
        libssh2_agent_disconnect(agent);
        Py_END_ALLOW_THREADS
    }

    libssh2_agent_free(agent);

    if(PyErr_Occurred()) {
        return NULL;
    }
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
/* }}} */


/* {{{ PYLIBSSH2_Session_session_methods
 */
static char PYLIBSSH2_Session_session_methods_doc[] = "\n\
session_methods() -> dict\n\
\n\
Returns a dictionnary with the currently active algorithms. \n\
CS keys is Client to Server and SC keys is Server to Client.\n\
\n\
@return a dictionnary with actual method neogicated\n\
@rtype dict";

static PyObject *
PYLIBSSH2_Session_session_methods(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    /* cs = client-server, sc = server-client */
    const char *kex, *hostkey;
    const char *crypt_cs, *crypt_sc;
    const char *mac_cs, *mac_sc;
    const char *comp_cs, *comp_sc;
    const char *lang_cs, *lang_sc;

    PyObject *methods;

    kex = libssh2_session_methods(self->session, LIBSSH2_METHOD_KEX);
    if (kex == NULL) {
        PyErr_SetString(PYLIBSSH2_Error, "session has not yet been started no methods negociated");
        return NULL;
    }
    hostkey = libssh2_session_methods(self->session, LIBSSH2_METHOD_HOSTKEY);
    crypt_cs = libssh2_session_methods(self->session, LIBSSH2_METHOD_CRYPT_CS);
    crypt_sc = libssh2_session_methods(self->session, LIBSSH2_METHOD_CRYPT_SC);
    mac_cs = libssh2_session_methods(self->session, LIBSSH2_METHOD_MAC_CS);
    mac_sc = libssh2_session_methods(self->session, LIBSSH2_METHOD_MAC_SC);
    comp_cs = libssh2_session_methods(self->session, LIBSSH2_METHOD_COMP_CS);
    comp_sc = libssh2_session_methods(self->session, LIBSSH2_METHOD_COMP_SC);
    lang_cs = libssh2_session_methods(self->session, LIBSSH2_METHOD_LANG_CS);
    lang_sc = libssh2_session_methods(self->session, LIBSSH2_METHOD_LANG_SC);

    /* create a python dictionnary to store cryptographic algorithms */
    methods = PyDict_New();
    PyDict_SetItemString(methods, "KEX", PyString_FromString(kex));
    PyDict_SetItemString(methods, "HOSTKEY", PyString_FromString(hostkey));
    PyDict_SetItemString(methods, "CRYPT_CS", PyString_FromString(crypt_cs));
    PyDict_SetItemString(methods, "CRYPT_SC", PyString_FromString(crypt_sc));
    PyDict_SetItemString(methods, "MAC_CS", PyString_FromString(mac_cs));
    PyDict_SetItemString(methods, "MAC_SC", PyString_FromString(mac_sc));
    PyDict_SetItemString(methods, "COMP_CS", PyString_FromString(comp_cs));
    PyDict_SetItemString(methods, "COMP_SC", PyString_FromString(comp_sc));
    PyDict_SetItemString(methods, "LANG_CS", PyString_FromString(lang_cs));
    PyDict_SetItemString(methods, "LANG_SC", PyString_FromString(lang_sc));

    return methods;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_session_method_pref
 */
static char PYLIBSSH2_Session_session_method_pref_doc[] = "\n\
session_method_pref(method_type, pref)\n\
\n\
Sets preferred methods to be negociated. Theses preferences must be\n\
prior to calling startup().\n\
\n\
@param  method_type: the method type constants\n\
@type   method_type: L{libssh2.METHOD}\n\
@param  pref: coma delimited list of preferred methods\n\
@type   pref: str\n";

static PyObject*
PYLIBSSH2_Session_session_method_pref(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int method;
    char *pref;
    int rc;

    if (!PyArg_ParseTuple(args, "is:session_method_pref", &method, &pref)) {
        return NULL;
    }
    rc = libssh2_session_method_pref(self->session, method, pref);
    if(rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_INVAL:
                PyErr_SetString(PYLIBSSH2_Error, "The requested method type was invalid.");
                return NULL;
            case LIBSSH2_ERROR_ALLOC:
                PyErr_SetString(PYLIBSSH2_Error, "An internal memory allocation call failed.");
                return NULL;
            case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
                PyErr_SetString(PYLIBSSH2_Error, "The requested method is not supported.");
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i", rc);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_open_session
 */
static char PYLIBSSH2_Session_open_session_doc[] = "\n\
open_session() -> libssh2.Channel\n\
\n\
Allocates a new channel for the current session.\n\
\n\
@return new channel opened\n\
@rtype  libssh2.Channel";

static PyObject *
PYLIBSSH2_Session_open_session(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_CHANNEL *channel;

    Py_BEGIN_ALLOW_THREADS
    channel = libssh2_channel_open_session(self->session);
    Py_END_ALLOW_THREADS
    if(channel == NULL) {
        char *errmsg;
        int rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket.: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_CHANNEL_FAILURE:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_FAILURE: %s", errmsg);
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Failed to open a channel session %i: %s", rc, errmsg);
                return NULL;
        }


    }
    PyObject *chan = (PyObject *)PYLIBSSH2_Channel_New(self->session, channel);
    if(chan) {
        PySet_Add(self->channels, chan);
    }
    return chan;
}
/* }}} */


/* {{{ PYLIBSSH2_Session_scp_recv
 */
static char PYLIBSSH2_Session_scp_recv_doc[] = "\n\
scp_recv(remote_path) -> libssh2.Channel\n\
\n\
Requests a remote file via SCP protocol.\n\
\n\
@param  remote_path: absolute path of remote file to transfer\n\
@type   remote_path: str\n\
\n\
@return new channel opened\n\
@rtype  libssh2.Channel";

static PyObject *
PYLIBSSH2_Session_scp_recv(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    char *path;
    LIBSSH2_CHANNEL *channel;
    struct stat fileinfo;
    if (!PyArg_ParseTuple(args, "s:scp_recv", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    channel = libssh2_scp_recv(self->session, path, &fileinfo);
    Py_END_ALLOW_THREADS
    if (channel == NULL) {
        /* CLEAN: PYLIBSSH2_CHANNEL_SCP_RECV_ERROR_MSG */
        PyErr_SetString(PYLIBSSH2_Error, "SCP receive error.");
        return NULL;
    }

    PyObject *chan = (PyObject *)PYLIBSSH2_Channel_New(self->session, channel);
    if(chan) {
        PySet_Add(self->channels, chan);
    }
    return Py_BuildValue("OO", chan, stat_to_statdict(&fileinfo));
}
/* }}} */

/* {{{ PYLIBSSH2_Session_scp_send
 */
static char PYLIBSSH2_Session_scp_send_doc[] = "\n\
scp_send(path, mode, size) -> libssh2.Channel\n\
\n\
Sends a file to remote host via SCP protocol.\n\
\n\
@param path: absolute path of file to transfer\n\
@type  path: str\n\
@param mode: file access mode to create file\n\
@type  mode: int\n\
@param size: size of file being transmitted\n\
@type  size: int\n\
\n\
@return new channel opened\n\
@rtype  libssh2.Channel";

static PyObject *
PYLIBSSH2_Session_scp_send(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    char *path;
    int mode;
    unsigned long filesize;
    int mtime;
    int atime;
    LIBSSH2_CHANNEL *channel;

    if (!PyArg_ParseTuple(args, "sikii:scp_send", &path, &mode, &filesize, &mtime, &atime)) {
        return NULL;
    }

#if LIBSSH2_VERSION_NUM >= 0x010206
    channel = libssh2_scp_send64(self->session, path, mode, filesize, mtime, atime);
#else
    channel = libssh2_scp_send_ex(self->session, path, mode, filesize, mtime, atime);
#endif

    if (channel == NULL) {
        char *errmsg;
        int rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC :
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND :
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SCP_PROTOCOL :
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SCP_PROTOCOL: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN :
                PyErr_Format(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }
    PyObject * chan = (PyObject *)PYLIBSSH2_Channel_New(self->session, channel);
    if(chan) {
        PySet_Add(self->channels, chan);
    }
    return chan;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_channel_close
 */
static char PYLIBSSH2_Session_channel_close_doc[] = "";

static PyObject *
PYLIBSSH2_Session_channel_close(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    PYLIBSSH2_CHANNEL *channel;
    int index;

    if (!PyArg_ParseTuple(args, "O:channel_close", &channel)) {
        return NULL;
    }

    PySet_Discard(self->channels, (PyObject*)channel);
    Channel_close(channel);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_set_blocking
 */
static char PYLIBSSH2_Session_set_blocking_doc[] = "";

static PyObject *
PYLIBSSH2_Session_set_blocking(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    /* 1 blocking, 0 non blocking */
    int block = 1;

    if (!PyArg_ParseTuple(args, "|i:setblocking", &block)) {
        return NULL;
    }

    libssh2_session_set_blocking(self->session, block);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_sftp_init
 */
static char PYLIBSSH2_Session_sftp_init_doc[] = "\n\
sftp_init() -> libssh2.Sftp\n\
\n\
Opens an SFTP Channel.\n\
\n\
@return new opened SFTP channel\n\
@rtype  libssh2.Sftp";

static PyObject *
PYLIBSSH2_Session_sftp_init(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_SFTP *sftp;

    sftp = libssh2_sftp_init(self->session);

    if (sftp == NULL) {
        char *errmsg;
        int rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, " Unable to send data on socket: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_EAGAIN: %s", errmsg);
                return NULL;
            case LIBSSH2_ERROR_CHANNEL_FAILURE:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_FAILURE: %s", errmsg);
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    PyObject *sftpObj = (PyObject *)PYLIBSSH2_Sftp_New(self->session, sftp);
    if(sftpObj) {
        PySet_Add(self->sftps, sftpObj);
    }
    return sftpObj;
}
/* }}} */


/* {{{ PYLIBSSH2_Session_sftp_shutdown
 */
static char PYLIBSSH2_Session_sftp_shutdown_doc[] = "";

static PyObject *
PYLIBSSH2_Session_sftp_shutdown(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTP *sftp;
    int index;

    if (!PyArg_ParseTuple(args, "O:sftp_shutdown", &sftp)) {
        return NULL;
    }

    PySet_Discard(self->sftps, (PyObject *)sftp);
    Sftp_shutdown(sftp);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_direct_tcpip
 */
static char PYLIBSSH2_Session_direct_tcpip_doc[] = "\n\
direct_tcpip(host, port, shost, sport) -> libssh2.Channel\n\
\n\
Tunnels a TCP connection through an SSH Session.\n\
\n\
@param  host: remote host\n\
@type   host: str\n\
@param  port: remote port\n\
@type   port: int\n\
@param  shost: local host\n\
@type   shost: str\n\
@param  sport: local port\n\
@type   sport: int\n\
\n\
@return new opened channel\n\
@rtype  libssh2.Channel";

static PyObject *
PYLIBSSH2_Session_direct_tcpip(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    /* remote host */
    char *host;
    /* local host */
    char *shost = "127.0.0.1";
    /* local port */
    int sport = 22;
    /* remote port */
    int port;
    LIBSSH2_CHANNEL *channel;

    if (!PyArg_ParseTuple(args, "si|si:direct_tcpip", &host, &port, &shost, &sport)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    channel = libssh2_channel_direct_tcpip_ex(self->session, host, port, shost, sport);
    Py_END_ALLOW_THREADS

    if (channel == NULL) {
        char *errmsg;
        int rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;
            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    return (PyObject *)channel;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_forward_listen
 */
static char PYLIBSSH2_Session_forward_listen_doc[] = "\n\
forward_listen(host, port, bound_port, queue_maxsize) -> libssh2.Listener\n\
\n\
Forwards a TCP connection through an SSH Session.\n\
\n\
@param  host: remote host\n\
@type   host: str\n\
@param  port: remote port\n\
@type   port: int\n\
@param  bound_port: populated with the actual port on the remote host\n\
@type   bound_port: int\n\
@param  queue_maxsize: maximum number of pending connections\n\
@type   int\n\
\n\
@return new libssh2.Listener instance on succes or None on failure\n\
@rtype  libssh2.Listener";

static PyObject *
PYLIBSSH2_Session_forward_listen(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    char *host;
    int port;
    int queue_maxsize;
    int *bound_port;
    LIBSSH2_LISTENER *listener;

    if (!PyArg_ParseTuple(args, "siii:forward_listen", &host, &port, &bound_port, &queue_maxsize)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    listener = libssh2_channel_forward_listen_ex(self->session, host, port,
                                                 bound_port, queue_maxsize);
    Py_END_ALLOW_THREADS

    if (listener == NULL) {
        char *errmsg;
        int rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_PROTO:
                PyErr_Format(PYLIBSSH2_Error, "An invalid SSH protocol response was received on the socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "The remote server refused the request: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }
    PyObject *list = (PyObject *)PYLIBSSH2_Listener_New(self->session, listener);
    if(list) {
        PySet_Add(self->listeners, list);
    }
    return list;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_forward_cancel
 */
static char PYLIBSSH2_Session_forward_cancel_doc[] = "";

static PyObject *
PYLIBSSH2_Session_forward_cancel(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    PYLIBSSH2_LISTENER *listen;
    int index;

    if (!PyArg_ParseTuple(args, "O:forward_cancel", &listen)) {
        return NULL;
    }

    PySet_Discard(self->listeners, (PyObject*)listen);
    Listener_cancel(listen);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_last_error
 */
static char PYLIBSSH2_Session_last_error_doc[] = "\n\
last_error() -> (int, str)\n\
\n\
Returns the last error in tuple format (code, message).\n\
\n\
@return error tuple (int, str)\n\
@rtype  tuple";

static PyObject *
PYLIBSSH2_Session_last_error(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    char *errmsg;
    int rc,want_buf=0;

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_session_last_error(self->session, &errmsg, NULL, want_buf);
    Py_END_ALLOW_THREADS

    return Py_BuildValue("(i,s)", rc, errmsg);
}
/* }}} */

/* {{{ PYLIBSSH2_Session_callback_set
 */
static char PYLIBSSH2_Session_callback_set_doc[] = "\n\
callback_set(cbtype, callback)\n\
\n\
Set (or reset) a callback function\n\
\n\
@param cbtype\n\
@param callback\n\
@return\n\
@rtype ";

/*
void
x11_callback(LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel,
             const char *shost, int sport, void **abstract)
*/

static PyObject *py_callback_func = NULL;

void
stub_x11_callback_func(LIBSSH2_SESSION *session,
                   LIBSSH2_CHANNEL *channel,
                   const char *shost,
                   int sport,
                   void **abstract)
{
    PRINTFUNCNAME
    int rc=0;

    PYLIBSSH2_SESSION *pysession;
    PYLIBSSH2_CHANNEL *pychannel;
    PyObject *pyabstract;

    /* Ensure current thread is ready to call Python C API */
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject *arglist;
    PyObject *result = NULL;

    pysession = PyObject_New(PYLIBSSH2_SESSION, &PYLIBSSH2_Session_Type);
    pysession->session = session;
    pysession->opened = 1;
    Py_XINCREF(pysession);

    pychannel = PyObject_New(PYLIBSSH2_CHANNEL, &PYLIBSSH2_Channel_Type);
    pychannel->channel = channel;
    Py_XINCREF(pychannel);

    pyabstract = Py_None;
    Py_XINCREF(pyabstract);

    arglist = Py_BuildValue("(OOsiO)",
        pysession, pychannel, shost, sport, pyabstract
    );
    Py_XINCREF(arglist);

    /* Performing Python callback with C API */
    result = PyEval_CallObject(py_callback_func, arglist);
    if (result && PyInt_Check(result)) {
        rc = PyInt_AsLong(result);
    }
    if(rc < 0) {
        PyErr_SetString(PyExc_TypeError, "An error occured while calling callback function");
        return;
    }

    /* Restore previous thread state and release acquired resources */
    PyGILState_Release(gstate);

    Py_XDECREF(pysession);
    Py_XDECREF(pychannel);
    Py_XDECREF(pyabstract);
    Py_XDECREF(arglist);
}

static PyObject *
PYLIBSSH2_Session_callback_set(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    /* type of callback to register see libssh2.h LIBSSH2_CALLBACK_* */
    int cbtype;

    /* callback is callable Python Object */
    PyObject *cb;

    if (PyArg_ParseTuple(args, "iO:callback_set", &cbtype, &cb)) {
        if (!PyCallable_Check(cb)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        if(cbtype == LIBSSH2_CALLBACK_X11) {
            PyErr_SetString(PYLIBSSH2_Error, "Only callback supported is LIBSSH2_CALLBACK_X11");
            return NULL;
        }
        Py_XINCREF(cb);
        Py_XINCREF(py_callback_func);
        py_callback_func = cb;

        Py_BEGIN_ALLOW_THREADS
        libssh2_session_callback_set(self->session, cbtype, stub_x11_callback_func);
        Py_END_ALLOW_THREADS
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_set_trace
 */
static char PYLIBSSH2_Session_set_trace_doc[] = "\n\
set_trace(bitmask)\n\
\n\
Set the trace level\n\
\n\
@param bitmask\n\
@return\n\
@rtype ";

static PyObject *
PYLIBSSH2_Session_set_trace(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc=0;
    int bitmask;

    if (!PyArg_ParseTuple(args, "i:set_trace", &bitmask)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    libssh2_trace(self->session, bitmask);
    Py_END_ALLOW_THREADS

    return Py_BuildValue("i", rc);
}
/* }}} */

/* 
void
kbd_callback(const char *name, int name_len, const char *instruction,
             int instruction_len, int num_prompts,
             const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
             LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void **abstract)
static PyObject *kbd_callback_func = NULL;
*/

static char *interactive_response;
static int interactive_response_len;

void
stub_kbd_callback_func(const char *name, int name_len,
                       const char *instruction, int instruction_len,
                       int num_prompts,
                       const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                       LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                       void **abstract)
{
    int i;
    (void)abstract;

    for (i=0; i < num_prompts; i++) {
        responses[i].text = strdup(interactive_response);
        responses[i].length = interactive_response_len;
    }
}

/* {{{ PYLIBSSH2_Session_userauth_keyboardinteractive
 */
static char PYLIBSSH2_Session_userauth_keyboardinteractive_doc[] = "\n\
userauth_keyboardinteractive(username)\n\
\n\
Authenticate a session using a challenge-response authentication\n\
\n\
@param username: name of user to attempt authentication\n";

/* {{{ PYLIBSSH2_Session_userauth_keyboardinteractive
 */
static PyObject *
PYLIBSSH2_Session_userauth_keyboardinteractive(PYLIBSSH2_SESSION *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc=0;
    char *username;
    /*PyObject *kbd_callback;*/

    if(!PyArg_ParseTuple(args, "ssi:userauth_keyboardinteractive", &username, &interactive_response, &interactive_response_len)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_userauth_keyboard_interactive(self->session, username, &stub_kbd_callback_func);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;
            
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;
            
            case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
                PyErr_Format(PYLIBSSH2_Error, "failed, invalid username/password or public/private key: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;

}

/* }}} */

/* {{{ PYLIBSSH2_Session_methods[]
 *
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 * { 'name', (PyCFunction)PYLIBSSH2_Session_name, METHOD_VARARGS }
 * for convenience
 * 
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Session_##name, METH_VARARGS, PYLIBSSH2_Session_##name##_doc }

static PyMethodDef PYLIBSSH2_Session_methods[] =
{
    ADD_METHOD(callback_set),
    ADD_METHOD(close),
    ADD_METHOD(direct_tcpip),
    ADD_METHOD(forward_listen),
    ADD_METHOD(forward_cancel),
    ADD_METHOD(hostkey_hash),
    ADD_METHOD(last_error),
    ADD_METHOD(open_session),
    ADD_METHOD(scp_recv),
    ADD_METHOD(scp_send),
    ADD_METHOD(channel_close),
    ADD_METHOD(session_methods),
    ADD_METHOD(session_method_pref),
    ADD_METHOD(set_banner),
    ADD_METHOD(set_blocking),
    ADD_METHOD(set_trace),
    ADD_METHOD(sftp_init),
    ADD_METHOD(sftp_shutdown),
    ADD_METHOD(startup),
    ADD_METHOD(userauth_agent),
    ADD_METHOD(userauth_authenticated),
    ADD_METHOD(userauth_hostbased_fromfile),
    ADD_METHOD(userauth_keyboardinteractive),
    ADD_METHOD(userauth_list),
    ADD_METHOD(userauth_password),
    ADD_METHOD(userauth_publickey_fromfile),
    { NULL, NULL }
};
#undef ADD_METHOD
/* }}} */

/* {{{ PYLIBSSH2_Session_New
 */
PYLIBSSH2_SESSION *
PYLIBSSH2_Session_New(LIBSSH2_SESSION *session)
{
    PRINTFUNCNAME
    PYLIBSSH2_SESSION *self;

    if(session == NULL) {
        return NULL;
    }

    self = PyObject_New(PYLIBSSH2_SESSION, &PYLIBSSH2_Session_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->opened = 0;
    self->socket = NULL;
    self->sftps = PySet_New(0);
    self->channels = PySet_New(0);
    self->listeners = PySet_New(0);

    libssh2_banner_set(session, LIBSSH2_SSH_DEFAULT_BANNER"_Python");

    return self;
}
/* }}} */

/* {{{ PYLIBSSH2_Session_dealloc
 */
static void
PYLIBSSH2_Session_dealloc(PYLIBSSH2_SESSION *self)
{
    PRINTFUNCNAME
    if (self) {
        if(self->session)
        {
            if (self->opened) {
                Session_close(self);
            }
            libssh2_session_free(self->session);
            self->session = NULL;

            if(self->socket) {
                Py_XDECREF(self->socket);
                self->socket = NULL;
            }
        }
        Py_XDECREF(self->sftps);
        Py_XDECREF(self->channels);
        Py_XDECREF(self->listeners);
        PyObject_Del(self);
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Session_getattr
 */
static PyObject *
PYLIBSSH2_Session_getattr(PYLIBSSH2_SESSION *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Session_methods, (PyObject *)self, name);
}
/* }}} */

/* {{{ PYLIBSSH2_Session_cmp
 */
static int
PYLIBSSH2_Session_cmp(PyObject * o1, PyObject * o2) {
    return ((PYLIBSSH2_SESSION*)o1)->session == ((PYLIBSSH2_SESSION*)o2)->session;
}
/* }}} */


/* {{{ PYLIBSSH2_Session_Type
 *
 * see /usr/include/python2.5/object.h line 261
 */
PyTypeObject PYLIBSSH2_Session_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                       /* ob_size */
    "Session",                               /* tp_name */
    sizeof(PYLIBSSH2_SESSION),               /* tp_basicsize */
    0,                                       /* tp_itemsize */
    (destructor)PYLIBSSH2_Session_dealloc,   /* tp_dealloc */
    0,                                       /* tp_print */
    (getattrfunc)PYLIBSSH2_Session_getattr,  /* tp_getattr */
    0,                                       /* tp_setattr */
    (cmpfunc)PYLIBSSH2_Session_cmp,                                       /* tp_compare */
    0,                                       /* tp_repr */
    0,                                       /* tp_as_number */
    0,                                       /* tp_as_sequence */
    0,                                       /* tp_as_mapping */
    0,                                       /* tp_hash  */
    0,                                       /* tp_call */
    0,                                       /* tp_str */
    0,                                       /* tp_getattro */
    0,                                       /* tp_setattro */
    0,                                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                      /* tp_flags */
    "Sesssion  objects",                     /* tp_doc */
};
/* }}} */

/* {{{ init_libssh2_Session
 */
int
init_libssh2_Session(PyObject *dict)
{
    PYLIBSSH2_Session_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Session_Type);
    PyDict_SetItemString(dict, "SessionType", (PyObject *)&PYLIBSSH2_Session_Type);

    return 1;
}
/* }}} */

