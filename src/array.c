typedef struct {
	PyObject_HEAD
	PyObject** data;
	long size;
	long current;
} array;

PyObject* c_array_getitem(array* self, long index) {
	return self->data[index];
}

PyObject* c_array_setitem(array* self, long index, PyObject* obj) {
	PyObject* old_obj = self->data[index];
	self->data[index] = obj;
	return old_obj;
}

static PyObject* array_new(PyTypeObject* type) {
	array* self;
	self = (array*) type->tp_alloc(type, 0);

	self->size = 0;
	self->current = 0;
	self->data = NULL;

	return (PyObject*) self;
}

static int array_init(array* self, PyObject* args, PyObject* kwargs) {
	long size;

	if (!PyArg_ParseTuple(args, "l", &size)) {
		return -1;
	}

	self->size = size;
	self->data = (PyObject**) malloc(self->size * sizeof(PyObject*));
	if (self->data == NULL) {
		return -1;
	}
	for (long i = 0; i < self->size; i++) self->data[i] = NULL;

	return 0;
}

static void array_dealloc(array* self) {
	for (long i = 0; i < self->size; i++)
		Py_XDECREF(self->data[i]);

	free(self->data);
	Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* py_array_getitem(array* self, PyObject* args) {
	long index;
	if (!PyArg_ParseTuple(args, "l", &index)) {
		PyErr_SetString(PyExc_ValueError, "Index should be an integer");
		return NULL;
	}

	if (INVALID_INDEX(index, self->size)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return NULL;
	}

	PyObject* obj = c_array_getitem(self, index);
	if (obj == NULL) {
		Py_RETURN_NONE;
	}

	Py_INCREF(obj);
	return obj;
}

static PyObject* py_array_setitem(array* self, PyObject* args) {
	PyObject* obj;
	long index;
	if (!PyArg_ParseTuple(args, "Ol", &obj, &index)) {
		PyErr_SetString(PyExc_ValueError, "You need to pass the object and the index");
		return NULL;
	}

	if (INVALID_INDEX(index, self->size)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return NULL;
	}

	Py_XDECREF(c_array_setitem(self, index, obj));
	Py_INCREF(obj);
	Py_RETURN_NONE;
}

static PyObject* array_iter(array* self) {
	self->current = 0;
	Py_INCREF((PyObject*) self);
	return (PyObject*) self;
}

static PyObject* array_iternext(array* self) {
	while (self->current < self->size) {
		PyObject* obj = self->data[self->current++];
		if (obj == NULL) {
			continue;
		}
		Py_INCREF(obj);
		return obj;
	}

	self->current = 0;
	return NULL;
}

static PyObject* py_array_size(array* self) {
	return PyLong_FromLong(self->size);
}

static PyMethodDef array_methods[] = {
	{ "size", (PyCFunction) py_array_size, METH_NOARGS, "Returns the size of the array" },
	{ "get", (PyCFunction) py_array_getitem, METH_VARARGS, "Returns the element in the ith position of the array, None if are not setted" },
	{ "set", (PyCFunction) py_array_setitem, METH_VARARGS, "Set the ith element of the array to the passed object" },
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject arrayType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gstructs.array",
	.tp_doc = "Provide high performance array operations",
	.tp_basicsize = sizeof(array),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_init = (initproc) array_init,
	.tp_new = (newfunc) array_new,
	.tp_dealloc = (destructor) array_dealloc,
	.tp_methods = array_methods,
	.tp_iter = (getiterfunc) array_iter,
	.tp_iternext = (iternextfunc) array_iternext,
};
