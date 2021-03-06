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


/* {{{ PYLIBSSH2_Sftpfile_close
 */

void
PYLIBSSH2_Sftpfile_close(PYLIBSSH2_SFTPFILE *self)
{
    PRINTFUNCNAME
    Py_BEGIN_ALLOW_THREADS
    libssh2_sftp_close_handle(self->handle);
    Py_END_ALLOW_THREADS
    self->handle = NULL;
}
/* }}} */


/* {{{ PYLIBSSH2_Sftpfile_read
 */
static char PYLIBSSH2_Sftpfile_read_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpfile_read(PYLIBSSH2_SFTPFILE *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc;
    int buffer_maxlen = 1024;
    PyObject *buffer;

    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpfile object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|i:read", &buffer_maxlen)) {
        return NULL;
    }

    buffer = PyString_FromStringAndSize(NULL, buffer_maxlen);
    if (buffer == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_read(self->handle, PyString_AsString(buffer), buffer_maxlen);
    Py_END_ALLOW_THREADS

    if (rc >= 0) {
        if ( rc != buffer_maxlen && _PyString_Resize(&buffer, rc) < 0) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return buffer;
    }

    Py_XDECREF(buffer);
    Py_INCREF(Py_None);

    return Py_None;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftpfile_write
 */
static char PYLIBSSH2_Sftpfile_write_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpfile_write(PYLIBSSH2_SFTPFILE *self, PyObject *args)
{
    PRINTFUNCNAME
    int rc, buffer_len;
    char *buffer;

    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpfile object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s#:write", &buffer, &buffer_len)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_write(self->handle, buffer, buffer_len);
    Py_END_ALLOW_THREADS

    if (rc < 0) {
        /* CLEAN: PYLIBSSH2_Sftpfile_CANT_WRITE_MSG */
        PyErr_Format(PYLIBSSH2_Error, "Unable to write sftp.");
        return NULL;
    }

    return Py_BuildValue("i", rc);

}
/* }}} */

/* {{{ PYLIBSSH2_Sftpfile_tell
 */
static char PYLIBSSH2_Sftpfile_tell_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpfile_tell(PYLIBSSH2_SFTPFILE *self, PyObject *args)
{
    PRINTFUNCNAME
    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpfile object has been closed/shutdown.");
        return NULL;
    }
    return PyInt_FromLong(libssh2_sftp_tell64(self->handle));
}
/* }}} */

/* {{{ PYLIBSSH2_Sftpfile_seek
 */
static char PYLIBSSH2_Sftpfile_seek_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftpfile_seek(PYLIBSSH2_SFTPFILE *self, PyObject *args)
{
    PRINTFUNCNAME
    unsigned long offset=0;

    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpfile object has been closed/shutdown.");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "k:seek", &offset)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    libssh2_sftp_seek64(self->handle, offset);
    Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */

/*
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *   {  'name', (PyCFunction)PYLIBSSH2_Sftpfile_name, METH_VARARGS }
 * for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Sftpfile_##name, METH_VARARGS, PYLIBSSH2_Sftpfile_##name##_doc }

static PyMethodDef PYLIBSSH2_Sftpfile_methods[] =
{
    ADD_METHOD(read),
    ADD_METHOD(write),
    ADD_METHOD(tell),
    ADD_METHOD(seek),
    { NULL, NULL }
};
#undef ADD_METHOD

PYLIBSSH2_SFTPFILE *
PYLIBSSH2_Sftpfile_New(LIBSSH2_SESSION * session, LIBSSH2_SFTP * sftp, LIBSSH2_SFTP_HANDLE *handle)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTPFILE *self;

    if(session == NULL || sftp == NULL || handle == NULL) {
        return NULL;
    }

    self = PyObject_New(PYLIBSSH2_SFTPFILE, &PYLIBSSH2_Sftpfile_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->sftp = sftp;
    self->handle = handle;

    return self;
}

static void
PYLIBSSH2_Sftpfile_dealloc(PYLIBSSH2_SFTPFILE *self)
{
    PRINTFUNCNAME
    if (self) {
        if(self->handle) {
            libssh2_sftp_close_handle(self->handle);
            self->handle = NULL;
            PyObject_Del(self);
        }
    }
}

static PyObject *
PYLIBSSH2_Sftpfile_getattr(PYLIBSSH2_SFTPFILE *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Sftpfile_methods, (PyObject *)self, name);
}

/* {{{ PYLIBSSH2_Sftpfile_hash
 */
static long
PYLIBSSH2_Sftpfile_hash(PYLIBSSH2_SFTPFILE * self) {
    long h, h1, h2, h3;
    h1 = PyObject_Hash(PyLong_FromVoidPtr(self->sftp));
    if(h1==-1) return -1;
    h2 = PyObject_Hash(PyLong_FromVoidPtr(self->session));
    if(h2==-1) return -1;
    h3 = PyObject_Hash(PyLong_FromVoidPtr(self->handle));
    if(h3==-1) return -1;
    h = h1 ^ h2;
    if(h==-1) return -2;
    return h;
}
/* }}} */

/*
 * see /usr/include/python2.5/object.c line 261
 */
PyTypeObject PYLIBSSH2_Sftpfile_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                       /* ob_size */
    "Sftpfile",                              /* tp_name */
    sizeof(PYLIBSSH2_SFTPFILE),              /* tp_basicsize */
    0,                                       /* tp_itemsize */
    (destructor)PYLIBSSH2_Sftpfile_dealloc,  /* tp_dealloc */
    0,                                       /* tp_print */
    (getattrfunc)PYLIBSSH2_Sftpfile_getattr, /* tp_getattr */
    0,                                       /* tp_setattr */
    0,                                       /* tp_compare */
    0,                                       /* tp_repr */
    0,                                       /* tp_as_number */
    0,                                       /* tp_as_sequence */
    0,                                       /* tp_as_mapping */
    (hashfunc)PYLIBSSH2_Sftpfile_hash,       /* tp_hash  */
    0,                                       /* tp_call */
    0,                                       /* tp_str */
    0,                                       /* tp_getattro */
    0,                                       /* tp_setattro */
    0,                                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                      /* tp_flags */
    "Sftpfile objects",                      /* tp_doc */
};

int
init_libssh2_Sftpfile(PyObject *dict)
{
    PYLIBSSH2_Sftpfile_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Sftpfile_Type);
    PyDict_SetItemString(dict, "SftpfileType", (PyObject *)&PYLIBSSH2_Sftpfile_Type);

    return 1;
}
