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

FILE* logFile = NULL;

/* {{{ PYLIBSSH2_doc
 */
PyDoc_STRVAR(PYLIBSSH2_doc,
"Python binding for libssh2 library\n\
\n\
pylibssh2 is a C extension module around C library libssh2. It provide an\n\
easy way to manage SSH connections in Python.\n\
\n\
The high-level API start with creation of an L{Session} object with a\n\
socket-like object connected. Then create a L{Channel} with L{open_session}\n\
method on the session instance.\n\
\n");

/* }}} */

PyObject *PYLIBSSH2_Error;

/* {{{ PYLIBSSH2_Session
 */
PyDoc_STRVAR(PYLIBSSH2_Session_doc,
"\n\
This class provide SSH Session operations.\n\
\n\
agent() -- init an ssh-agent\
close() -- closes the session\n\
direct_tcpip() -- tunnels a TCP connection\n\
forward_listen() -- forwards a TCP connection\n\
hostkey_hash() -- returns the computed digest of the remote host key\n\
last_error() -- returns the last error in tuple format\n\
open_session() -- allocates a new channel\n\
scp_recv() -- requests a remote file via SCP protocol\n\
scp_send() -- sends a remote file via SCP protocol\n\
session_method_pref() -- sets preferred methods to be negociated\n\
session_methods() -- returns a dictionnary with the currently active algorithms\n\
set_banner() -- sets the banner that will be sent to remote host\n\
sftp_init() -- opens an SFTP Channel\n\
startup() -- perform the SSH startup\n\
userauth_authenticated() -- returns authentification status\n\
userauth_list() -- lists the authentification methods\n\
userauth_password() -- authenticates a session with credentials\n\
userauth_publickey() -- authenticates a session with a publickey\n\
userauth_publickey_fromfile() -- authenticates a session with publickey\n\
");

static PyObject *
PYLIBSSH2_Session(PyObject *self, PyObject *args)
{
    LIBSSH2_SESSION *session;
    session = libssh2_session_init();
    if(session == NULL) {
        return NULL;
    }
    return (PyObject *)PYLIBSSH2_Session_New(session);
}
/* }}} */

/* {{{ PYLIBSSH2_methods[]
 */
static PyMethodDef PYLIBSSH2_methods[] = {
    { "Session", (PyCFunction)PYLIBSSH2_Session, METH_VARARGS, PYLIBSSH2_Session_doc },
    { NULL, NULL }
};
/* }}} */

/* {{{ init_libssh2
 */
PyMODINIT_FUNC
init_libssh2(void)
{
    static void *PYLIBSSH2_API[PYLIBSSH2_API_pointers];
    PyObject *c_api_object;
    PyObject *module, *dict;

    module = Py_InitModule3(
        PYLIBSSH2_MODULE_NAME, 
        PYLIBSSH2_methods, 
        PYLIBSSH2_doc
    );
    if (module == NULL) {
        return;
    }

#ifdef DEBUG
    logFile = fopen("/tmp/pylibssh2.log", "w");
#endif

    PYLIBSSH2_API[PYLIBSSH2_Session_New_NUM] = (void *) PYLIBSSH2_Session_New;
    PYLIBSSH2_API[PYLIBSSH2_Channel_New_NUM] = (void *) PYLIBSSH2_Channel_New;
    PYLIBSSH2_API[PYLIBSSH2_Sftp_New_NUM] = (void *) PYLIBSSH2_Sftp_New;
    PYLIBSSH2_API[PYLIBSSH2_Sftpfile_New_NUM] = (void *) PYLIBSSH2_Sftpfile_New;
    PYLIBSSH2_API[PYLIBSSH2_Sftpdir_New_NUM] = (void *) PYLIBSSH2_Sftpdir_New;

    c_api_object = PyCObject_FromVoidPtr((void *)PYLIBSSH2_API, NULL);
    if (c_api_object != NULL) {
        PyModule_AddObject(module, "_C_API", c_api_object);
    }

    PYLIBSSH2_Error = PyErr_NewException(
        PYLIBSSH2_MODULE_NAME".Error",
        NULL,
        NULL
    );
    if (PYLIBSSH2_Error == NULL) {
        goto error;
    }
    if (PyModule_AddObject(module, "Error", PYLIBSSH2_Error) != 0) {
        goto error;
    }

    PyModule_AddIntConstant(module, "FINGERPRINT_MD5", 0x0000);
    PyModule_AddIntConstant(module, "FINGERPRINT_SHA1", 0x0001);
    PyModule_AddIntConstant(module, "FINGERPRINT_HEX", 0x0000);
    PyModule_AddIntConstant(module, "FINGERPRINT_RAW", 0x0002);

    PyModule_AddIntConstant(module, "METHOD_KEX",  LIBSSH2_METHOD_KEX);
    PyModule_AddIntConstant(module, "METHOD_HOSTKEY",  LIBSSH2_METHOD_HOSTKEY);
    PyModule_AddIntConstant(module, "METHOD_CRYPT_CS",  LIBSSH2_METHOD_CRYPT_CS);
    PyModule_AddIntConstant(module, "METHOD_CRYPT_SC",  LIBSSH2_METHOD_CRYPT_SC);
    PyModule_AddIntConstant(module, "METHOD_MAC_CS",  LIBSSH2_METHOD_MAC_CS);
    PyModule_AddIntConstant(module, "METHOD_MAC_SC",  LIBSSH2_METHOD_MAC_SC);
    PyModule_AddIntConstant(module, "METHOD_COMP_CS",  LIBSSH2_METHOD_COMP_CS);
    PyModule_AddIntConstant(module, "METHOD_COMP_SC",  LIBSSH2_METHOD_COMP_SC);

    PyModule_AddIntConstant(module, "SFTP_SYMLINK", LIBSSH2_SFTP_SYMLINK);
    PyModule_AddIntConstant(module, "SFTP_READLINK", LIBSSH2_SFTP_READLINK);
    PyModule_AddIntConstant(module, "SFTP_REALPATH", LIBSSH2_SFTP_REALPATH);

    PyModule_AddIntConstant(module, "SFTP_STAT", LIBSSH2_SFTP_STAT);
    PyModule_AddIntConstant(module, "SFTP_LSTAT", LIBSSH2_SFTP_LSTAT);
    
    PyModule_AddStringConstant(module, "DEFAULT_BANNER", LIBSSH2_SSH_DEFAULT_BANNER"_Python");
    PyModule_AddStringConstant(module, "LIBSSH2_VERSION", LIBSSH2_VERSION);

    PyModule_AddIntConstant(module, "CHANNEL_FLUSH_ALL", LIBSSH2_CHANNEL_FLUSH_ALL);
    PyModule_AddIntConstant(module, "CHANNEL_FLUSH_EXTENDED_DATA", LIBSSH2_CHANNEL_FLUSH_EXTENDED_DATA);

    dict = PyModule_GetDict(module);
    if (!init_libssh2_Session(dict)) {
        goto error;
    }
    if (!init_libssh2_Channel(dict)) {
        goto error;
    }
    if (!init_libssh2_Sftp(dict)) {
        goto error;
    }
    if (!init_libssh2_Sftpfile(dict)) {
        goto error;
    }
    if (!init_libssh2_Sftpdir(dict)) {
        goto error;
    }

    error:
    ;
}
/* }}} */

