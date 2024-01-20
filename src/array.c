typedef struct {
	PyObject_HEAD
	PyObject** data;
	long size;
	long current;
} array;

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

static PyObject* array_getitem(array* self, Py_ssize_t s_index) {
	long index = (long) s_index;

	if (INVALID_INDEX(index, self->size)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return NULL;
	}

	PyObject* obj = self->data[index];
	if (obj == NULL) obj = Py_None;

	Py_INCREF(obj);
	return obj;
}

static int array_setitem(array* self, Py_ssize_t s_index, PyObject* obj) {
	long index = (long) s_index;

	if (obj == NULL) {
		Py_XDECREF(self->data[index]);
		self->data[index] = NULL;
		return 0;
	}

	if (INVALID_INDEX(index, self->size)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return -1;
	}

	Py_XDECREF(self->data[index]);
	Py_INCREF(obj);
	self->data[index] = obj;
	return 0;
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

static Py_ssize_t array_size(array* self) {
	return (Py_ssize_t) self->size;
}

static PyMethodDef array_methods[] = {
	{ NULL, NULL, 0, NULL }
};

static PySequenceMethods array_as_sequence_methods = {
	.sq_length = (lenfunc) array_size,
	.sq_item = (ssizeargfunc) array_getitem,
	.sq_ass_item = (ssizeobjargproc) array_setitem,
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
	.tp_as_sequence = &array_as_sequence_methods,
};
