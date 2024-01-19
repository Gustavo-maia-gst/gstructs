typedef struct {
	PyObject_HEAD
	PyObject** data;
	long size;
} array;

PyObject* array_new(PyTypeObject* type, PyObject*) {
	array* self;
	self = (array*) type->tp_alloc(type, 0);

	self->size = 512;
	self->data = (PyObject**) malloc(self->size * sizeof(PyObject*));
	if (self->data == NULL) {
		MEMORY_ERROR;
	}

	return (PyObject*) self;
}

void array_dealloc(array* self) {
	for (int i = 0; i < self->sp - 1; i++)
		Py_XDECREF(self->data[i]);

	free(self->data);
	Py_TYPE(self)->tp_free((PyObject*) self);
}

PyMethodDef array_methods[] = {
	{ "push", (PyCFunction) py_array_push, METH_VARARGS, "Push an object to the array" },
	{ "pop", (PyCFunction) py_array_pop, METH_NOARGS, "Pop an object from the array" },
	{ "top", (PyCFunction) py_array_top, METH_NOARGS, "Returns the element at the top of the array" },
	{ "size", (PyCFunction) py_array_size, METH_NOARGS, "Returns the number of elements in the array" },
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject arrayType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gstructs.array",
	.tp_doc = "Provide high performance array operations",
	.tp_basicsize = sizeof(array),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = (newfunc) array_new,
	.tp_dealloc = (destructor) array_dealloc,
	.tp_methods = array_methods
};
