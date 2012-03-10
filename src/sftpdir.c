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



/* {{{ PYLIBSSH2_Sftpdir_close
 */

void
PYLIBSSH2_Sftpdir_close(PYLIBSSH2_SFTPDIR *self)
{
    PRINTFUNCNAME
    Py_BEGIN_ALLOW_THREADS
    libssh2_sftp_close_handle(self->handle);
    Py_END_ALLOW_THREADS
    self->handle = NULL;
}
/* }}} */

/* {{{ PYLIBSSH2_Sftpdir_readdir
 */
static char PYLIBSSH2_Sftpdir_read_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpdir_read(PYLIBSSH2_SFTPDIR *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int buffer_maxlen = 0;
    int longentry_maxlen = 255;
    PyObject *buffer;

    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpdir object has been closed/shutdown.");
        return NULL;
    }

    buffer = PyString_FromStringAndSize(NULL, longentry_maxlen);
    if (buffer == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_BEGIN_ALLOW_THREADS
    buffer_maxlen = libssh2_sftp_readdir(self->handle, PyString_AsString(buffer),
                                         longentry_maxlen, &attrs);
    Py_END_ALLOW_THREADS

    if (buffer_maxlen == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    } else if ( buffer_maxlen == -1) {
        /* CLEAN: PYLIBSSH2_SFTPDIR_CANT_READDIR_MSG */
        PyErr_SetString(PYLIBSSH2_Error, "Unable to readdir.");
        return NULL;
    }

    if (buffer_maxlen != longentry_maxlen &&
        _PyString_Resize(&buffer, buffer_maxlen) < 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject *dict = sftp_attrs_to_statdict(&attrs);
    return Py_BuildValue("NO", buffer, dict);
}
/* }}} */

/* {{{ PYLIBSSH2_Sftpdir_listdir
 */
static char PYLIBSSH2_Sftpdir_list_files_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpdir_list_files(PYLIBSSH2_SFTPDIR *self, PyObject *args)
{
    PRINTFUNCNAME
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int buffer_maxlen = 0;
    int longentry_maxlen = 255;
    PyObject *buffer;
    PyObject *dict = NULL;

    if(self->handle == NULL) {
        PyErr_Format(PYLIBSSH2_Error, "Sftpdir object has been closed/shutdown.");
        return NULL;
    }

    dict = PyDict_New();
    while (1) {
        buffer = PyString_FromStringAndSize(NULL, longentry_maxlen);
        if (buffer == NULL) {
            Py_INCREF(Py_None);
            return Py_None;
        }

        Py_BEGIN_ALLOW_THREADS
        buffer_maxlen = libssh2_sftp_readdir(self->handle,
            PyString_AsString(buffer), longentry_maxlen, &attrs);
        Py_END_ALLOW_THREADS

        if (buffer_maxlen == 0) {
            break;
        } else if (buffer_maxlen == -1) {
            PyErr_SetString(PYLIBSSH2_Error, "Unable to listdir.");
            return NULL;
        }

        if ( buffer_maxlen != longentry_maxlen &&
             _PyString_Resize(&buffer, buffer_maxlen) < 0) {
            Py_INCREF(Py_None);
            return Py_None;
        }

        PyDict_SetItem(dict, buffer, sftp_attrs_to_statdict(&attrs));
    }

    return dict;
}
/* }}} */


/*
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *   {  'name', (PyCFunction)PYLIBSSH2_Sftpdir_name, METH_VARARGS }
 * for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Sftpdir_##name, METH_VARARGS, PYLIBSSH2_Sftpdir_##name##_doc }

static PyMethodDef PYLIBSSH2_Sftpdir_methods[] =
{
    ADD_METHOD(read),
    ADD_METHOD(list_files),
    { NULL, NULL }
};
#undef ADD_METHOD

PYLIBSSH2_SFTPDIR *
PYLIBSSH2_Sftpdir_New(LIBSSH2_SESSION* session, LIBSSH2_SFTP* sftp, LIBSSH2_SFTP_HANDLE *handle)
{
    PRINTFUNCNAME
    PYLIBSSH2_SFTPDIR *self;

    if(session == NULL || sftp == NULL || handle == NULL) {
        return NULL;
    }

    self = PyObject_New(PYLIBSSH2_SFTPDIR, &PYLIBSSH2_Sftpdir_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->sftp = sftp;
    self->handle = handle;

    return self;
}

static void
PYLIBSSH2_Sftpdir_dealloc(PYLIBSSH2_SFTPDIR *self)
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
PYLIBSSH2_Sftpdir_getattr(PYLIBSSH2_SFTPDIR *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Sftpdir_methods, (PyObject *)self, name);
}

/* {{{ PYLIBSSH2_Sftpdir_hash
 */
static long
PYLIBSSH2_Sftpdir_hash(PYLIBSSH2_SFTPDIR * self) {
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
PyTypeObject PYLIBSSH2_Sftpdir_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                      /* ob_size */
    "Sftpdir",                              /* tp_name */
    sizeof(PYLIBSSH2_SFTPDIR),              /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)PYLIBSSH2_Sftpdir_dealloc,  /* tp_dealloc */
    0,                                      /* tp_print */
    (getattrfunc)PYLIBSSH2_Sftpdir_getattr, /* tp_getattr */
    0,                                      /* tp_setattr */
    0,                                      /* tp_compare */
    0,                                      /* tp_repr */
    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */
    (hashfunc)PYLIBSSH2_Sftpdir_hash,       /* tp_hash  */
    0,                                      /* tp_call */
    0,                                      /* tp_str */
    0,                                      /* tp_getattro */
    0,                                      /* tp_setattro */
    0,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    "Sftpdir objects",                      /* tp_doc */
};

int
init_libssh2_Sftpdir(PyObject *dict)
{
    PYLIBSSH2_Sftpdir_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Sftpdir_Type);
    PyDict_SetItemString(dict, "SftpdirType", (PyObject *)&PYLIBSSH2_Sftpdir_Type);

    return 1;
}
