/*-
 * pylibssh2 - python bindings for libssh2 library
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


/* {{{ PYLIBSSH2_Channel_close
 */
static char PYLIBSSH2_Agent_connect_doc[] = "";

static PyObject *
PYLIBSSH2_Agent_connect(PYLIBSSH2_AGENT *self, PyObject *args)
{
    int rc;

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_agent_connect(self->agent);
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

    self->opened = 1;

    Py_INCREF(Py_None);
    return Py_None;
}
/* }}} */


/* {{{ PYLIBSSH2_Channel_close
 */
static char PYLIBSSH2_Agent_disconnect_doc[] = "";

static PyObject *
PYLIBSSH2_Agent_disconnect(PYLIBSSH2_AGENT *self, PyObject *args)
{
    int rc;

    Py_BEGIN_ALLOW_THREADS
    rc = libssh2_agent_disconnect(self->agent);
    Py_END_ALLOW_THREADS

    self->opened = 0;

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

/* {{{ PYLIBSSH2_Agent_get_identity
 */
static char PYLIBSSH2_Agent_get_identities_doc[] = "";

static PyObject *
PYLIBSSH2_Agent_get_identities(PYLIBSSH2_AGENT *self, PyObject *args)
{
    int rc;
    struct libssh2_agent_publickey *store = NULL;

    rc = libssh2_agent_list_identities(self->agent);
    if (rc) {
        PyErr_Format(PYLIBSSH2_Error, "Unknown Error");
        return NULL;
    }

    PyObject* pyList = PyList_New(0);

    while(1) {
        rc = libssh2_agent_get_identity(self->agent, &store, store);
        if (rc < 0) {
            PyErr_Format(PYLIBSSH2_Error, "Unknown Error");
            return NULL;
        }
        else if(rc == 0){
            PyList_Append(pyList, (PyObject*)PYLIBSSH2_Agent_PublicKey_New(store));
        }
        else if(rc == 1){
            break;
        }
    }

    Py_XINCREF(pyList);
    return pyList;
}
/* }}} */


/* {{{ PYLIBSSH2_Agent_methods[]
 *
 * ADD_METHOD(name) expands to a correct PyMethodDef declaration
 *  { 'name', (PyCFunction)PYLIBSSH2_Agent_name, METHOD_VARARGS }
 *  for convenience
 */
#define ADD_METHOD(name) \
{ #name, (PyCFunction)PYLIBSSH2_Agent_##name, METH_VARARGS, PYLIBSSH2_Agent_##name##_doc }

struct PyMethodDef PYLIBSSH2_Agent_methods[] = {
    ADD_METHOD(connect),
    ADD_METHOD(disconnect),
    ADD_METHOD(get_identities),
    { NULL, NULL }
};
#undef ADD_METHOD
/* }}} */

/* {{{ PYLIBSSH2_Agent_New
 */
PYLIBSSH2_AGENT *
PYLIBSSH2_Agent_New(LIBSSH2_SESSION *session, LIBSSH2_AGENT *agent, int dealloc)
{
    PYLIBSSH2_AGENT *self;

    self = PyObject_New(PYLIBSSH2_AGENT, &PYLIBSSH2_Agent_Type);
    if (self == NULL) {
        return NULL;
    }

    self->session = session;
    self->agent =   agent;
    self->dealloc = dealloc;
    self->opened =  0;

    return self;
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_dealloc
 */
static void
PYLIBSSH2_Agent_dealloc(PYLIBSSH2_AGENT *self)
{
    if (self->dealloc) {
        libssh2_agent_free(self->agent);
    }

    if (self) {
        PyObject_Del(self);
    }
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_getattr
 */
static PyObject *
PYLIBSSH2_Agent_getattr(PYLIBSSH2_AGENT *self, char *name)
{
    return Py_FindMethod(PYLIBSSH2_Agent_methods, (PyObject *) self, name);
}
/* }}} */

/* {{{ PYLIBSSH2_Agent_Type
 *
 * see /usr/include/python2.5/object.h line 261
 */
PyTypeObject PYLIBSSH2_Agent_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                       /* ob_size */
    "Agent",                              /* tp_name */
    sizeof(PYLIBSSH2_AGENT),              /* tp_basicsize */
    0,                                       /* tp_itemsize */
    (destructor)PYLIBSSH2_Agent_dealloc,  /* tp_dealloc */
    0,                                       /* tp_print */
    (getattrfunc)PYLIBSSH2_Agent_getattr, /* tp_getattr */
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
    "Agent objects",                      /* tp_doc */
};
/* }}} */

/* {{{ init_libssh2_Agent
 */
int
init_libssh2_Agent(PyObject *dict)
{
    PYLIBSSH2_Agent_Type.ob_type = &PyType_Type;
    Py_INCREF(&PYLIBSSH2_Agent_Type);
    PyDict_SetItemString(dict, "AgentType", (PyObject *)&PYLIBSSH2_Agent_Type);

    return 1;
}
/* }}} */
