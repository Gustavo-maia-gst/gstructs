#include <Python.h>
#define MEMORY_ERROR PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory"); return NULL

#include "stack.c"
#include "queue.c"


static PyModuleDef gstructs = {
	PyModuleDef_HEAD_INIT,
	.m_name = "gstructs",
	.m_doc = "High performance data structures for python",
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

	module = PyModule_Create(&gstructs);

	Py_INCREF(&stackType);
	PyModule_AddObject(module, "stack", (PyObject*) &stackType);

	Py_INCREF(&queueType);
	PyModule_AddObject(module, "queue", (PyObject*) &queueType);
	
	return module;
}
