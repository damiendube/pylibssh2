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


/* {{{ Channel_close
 */
void
Channel_close(PYLIBSSH2_CHANNEL *self)
{
    PRINTFUNCNAME
    if(self->channel) {
        Py_BEGIN_ALLOW_THREADS
        libssh2_channel_close(self->channel);
        Py_END_ALLOW_THREADS

        libssh2_channel_free(self->channel);
        self->channel = NULL;
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_pty
 */
static char PYLIBSSH2_Channel_pty_doc[] = "\n\
pty(term) -> int\n\
\n\
Requests a pty with term type on a channel.\n\
\n\
@param  term: terminal emulation type (vt100, ansi, etc...)\n\
@type   term: str\n\
\n\
@return 0 on succes or negative on failure\n\
@rtype  int";

/*
 * refer libssh2_channel_request_pty_ex()
 */
static PyObject *
PYLIBSSH2_Channel_pty(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    /* terminal type */
    char *term;
    /* terminal type length */
    int term_len;
    /* terminal modes capabilities */
    char *modes = NULL;
    /* modes length */
    int modes_len = 0;
    /* terminal dimensions in characters */
    int width = 80, height = 24;
    /* default dimensions */
    int width_px = 0, height_px = 0;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s#|s#iiii:pty", &term, &term_len, &modes,
                          &modes_len, &width, &height, &width_px, &height_px)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_request_pty_ex(self->channel, term, term_len, modes, modes_len,
                                        width, height, width_px, height_px);
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

            case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_pty_resize
 */
static char PYLIBSSH2_Channel_pty_resize_doc[] = "\n\
pty_resize(width, height, width_px, height_px) -> int\n\
\n\
Requests a pty resize on a channel with the given width and height.\n\
\n\
@param  width: terminal width\n\
@type   width: int\n\
@param  height: terminal height\n\
@type   height: int\n\
@param  width_px: terminal width in pixel (opt)\n\
@type   height: int\n\
@param  height_px: terminal height in pixel (opt)\n\
@type   height: int\n\
\n\
@return 0 on success or negative on failure\n\
@rtype  int";

/*
 * refer libssh2_channel_request_pty_size_ex()
 */
static PyObject*
PYLIBSSH2_Channel_pty_resize(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc = -1;

    /* terminal dimensions in characters */
    int width  = 80;
    int height = 24;

    /* terminal dimensions in pixels */
    int width_px  = 0;
    int height_px = 0;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args,"ii|ii:pty_resize", &width, &height,
                                                   &width_px, &height_px)){
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_request_pty_size_ex(self->channel, width, height,
                                                width_px, height_px);
    Py_END_ALLOW_THREADS

    if (rc) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_shell
 */
static char PYLIBSSH2_Channel_shell_doc[] = "\n\
shell() -> int\n\
\n\
Requests a shell on the channel.\n\
\n";

static PyObject*
PYLIBSSH2_Channel_shell(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_shell(self->channel);
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

            case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_execute
 */
static char PYLIBSSH2_Channel_execute_doc[] = "\n\
execute(command) -> int\n\
\n\
Executes command on the channel.\n\
\n\
@param  command: message data\n\
@type   command: str\n\
\n";

static PyObject*
PYLIBSSH2_Channel_execute(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    /* command to execute */
    char *command;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s:execute", &command)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_exec(self->channel, command);
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

            case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_setenv
 */
static char PYLIBSSH2_Channel_setenv_doc[] = "\n\
setenv(name, value) -> int\n\
\n\
Sets envrionment variable on the channel.\n\
\n\
@param name: evironment variable name\n\
@type  name: str\n\
@param value: evironment variable value\n\
@type  value: str\n";

static PyObject*
PYLIBSSH2_Channel_setenv(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    /* variable envrionnement name */
    char *env_key;
    /* variable envrionnement value */
    char *env_val;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "ss:setenv", &env_key, &env_val)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_setenv(self->channel, env_key, env_val);
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

            case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_setblocking
 */
static char PYLIBSSH2_Channel_setblocking_doc[] = "\n\
setblocking([mode])\n\
\n\
Sets blocking mode on the channel. Default mode is blocking.\n\
\n\
@param  mode: blocking (1) or non blocking (0) mode\n\
@type   mode: int";

static PyObject*
PYLIBSSH2_Channel_setblocking(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    /* 1 blocking, 0 non blocking */
    int block = 1;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "i:setblocking", &block)) {
        return NULL;
    }

    libssh2_channel_set_blocking(self->channel, block);

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_read
 */
static char PYLIBSSH2_Channel_read_doc[] = "\n\
read(size) -> str\n\
\n\
Reads size bytes on the channel.\n\
\n\
@param size: size of the buffer storage\n\
@type  size: int\n\
\n\
@return bytes read or negative on failure\n\
@rtype  str";

static PyObject *
PYLIBSSH2_Channel_read(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc = 0;
    int buffer_size= 1024;
    char * cbuf;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|i:read", &buffer_size)) {
        return NULL;
    }

    if (libssh2_channel_eof(self->channel) != 1) {

        cbuf = malloc(buffer_size);
        if (cbuf == NULL) {
            return NULL;
        }

        Py_BEGIN_ALLOW_THREADS
        rc = libssh2_channel_read(self->channel, cbuf, buffer_size);
        Py_END_ALLOW_THREADS

        if(rc >= 0) {
            PyObject* returnObj = PyByteArray_FromStringAndSize(cbuf, rc);
            free(cbuf);
            return returnObj;
        }
        else {
            free(cbuf);

            char *errmsg;
            if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
                // This is not the error that failed, do not take the string.
                errmsg = "";
            }
            switch(rc) {
                case LIBSSH2_ERROR_SOCKET_SEND:
                    PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                    return NULL;

                case LIBSSH2_ERROR_CHANNEL_CLOSED:
                    PyErr_Format(PYLIBSSH2_Error, "The channel has been closed: %s", errmsg);
                    return NULL;

                default:
                    PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                    return NULL;
            }
        }
    }
    else {
        return PyString_FromString("");
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_read_ex
 */
static char PYLIBSSH2_Channel_read_ex_doc[] = "\n\
read(size, [stream_id]) -> str\n\
\n\
Reads size bytes on the channel.\n\
\n\
@param size: size of the buffer storage\n\
@type  size: int\n\
@param stream_id: substream ID number\n\
@type  stream_id: int\n\
\n\
@return string containing bytes read or negative value on failure\n\
@rtype  str or int";

static PyObject *
PYLIBSSH2_Channel_read_ex(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int buffer_size = 1024;
    int stream_id = 0;
    /* buffer to read as a python object */
    PyObject *buffer;
    char * cbuf;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|ii:read_ex", &buffer_size, &stream_id)) {
        return NULL;
    }
    buffer = PyString_FromStringAndSize(NULL, buffer_size);
    if (buffer == NULL) {
        return NULL;
    }
    cbuf = PyString_AsString(buffer);

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_read_ex(self->channel, stream_id, cbuf, buffer_size);
    Py_END_ALLOW_THREADS

    if (rc > 0) {
       if (rc != buffer_size && _PyString_Resize(&buffer, rc) < 0)
            return NULL;
    }
    else {

        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_CHANNEL_CLOSED:
                PyErr_Format(PYLIBSSH2_Error, "The channel has been closed: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }
    /**
       we do NOT increment the reference on the buffer anymore
       it is already done by PyString_FromStringAndSize
    */
    return Py_BuildValue("iN", rc, buffer);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_write
 */
static char PYLIBSSH2_Channel_write_doc[] = "\n\
write(message) -> int\n\
\n\
Writes data on a channel.\n\
\n\
@param  message: data to write\n\
@type   message: str\n\
\n\
@return written bytes on success\n\
@rtype  int";

static PyObject *
PYLIBSSH2_Channel_write(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    char *message;
    int message_len;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s#:write", &message, &message_len)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_write(self->channel, message, message_len);
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

            case LIBSSH2_ERROR_CHANNEL_CLOSED:
                PyErr_Format(PYLIBSSH2_Error, "The channel has been closed: %s", errmsg);
                return NULL;

            case LIBSSH2_ERROR_CHANNEL_EOF_SENT:
                PyErr_Format(PYLIBSSH2_Error, "The channel has been requested to be closed: %s", errmsg);
                return NULL;

            default:
                PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i: %s", rc, errmsg);
                return NULL;
        }
    }

    return Py_BuildValue("i", rc);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_flush
 */
static char PYLIBSSH2_Channel_flush_doc[] = "\n\
flush() -> int\n\
\n\
Flushs the read buffer for a given channel.\n";

static PyObject*
PYLIBSSH2_Channel_flush(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_flush(self->channel);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
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

/* {{{ PYLIBSSH2_Channel_exit_status
 */
static char PYLIBSSH2_Channel_exit_status_doc[] = "\n\
exit_status() -> int \n\
\n\
Gets the exit code raised by the process running on the remote host.\n\
\n\
@return the exit status reported by remote host or 0 on failure\n\
@rtype  int";

static PyObject *
PYLIBSSH2_Channel_exit_status(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    rc = libssh2_channel_get_exit_status(self->channel);
    if(rc == 0) {
	    PyErr_SetString(PYLIBSSH2_Error, "Unable to get exit status.");
        return NULL;
    }
    return Py_BuildValue("i", rc);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_eof
 */
static char PYLIBSSH2_Channel_eof_doc[] = "\n\
eof() -> int\n\
\n\
Checks if the remote host has sent an EOF status.\n\
\n\
@return 1 if the remote host has sent EOF otherwise 0\n\
@rtype   int";

static PyObject *
PYLIBSSH2_Channel_eof(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }
    rc = libssh2_channel_eof(self->channel);
    if(rc < 0) {
        /* CLEAN: PYLIBSSH2_CANT_FLUSH_CHANNEL_MSG */
        PyErr_SetString(PYLIBSSH2_Error, "Unable to check channel eof.");
        return NULL;
    }
    return Py_BuildValue("i", rc);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_send_eof
 */
static char PYLIBSSH2_Channel_send_eof_doc[] = "\n\
send_eof() -> int\n\
\n\
Sends EOF status on the channel to remote server.\n";

static PyObject*
PYLIBSSH2_Channel_send_eof(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_send_eof(self->channel);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_SOCKET_SEND:
                PyErr_Format(PYLIBSSH2_Error, "Unable to send data on socket.: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_wait_eof
 */
static char PYLIBSSH2_Channel_wait_eof_doc[] = "\n\
wait_eof() -> int\n\
\n\
Sends EOF status on the channel to remote server.\n";

static PyObject*
PYLIBSSH2_Channel_wait_eof(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_wait_eof(self->channel);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, ": %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_wait_closed
 */
static char PYLIBSSH2_Channel_wait_closed_doc[] = "\n\
wait_closed() -> int\n\
\n\
Wait for the remote channel to ack channel close.\n\
\n\
@param channel\n\
@type libssh2.Channel\n";

static PyObject*
PYLIBSSH2_Channel_wait_closed(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_wait_closed(self->channel);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        char *errmsg;
        if(libssh2_session_last_error(self->session, &errmsg, NULL, 0) != rc) {
            // This is not the error that failed, do not take the string.
            errmsg = "";
        }
        switch(rc) {
            case LIBSSH2_ERROR_EAGAIN:
                PyErr_Format(PYLIBSSH2_Error, ": %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_window_read
 */
static char PYLIBSSH2_Channel_window_read_doc[] = "\n\
window_read(read_avail, window_size_initial) -> int\n\
\n\
Checks the status of the read window.\n\
\n\
\n\
@return windows: the number of bytes which the remote end may send without\n\
@return read_avail: number of bytes available to be read\n\
@return window_size_initial: as defined by the channel_open request\n\
@rtype  (int, int, int)";

static PyObject *
PYLIBSSH2_Channel_window_read(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    unsigned long rc=0;
    unsigned long read_avail;
    unsigned long window_size_initial;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_window_read_ex(self->channel, &read_avail, &window_size_initial);
    Py_END_ALLOW_THREADS
    
    return Py_BuildValue("(kkk)", rc, read_avail, window_size_initial);
}
/* }}} */

static char PYLIBSSH2_Channel_window_write_doc[] = "\n"
"window_write(write_avail) -> int\n"
"\n"
"check the status of the write window.\n"
"\n"
"\n"
"@return windows: the number of bytes which may be safely writen on the\n"
"channel without blocking."
"@return window_size_initial: as defined by the channel_open request\n"
"@rtype  (int, int)";

static PyObject *
PYLIBSSH2_Channel_window_write(PYLIBSSH2_CHANNEL *self)
{
    PRINTFUNCNAME
    unsigned long rc=0;
    unsigned long window_size_initial;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_window_write_ex(self->channel, &window_size_initial);
    Py_END_ALLOW_THREADS

    return Py_BuildValue("(kk)", rc, window_size_initial);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_x11_req
 */
static char PYLIBSSH2_Channel_x11_req_doc[] = "\n\
x11_req([single_connection, auth_proto, auth_cookie, display]) -> int\n\
\n\
Requests an X11 Forwarding on the channel.\n\
\n\
@param  single_connection: forward a single connection\n\
@type  single_connection: int\n\
@param  auth_proto: X11 authentication protocol to use\n\
@type  auth_proto: str\n\
@param  auth_cookie: the cookie\n\
@type  auth_cookie: str\n\
@param  display: screen number\n\
@type  display: int\n";

static PyObject*
PYLIBSSH2_Channel_x11_req(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int single_connection = 0;
    char *auth_proto = NULL;
    char *auth_cookie = NULL;
    int display = 0;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|issi:x11_req", &single_connection, &auth_proto, &auth_cookie, &display)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_channel_x11_req_ex(self->channel, single_connection, auth_proto, auth_cookie, display);
    Py_END_ALLOW_THREADS

    if(rc < 0) {
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

            case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
                PyErr_Format(PYLIBSSH2_Error, "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: %s", errmsg);
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

/* {{{ PYLIBSSH2_Channel_poll_read
 */
static char PYLIBSSH2_Channel_poll_read_doc[] = "\n\
poll_read([extended]) -> int\n\
\n\
Checks if data is available on the channel.\n\
\n\
@param  extended: if message channel datas is extended\n\
@type   extended: int\n\
\n\
@return 1 when data is available or 0 otherwise\n\
@rtype  int";

static PyObject *
PYLIBSSH2_Channel_poll_read(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int extended = 0;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|i:poll_read", &extended)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_poll_channel_read(self->channel, extended);
    Py_END_ALLOW_THREADS;

    return Py_BuildValue("i", rc);
}
/* }}} */


/* {{{ PYLIBSSH2_Channel_receive_window_adjust
 */
static char PYLIBSSH2_Channel_receive_window_adjust_doc[] = "\n\
receive_window_adjust(adjustment, [force]) -> int\n\
\n\
Adjust the channel window.\n\
\n\
@param  adjustment: adjustment bytes\n\
@type   adjustment: int\n\
\n\
@param  force: force\n\
@type   force: int\n\
\n\
@return window size\n\
@rtype  int";

static PyObject *
PYLIBSSH2_Channel_receive_window_adjust(PYLIBSSH2_CHANNEL *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int adjustment;
    int force = 0;
    unsigned int window;

    if(self->channel == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Channel object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "i|i:receive_window_adjust", &adjustment, &force)) {
        return NULL;
    }

    rc = libssh2_channel_receive_window_adjust2(self->channel, adjustment, force, &window);

    if(rc < 0) {
		PyErr_Format(PYLIBSSH2_Error, "Unknown Error %i", rc);
		return NULL;
    }
    return Py_BuildValue("i", window);
}
/* }}} */


/* {{{ PYLIBSSH2_Channel_methods[]
 *
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *  { 'name', (PyCFunction)PYLIBSSH2_Channel_name, METHOD_VARARGS }
 *  for convenience
 */

#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Channel_##name, METH_VARARGS, PYLIBSSH2_Channel_##name##_doc }
static PyMethodDef PYLIBSSH2_Channel_methods[] =
{
    //ADD_METHOD(close),
    ADD_METHOD(pty),
    ADD_METHOD(pty_resize),
    ADD_METHOD(shell),
    ADD_METHOD(execute),
    ADD_METHOD(setenv),
    ADD_METHOD(setblocking),
    ADD_METHOD(read_ex),
    ADD_METHOD(read),
    ADD_METHOD(write),
    ADD_METHOD(flush),
    ADD_METHOD(eof),
    ADD_METHOD(exit_status),
    ADD_METHOD(send_eof),
    ADD_METHOD(wait_eof),
    ADD_METHOD(wait_closed),
    ADD_METHOD(window_read),
    ADD_METHOD(window_write),
    ADD_METHOD(poll_read),
    ADD_METHOD(x11_req),
    ADD_METHOD(receive_window_adjust),
    { NULL, NULL }
};
#undef ADD_METHOD
/* }}} */

/* {{{ PYLIBSSH2_Channel_New
 */
PYLIBSSH2_CHANNEL *
PYLIBSSH2_Channel_New(LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel)
{
    PRINTFUNCNAME
    PYLIBSSH2_CHANNEL *self;

    if(session == NULL || channel == NULL) {
        return NULL;
    }

    self = PyObject_New(PYLIBSSH2_CHANNEL, &PYLIBSSH2_Channel_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->channel = channel;

    return self;
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_dealloc
 */
static void
PYLIBSSH2_Channel_dealloc(PYLIBSSH2_CHANNEL *self)
{
    PRINTFUNCNAME
    if (self) {
        if(self->channel) {
            Channel_close(self);
        }
        PyObject_Del(self);
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_getattr
 */
static PyObject *
PYLIBSSH2_Channel_getattr(PYLIBSSH2_CHANNEL *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Channel_methods, (PyObject *)self, name);
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_hash
 */
static long
PYLIBSSH2_Channel_hash(PYLIBSSH2_CHANNEL * self) {
    long h, h1, h2;
    h1 = PyObject_Hash(PyLong_FromVoidPtr(self->channel));
    if(h1==-1) return -1;
    h2 = PyObject_Hash(PyLong_FromVoidPtr(self->session));
    if(h2==-1) return -1;
    h = h1 ^ h2;
    if(h==-1) return -2;
    return h;
}
/* }}} */

/* {{{ PYLIBSSH2_Channel_Type
 * see /usr/include/python2.5/object.h line 261
 */
PyTypeObject PYLIBSSH2_Channel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                      /* ob_size */
    "Channel",                              /* tp_name */
    sizeof(PYLIBSSH2_CHANNEL),              /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)PYLIBSSH2_Channel_dealloc,  /* tp_dealloc */
    0,                                      /* tp_print */
    (getattrfunc)PYLIBSSH2_Channel_getattr, /* tp_getattr */
    0,                                      /* tp_setattr */
    0,                                      /* tp_compare */
    0,                                      /* tp_repr */
    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */
    (hashfunc)PYLIBSSH2_Channel_hash,       /* tp_hash  */
    0,                                      /* tp_call */
    0,                                      /* tp_str */
    0,                                      /* tp_getattro */
    0,                                      /* tp_setattro */
    0,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    "Channel objects",                      /* tp_doc */
};
/* }}} */

 /* {{{ init_libssh2_Channel
 */
int
init_libssh2_Channel(PyObject *dict)
{
    PYLIBSSH2_Channel_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Channel_Type);
    PyDict_SetItemString(dict, "ChannelType", (PyObject *)&PYLIBSSH2_Channel_Type);

    return 1;
}
/* }}} */
