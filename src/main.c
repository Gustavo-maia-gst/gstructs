#include <Python.h>
#define MEMORY_ERROR PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory"); return NULL
#define INVALID_INDEX(i, max_i) (i < 0 || i >= max_i)

#include "stack.c"
#include "queue.c"
#include "array.c"
#include "vector.c"

static PyModuleDef gstructs = {
	PyModuleDef_HEAD_INIT,
	.m_name = "gstructs",
	.m_doc = "Low level data structures for python",
	.m_size = -1
};
 
PyMODINIT_FUNC PyInit_gstructs(void) {
	PyObject* module;

    if (PyType_Ready(&stackType) < 0) {
		return NULL;
	}
    if (PyType_Ready(&queueType) < 0) {
		return NULL;
	}
    if (PyType_Ready(&arrayType) < 0) {
		return NULL;
	}
    if (PyType_Ready(&vectorType) < 0) {
		return NULL;
	}

	module = PyModule_Create(&gstructs);

	Py_INCREF(&stackType);
	PyModule_AddObject(module, "stack", (PyObject*) &stackType);
	Py_INCREF(&queueType);
	PyModule_AddObject(module, "queue", (PyObject*) &queueType);
	Py_INCREF(&arrayType);
	PyModule_AddObject(module, "array", (PyObject*) &arrayType);
	Py_INCREF(&vectorType);
	PyModule_AddObject(module, "vector", (PyObject*) &vectorType);

	return module;
}
