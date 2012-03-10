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

void libssh2_sftp_errno_to_exception(int err);
const char* libssh2_sftp_errno_to_str(int err);

/* {{{ PYLIBSSH2_Sftp_open_dir
 */
static char PYLIBSSH2_Sftp_open_dir_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftp_open_dir(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_SFTP_HANDLE *handle;
    char *path;
    int sftp_err;
    char* errmsg;
    int rc;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:open_dir", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    handle = libssh2_sftp_opendir(self->sftp, path);
    Py_END_ALLOW_THREADS

    if (handle == NULL) {
        rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, ": %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, " An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", , errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unable to open sftp directory %i: %s", rc, errmsg);
                return NULL;
        }
    }
    PyObject *channel = (PyObject *) PYLIBSSH2_Sftpdir_New(self->session, self->sftp, handle);
    if(channel) {
        //Py_INCREF(channel);
        PySet_Add(self->directories, channel);
    }
    return channel;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_open
 */
static char PYLIBSSH2_Sftp_open_file_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftp_open_file(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_SFTP_HANDLE *handle;
    char *path;
    char *flags = "r";
    long mode = 0755;
    int sftp_err;
    char* errmsg;
    int rc;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "ss|i:open", &path, &flags, &mode)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    handle = libssh2_sftp_open(self->sftp, path, get_flags(flags), mode);
    Py_END_ALLOW_THREADS

    if (handle == NULL) {
        rc = libssh2_session_last_error(self->session, &errmsg, NULL, 0);
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, ": %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, " An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, "Marked for non-blocking I/O but the call would block: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unable to open sftp file %i: %s", rc, errmsg);
                return NULL;
        }
    }

    PyObject *channel = (PyObject *)PYLIBSSH2_Sftpfile_New(self->session, self->sftp, handle);
    if(channel) {
        //Py_INCREF(channel);
        PySet_Add(self->files, channel);
    }
    return channel;
}
/* }}} */

/* {{{ Sftp_shutdown
 */

void
Sftp_shutdown(PYLIBSSH2_SFTP *self)
{
    PRINTFUNCNAME
    PyObject *item;

    if(self->sftp == NULL) {
        return;
    }

    while(PySet_Size(self->directories)) {
        item = PySet_Pop(self->directories);
        PYLIBSSH2_Sftpdir_close((PYLIBSSH2_SFTPDIR*)item);
        Py_XDECREF(item);
    }

    while(PySet_Size(self->files)) {
        item = PySet_Pop(self->files);
        PYLIBSSH2_Sftpdir_close((PYLIBSSH2_SFTPDIR*)item);
        Py_XDECREF(item);
    }

    libssh2_sftp_shutdown(self->sftp);
    self->sftp = NULL;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_unlink
 */
static char PYLIBSSH2_Sftp_unlink_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftp_unlink(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *path;
    int sftp_err;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:unlink", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_unlink(self->sftp, path);
    Py_END_ALLOW_THREADS

    if (rc) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // //PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while unlink an SFTP file %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_rename
 */
static char PYLIBSSH2_Sftp_rename_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftp_rename(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *src, *dst;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "ss:rename", &src, &dst)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_rename(self->sftp, src, dst);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while renaming an SFTP file %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_mkdir
 */
static char PYLIBSSH2_Sftp_mkdir_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftp_mkdir(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *path;
    long mode = 0755;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s|i:mkdir", &path, &mode)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_mkdir(self->sftp, path, mode);
    Py_END_ALLOW_THREADS

    if (rc) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while creating a directory on the remote file system %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_rmdir
 */
static char PYLIBSSH2_Sftp_rmdir_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftp_rmdir(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *path;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:rmdir", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_rmdir(self->sftp, path);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while removing an SFTP directory %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_realpath
 */
static char PYLIBSSH2_Sftp_realpath_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftp_realpath(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, target_len = 2096, sftp_err;
    char *path;
    char target[target_len];
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:realpath", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_realpath(self->sftp, path, target, target_len);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_BUFFER_TOO_SMALL: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while realpath %i: %s", rc, errmsg);
                return NULL;
        }
    }

    return PyString_FromStringAndSize(target, rc);
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_realpath
 */
static char PYLIBSSH2_Sftp_readlink_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftp_readlink(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, target_len = 2096, sftp_err;
    char *path;
    char target[target_len];
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:readlink", &path)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_readlink(self->sftp, path, target, target_len);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_BUFFER_TOO_SMALL: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Error while readlink %i: %s", rc, errmsg);
                return NULL;
        }
    }

    return PyString_FromStringAndSize(target, rc);
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_symlink
 */
