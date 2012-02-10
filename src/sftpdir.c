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
static char PYLIBSSH2_Sftpdir_close_doc[] = "\n\
myfunction(name, value) -> returnType \n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject*
PYLIBSSH2_Sftpdir_close(PYLIBSSH2_SFTPDIR *self, PyObject *args)
{
    int rc;

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_sftp_close_handle(self->handle);
    Py_END_ALLOW_THREADS

    if (rc) {
        /* CLEAN: PYLIBSSH2_SFTPFILE_CANT_CLOSE_MSG */
        PyErr_SetString(PYLIBSSH2_Error, "Unable to close sftp handle.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
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
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int buffer_maxlen = 0;
    int longentry_maxlen = 255;
    PyObject *buffer;

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

    PyObject *dict = get_attrs(&attrs);
    return Py_BuildValue("NO", buffer, dict);
}
/* }}} */

/* {{{ PYLIBSSH2_Sftpdir_listdir
 */
static char PYLIBSSH2_Sftpdir_list_doc[] = "\n\
\n\
Arguments:\n\
\n\
Returns:\n\
";

static PyObject *
PYLIBSSH2_Sftpdir_list(PYLIBSSH2_SFTPDIR *self, PyObject *args)
{
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int buffer_maxlen = 0;
    int longentry_maxlen = 255;
    PyObject *buffer;
    PyObject *all = NULL;
    PyObject *list = NULL;

    all = PyList_New(0);
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

        list = PyList_New(0);
        PyList_Append(list, buffer);
        PyList_Append(list, get_attrs(&attrs));
        PyList_Append(all, list);
    }

    return all;
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
    ADD_METHOD(close),
    ADD_METHOD(read),
    ADD_METHOD(list),
    { NULL, NULL }
};
#undef ADD_METHOD

PYLIBSSH2_SFTPDIR *
PYLIBSSH2_Sftpdir_New(LIBSSH2_SESSION* session, LIBSSH2_SFTP* sftp, LIBSSH2_SFTP_HANDLE *handle, int dealloc)
{
    PYLIBSSH2_SFTPDIR *self;

    self = PyObject_New(PYLIBSSH2_SFTPDIR, &PYLIBSSH2_Sftpdir_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->sftp = sftp;
    self->handle = handle;
    self->dealloc = dealloc;

    return self;
}

static void
PYLIBSSH2_Sftpdir_dealloc(PYLIBSSH2_SFTPDIR *self)
{
    PyObject_Del(self);
}

static PyObject *
PYLIBSSH2_Sftpdir_getattr(PYLIBSSH2_SFTPDIR *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Sftpdir_methods, (PyObject *)self, name);
}

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
    0,                                      /* tp_hash  */
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
