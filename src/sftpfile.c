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

/*
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *   {  'name', (PyCFunction)PYLIBSSH2_Sftpfile_name, METH_VARARGS }
 * for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Sftpfile_##name, METH_VARARGS, PYLIBSSH2_Sftpfile_##name##_doc }

static PyMethodDef PYLIBSSH2_Sftpfile_methods[] =
{
    { NULL, NULL }
};

PYLIBSSH2_SFTPFILE *
PYLIBSSH2_Sftpfile_New(LIBSSH2_SFTP_HANDLE *sftpfile, int dealloc)
{
    PYLIBSSH2_SFTPFILE *self;

    self = PyObject_New(PYLIBSSH2_SFTPFILE, &PYLIBSSH2_Sftpfile_Type);
    if (self == NULL) {
        return NULL;
    }

    self->sftpfile = sftpfile;
    self->dealloc = dealloc;

    return self;
}

static void
PYLIBSSH2_Sftpfile_dealloc(PYLIBSSH2_SFTPFILE *self)
{
    PyObject_Del(self);
}

static PyObject *
PYLIBSSH2_Sftpfile_getattr(PYLIBSSH2_SFTPFILE *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Sftpfile_methods, (PyObject *)self, name);
}

/*
 * see /usr/include/python2.5/object.c line 261
 */
PyTypeObject PYLIBSSH2_Sftpfile_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                     /* ob_size */
    "Sftpfile",                          /* tp_name */
    sizeof(PYLIBSSH2_SFTPFILE),               /* tp_basicsize */
    0,                                     /* tp_itemsize */
    (destructor)PYLIBSSH2_Sftpfile_dealloc,    /* tp_dealloc */
    0,                                     /* tp_print */
    (getattrfunc)PYLIBSSH2_Sftpfile_getattr,  /* tp_getattr */
    0,                                     /* tp_setattr */
    0,                                     /* tp_compare */
    0,                                     /* tp_repr */
    0,                                     /* tp_as_number */
    0,                                     /* tp_as_sequence */
    0,                                     /* tp_as_mapping */
    0,                                     /* tp_hash  */
    0,                                     /* tp_call */
    0,                                     /* tp_str */
    0,                                     /* tp_getattro */
    0,                                     /* tp_setattro */
    0,                                     /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                    /* tp_flags */
    "Sftpfile objects",                  /* tp_doc */
};

int
init_libssh2_Sftpfile(PyObject *dict)
{
    PYLIBSSH2_Sftpfile_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Sftpfile_Type);
    PyDict_SetItemString(dict, "SftpfileType", (PyObject *)&PYLIBSSH2_Sftpfile_Type);

    return 1;
}