/* {{{ sftp_attrs_to_statdict
 */
PyObject *
sftp_attrs_to_statdict(LIBSSH2_SFTP_ATTRIBUTES *attr)
{
    PyObject *attrs=NULL;

    attrs = PyDict_New();
    PyDict_SetItem(attrs, PyString_FromString("st_size"), PyLong_FromUnsignedLong(
        (unsigned long)attr->filesize));
    PyDict_SetItem(attrs, PyString_FromString("st_uid"), PyLong_FromUnsignedLong(
        (unsigned long)attr->uid));
    PyDict_SetItem(attrs, PyString_FromString("st_gid"), PyLong_FromUnsignedLong(
        (unsigned long)attr->gid));
    PyDict_SetItem(attrs, PyString_FromString("st_mode"), PyLong_FromUnsignedLong(
        (unsigned long)attr->permissions));
    PyDict_SetItem(attrs, PyString_FromString("st_atime"), PyLong_FromUnsignedLong(
        (unsigned long)attr->atime));
    PyDict_SetItem(attrs, PyString_FromString("st_mtime"), PyLong_FromUnsignedLong(
        (unsigned long)attr->mtime));
    PyDict_SetItem(attrs, PyString_FromString("st_flags"), PyLong_FromUnsignedLong(
        (unsigned long)attr->flags));

    return attrs;
}
/* }}} */


/* {{{ stat_to_statdict
 */
PyObject *
stat_to_statdict(struct stat *attr){
    PyObject *attrs=NULL;

    attrs = PyDict_New();
    PyDict_SetItem(attrs, PyString_FromString("st_size"), PyLong_FromUnsignedLong(
        (unsigned long)attr->st_size));
    PyDict_SetItem(attrs, PyString_FromString("st_uid"), PyLong_FromUnsignedLong(
        (unsigned long)attr->st_uid));
    PyDict_SetItem(attrs, PyString_FromString("st_gid"), PyLong_FromUnsignedLong(
        (unsigned long)attr->st_gid));
    PyDict_SetItem(attrs, PyString_FromString("st_mode"), PyLong_FromUnsignedLong(
        (unsigned long)attr->st_mode));
    PyDict_SetItem(attrs, PyString_FromString("st_atime"), PyFloat_FromDouble(
        attr->st_atim.tv_sec + attr->st_mtim.tv_nsec * 0.000000001));
    PyDict_SetItem(attrs, PyString_FromString("st_mtime"), PyFloat_FromDouble(
        attr->st_mtim.tv_sec + attr->st_mtim.tv_nsec * 0.000000001));

    return attrs;
}
/* }}} */
