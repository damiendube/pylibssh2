/*-
 * pylibssh2 - python bindings for libssh2 library
 *
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

/* {{{ PYLIBSSH2_Agent_PublicKey_methods[]
 *
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *  { 'name', (PyCFunction)PYLIBSSH2_Agent_PublicKey_name, METHOD_VARARGS }
 *  for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Agent_PublicKey_##name, METH_VARARGS, PYLIBSSH2_Agent_PublicKey_##name##_doc }

struct PyMethodDef PYLIBSSH2_Agent_PublicKey_methods[] = {
    { NULL, NULL }
};
#undef ADD_METHOD
/* }}} */

/* {{{ PYLIBSSH2_Agent_PublicKey_New
 */
PYLIBSSH2_AGENT_PUBLICKEY *
PYLIBSSH2_Agent_PublicKey_New(struct libssh2_agent_publickey *publickey)
{
    PYLIBSSH2_AGENT_PUBLICKEY *self;

    self = PyObject_New(PYLIBSSH2_AGENT_PUBLICKEY, &PYLIBSSH2_Agent_PublicKey_Type);
    if (self == NULL) {
        return NULL;
    }

    self->publickey = publickey;

    return self;
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_PublicKey_dealloc
 */
static void
PYLIBSSH2_Agent_PublicKey_dealloc(PYLIBSSH2_LISTENER *self)
{
    if (self) {
        PyObject_Del(self);
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_PublicKey_getattr
 */
static PyObject *
PYLIBSSH2_Agent_PublicKey_getattr(PYLIBSSH2_LISTENER *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Agent_PublicKey_methods, (PyObject *) self, name);
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_PublicKey_Type
 *
 * see /usr/include/python2.5/object.h line 261
 */
PyTypeObject PYLIBSSH2_Agent_PublicKey_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                       /* ob_size */
    "Agent_PublicKey",                              /* tp_name */
    sizeof(PYLIBSSH2_LISTENER),              /* tp_basicsize */
    0,                                       /* tp_itemsize */
    (destructor)PYLIBSSH2_Agent_PublicKey_dealloc,  /* tp_dealloc */
    0,                                       /* tp_print */
    (getattrfunc)PYLIBSSH2_Agent_PublicKey_getattr, /* tp_getattr */
    0,                                       /* tp_setattr */
    0,                                       /* tp_compare */
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
    "Agent_PublicKey objects",                      /* tp_doc */
};
/* }}} */

/* {{{ init_libssh2_Agent_PublicKey
 */
int
init_libssh2_Agent_PublicKey(PyObject *dict)
{
    PYLIBSSH2_Agent_PublicKey_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Agent_PublicKey_Type);
    PyDict_SetItemString(dict, "Agent_PublicKeyType", (PyObject *)&PYLIBSSH2_Agent_PublicKey_Type);

    return 1;
}
/* }}} */