static char PYLIBSSH2_Sftp_symlink_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftp_symlink(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *path, *target;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "ss:symlink", &path, &target)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_symlink(self->sftp, path, target);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_BUFFER_TOO_SMALL: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unable to sftp symlink %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_get_stat
 */
static char PYLIBSSH2_Sftp_get_stat_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftp_get_stat(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *path;
    int path_len = 0;
    int type = LIBSSH2_SFTP_STAT;
    LIBSSH2_SFTP_ATTRIBUTES attr;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s#|i:get_stat", &path, &path_len, &type)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_stat_ex(self->sftp, path, path_len, type, &attr);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unable to get stat %i: %s", rc, errmsg);
                return NULL;
        }
    }

    return sftp_attrs_to_statdict(&attr);
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_set_stat
 */
static char PYLIBSSH2_Sftp_set_stat_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";
static PyObject*
PYLIBSSH2_Sftp_set_stat(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, sftp_err;
    char *path;
    LIBSSH2_SFTP_ATTRIBUTES attr;
    PyObject *attrs;
    char* errmsg;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "sO:set_stat", &path, &attrs)) {
        return NULL;
    }

    attr.flags = 0;
    if (PyMapping_HasKeyString(attrs, "st_mode")) {
        attr.flags |= LIBSSH2_SFTP_ATTR_PERMISSIONS;
        attr.permissions = PyLong_AsLong(PyDict_GetItemString(attrs, "st_mode"));
    }

    if (PyMapping_HasKeyString(attrs, "st_uid") && PyMapping_HasKeyString(attrs, "st_gid")) {
        if (PyMapping_HasKeyString(attrs, "st_uid")) {
            attr.flags |= LIBSSH2_SFTP_ATTR_UIDGID;
            attr.uid = PyLong_AsLong(PyDict_GetItemString(attrs, "st_uid"));
        }
        if (PyMapping_HasKeyString(attrs, "st_gid")) {
            attr.flags |= LIBSSH2_SFTP_ATTR_UIDGID;
            attr.uid = PyLong_AsLong(PyDict_GetItemString(attrs, "st_gid"));
        }
    }

    if (PyMapping_HasKeyString(attrs, "st_atime") && PyMapping_HasKeyString(attrs, "st_mtime")) {
        if (PyMapping_HasKeyString(attrs, "st_atime")) {
            attr.flags |= LIBSSH2_SFTP_ATTR_ACMODTIME;
            attr.uid = PyLong_AsLong(PyDict_GetItemString(attrs, "st_atime"));
        }
        if (PyMapping_HasKeyString(attrs, "st_mtime")) {
            attr.flags |= LIBSSH2_SFTP_ATTR_ACMODTIME;
            attr.uid = PyLong_AsLong(PyDict_GetItemString(attrs, "st_mtime"));
        }
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_setstat(self->sftp, path, &attr);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        if (libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            errmsg = "";
        }
        switch (rc) {
            case LIBSSH2_ERROR_ALLOC:
                PyErr_Format(PYLIBSSH2_Error, "An internal memory allocation call failed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SOCKET_TIMEOUT:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_SOCKET_TIMEOUT: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_SFTP_PROTOCOL:
                sftp_err = libssh2_sftp_last_error(self->sftp);
                libssh2_sftp_errno_to_exception(sftp_err); // PyErr_Format(PYLIBSSH2_Error, "An invalid SFTP protocol response was received on the socket, or an SFTP operation caused an errorcode to be returned by the server %s: %s", libssh2_sftp_errno_to_str(sftp_err), errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unable to set file stat %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_set_stat
 */
static char PYLIBSSH2_Sftp_close_dir_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";
static PyObject*
PYLIBSSH2_Sftp_close_dir(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTPDIR *dir;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O:close_dir", &dir)) {
        return NULL;
    }

    PySet_Discard(self->directories, (PyObject*)dir);
    PYLIBSSH2_Sftpdir_close(dir);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */


/* {{{ PYLIBSSH2_Sftp_set_stat
 */
static char PYLIBSSH2_Sftp_close_file_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";
static PyObject*
PYLIBSSH2_Sftp_close_file(PYLIBSSH2_SFTP *self, PyObject *args)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTPFILE *file;

    if(self->sftp == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftp object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O:close_file", &file)) {
        return NULL;
    }

    PySet_Discard(self->files, (PyObject*)file);
    PYLIBSSH2_Sftpfile_close(file);


    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_methods[]
 *
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *   {  'name', (PyCFunction)PYLIBSSH2_Sftp_name, METH_VARARGS }
 * for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Sftp_##name, METH_VARARGS, PYLIBSSH2_Sftp_##name##_doc }

static PyMethodDef PYLIBSSH2_Sftp_methods[] = {
    ADD_METHOD(open_dir),
    ADD_METHOD(open_file),
    ADD_METHOD(close_dir),
    ADD_METHOD(close_file),
    ADD_METHOD(unlink),
    ADD_METHOD(rename),
    ADD_METHOD(mkdir),
    ADD_METHOD(rmdir),
    ADD_METHOD(realpath),
    ADD_METHOD(readlink),
    ADD_METHOD(symlink),
    ADD_METHOD(get_stat),
    ADD_METHOD(set_stat),
    { NULL, NULL }
};
#undef ADD_METHOD
/* }}} */

/* {{{ PYLIBSSH2_Sftp_New
 */
PYLIBSSH2_SFTP *
PYLIBSSH2_Sftp_New(LIBSSH2_SESSION *session, LIBSSH2_SFTP *sftp)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTP *self;

    if(session == NULL || sftp == NULL) {
        return NULL;
    }

    self = PyObject_New(PYLIBSSH2_SFTP, &PYLIBSSH2_Sftp_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->sftp = sftp;
    self->directories = PySet_New(0);
    self->files = PySet_New(0);
    return self;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_dealloc
 */
static void PYLIBSSH2_Sftp_dealloc(PYLIBSSH2_SFTP *self)
{
    PRINTFUNCNAME
    if (self) {
        if(self->sftp) {
            Sftp_shutdown(self);
            self->sftp = NULL;
        }
        Py_XDECREF(self->directories);
        Py_XDECREF(self->files);
        PyObject_Del(self);
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_getattr
 */
static PyObject *
PYLIBSSH2_Sftp_getattr(PYLIBSSH2_SFTP *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Sftp_methods, (PyObject *) self, name);
}
/* }}} */

/* {{{ PYLIBSSH2_Session_cmp
 */
static int
PYLIBSSH2_Sftp_cmp(PYLIBSSH2_SFTP * self, PYLIBSSH2_SFTP * other) {

    if(self) {
        if(other) {
            return self->session == other->session &&
                self->sftp == other->sftp;
        } else {
            return 1;
        }
    }
    else if(other) {
        return -1;
    } else {
        return 0;
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_hash
 */
static long
PYLIBSSH2_Sftp_hash(PYLIBSSH2_SFTP * self) {
    long h, h1, h2;
    h1 = PyObject_Hash(PyLong_FromVoidPtr(self->sftp));
    if(h1==-1) return -1;
    h2 = PyObject_Hash(PyLong_FromVoidPtr(self->session));
    if(h2==-1) return -1;
    h = h1 ^ h2;
    if(h==-1) return -2;
    return h;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftp_Type
 *
 * see /usr/include/python2.5/object.h line 261
 */
    PyTypeObject PYLIBSSH2_Sftp_Type = { PyObject_HEAD_INIT(NULL) 0, /* ob_size */
    "Sftp", /* tp_name */
    sizeof(PYLIBSSH2_SFTP), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)PYLIBSSH2_Sftp_dealloc, /* tp_dealloc */
    0, /* tp_print */
    (getattrfunc)PYLIBSSH2_Sftp_getattr, /* tp_getattr */
    0, /* tp_setattr */
    (cmpfunc)PYLIBSSH2_Sftp_cmp, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    (hashfunc)PYLIBSSH2_Sftp_hash, /* tp_hash  */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "Sftp objects", /* tp_doc */
};
/* }}} */

/* {{{ init_libssh2_Sftp
 */
int init_libssh2_Sftp(PyObject *dict)
{
    PYLIBSSH2_Sftp_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Sftp_Type);
    PyDict_SetItemString(dict, "SFTPType", (PyObject *) &PYLIBSSH2_Sftp_Type);

    return 1;
}
/* }}} */

/* {{{ libssh2_sftp_errno_to_str
 */
const char* libssh2_sftp_errno_to_str(int err)
{
    switch (err) {
        case 0:
            return "LIBSSH2_FX_OK";
        case 1:
            return "LIBSSH2_FX_EOF";
        case 2:
            return "LIBSSH2_FX_NO_SUCH_FILE";
        case 3:
            return "LIBSSH2_FX_PERMISSION_DENIED";
        case 4:
            return "LIBSSH2_FX_FAILURE";
        case 5:
            return "LIBSSH2_FX_BAD_MESSAGE";
        case 6:
            return "LIBSSH2_FX_NO_CONNECTION";
        case 7:
            return "LIBSSH2_FX_CONNECTION_LOST";
        case 8:
            return "LIBSSH2_FX_OP_UNSUPPORTED";
        case 9:
            return "LIBSSH2_FX_INVALID_HANDLE";
        case 10:
            return "LIBSSH2_FX_NO_SUCH_PATH";
        case 11:
            return "LIBSSH2_FX_FILE_ALREADY_EXISTS";
        case 12:
            return "LIBSSH2_FX_WRITE_PROTECT";
        case 13:
            return "LIBSSH2_FX_NO_MEDIA";
        case 14:
            return "LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM";
        case 15:
            return "LIBSSH2_FX_QUOTA_EXCEEDED";
        case 16:
            return "LIBSSH2_FX_UNKNOWN_PRINCIPAL";
        case 17:
            return "LIBSSH2_FX_LOCK_CONFLICT";
        case 18:
            return "LIBSSH2_FX_DIR_NOT_EMPTY";
        case 19:
            return "LIBSSH2_FX_NOT_A_DIRECTORY";
        case 20:
            return "LIBSSH2_FX_INVALID_FILENAME";
        case 21:
            return "LIBSSH2_FX_LINK_LOOP";
        default:
            return "UNKNOWN ERROR";
    }
}
/* }}} */

void libssh2_sftp_errno_to_exception(int err)
{
    switch (err) {
//case 1:
//            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(), PyString_FromString("LIBSSH2_FX_EOF")));
//            break;
        case 2:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENOENT), PyString_FromString("LIBSSH2_FX_NO_SUCH_FILE")));
            break;
        case 3:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EPERM), PyString_FromString("LIBSSH2_FX_PERMISSION_DENIED")));
            break;
        case 4:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EIO), PyString_FromString("LIBSSH2_FX_FAILURE")));
            break;
        case 5:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EBADMSG), PyString_FromString("LIBSSH2_FX_BAD_MESSAGE")));
            break;
        case 6:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EIO), PyString_FromString("LIBSSH2_FX_NO_CONNECTION")));
            break;
        case 7:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ECONNABORTED), PyString_FromString("LIBSSH2_FX_CONNECTION_LOST")));
            break;
        case 8:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EPERM), PyString_FromString("LIBSSH2_FX_OP_UNSUPPORTED")));
            break;
        case 9:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EIO), PyString_FromString("LIBSSH2_FX_INVALID_HANDLE")));
            break;
        case 10:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENOENT), PyString_FromString("LIBSSH2_FX_NO_SUCH_PATH")));
            break;
        case 11:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EEXIST), PyString_FromString("LIBSSH2_FX_FILE_ALREADY_EXISTS")));
            break;
        case 12:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EACCES), PyString_FromString("LIBSSH2_FX_WRITE_PROTECT")));
            break;
        case 13:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENODEV), PyString_FromString("LIBSSH2_FX_NO_MEDIA")));
            break;
        case 14:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENOMEM), PyString_FromString("LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM")));
            break;
        case 15:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EDQUOT), PyString_FromString("LIBSSH2_FX_QUOTA_EXCEEDED")));
            break;
        case 16:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EIO), PyString_FromString("LIBSSH2_FX_UNKNOWN_PRINCIPAL")));
            break;
        case 17:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EDEADLOCK), PyString_FromString("LIBSSH2_FX_LOCK_CONFLICT")));
            break;
        case 18:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENOTEMPTY), PyString_FromString("LIBSSH2_FX_DIR_NOT_EMPTY")));
            break;
        case 19:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ENOTDIR), PyString_FromString("LIBSSH2_FX_NOT_A_DIRECTORY")));
            break;
        case 20:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(EINVAL), PyString_FromString("LIBSSH2_FX_INVALID_FILENAME")));
            break;
        case 21:
            PyErr_SetObject(PyExc_IOError, Py_BuildValue("(OO)", PyInt_FromLong(ELOOP), PyString_FromString("LIBSSH2_FX_LINK_LOOP")));
            break;
        default:
            PyErr_Format(PYLIBSSH2_Error, "Unknown SFTP error %i", err);
            break;
    }
}

