typedef enum {
	INTEGER = 1,
	DOUBLE = 2
} num_type;

#define NUM_GET(type, src_obj) ((type == INTEGER) ? PyLong_AsLong(src_obj) : PyFloat_AsDouble(src_obj))
#define OBJ_GET(type, src_ptr) ((type == INTEGER) ? PyLong_FromLong((long) src_ptr) : PyFloat_FromDouble((double) src_ptr))

typedef struct {
	PyObject_HEAD
	void* data;
	long alloc_size;
	long length;
	long longSum;
	double doubleSum;
	long current;
	num_type data_type;
} vector;

int c_vector_realloc(vector* self) {
	self->alloc_size *= 4;
	self->data = realloc(self->data, self->alloc_size * sizeof(double));
	if (self->data == NULL) {
		return 0;
	}
	return 1;
}

int c_vector_long_append(vector* self, long elem) {
	long* data = (long*) self->data;
	data[self->length++] = elem;

	self->longSum += elem;

	return 1;
}

int c_vector_double_append(vector* self, double elem) {
	double* data = (double*) self->data;
	data[self->length++] = elem;

	self->doubleSum += elem;

	return 1;
}

int c_vector_append(vector* self, PyObject* elem) {
	if (self->length >= self->alloc_size) {
		if (!c_vector_realloc(self))
			return 0;
	}

	switch (self->data_type) {
		case INTEGER:
			if (!c_vector_long_append(self, PyLong_AsLong(elem)))
				return 0;
			break;
		case DOUBLE:
			if (!c_vector_double_append(self, PyFloat_AsDouble(elem)))
				return 0;
			break;
	}

	return 1;
}

int c_vector_long_erase(vector* self, long index) {
	long* data = (long*) self->data;
	long item = data[index];
	
	self->longSum -= item;
	self->length--;

	for (long i = index+1; i < self->length; i++) {
		data[i-1] = data[i];
	}

	return 1;
}

int c_vector_double_erase(vector* self, long index) {
	double* data = (double*) self->data;
	double item = data[index];
	
	self->doubleSum -= item;
	self->length--;

	for (long i = index+1; i < self->length; i++) {
		data[i-1] = data[i];
	}

	return 1;
}

int c_vector_erase(vector* self, long index) {
	switch (self->data_type) {
		case INTEGER:
			if (!c_vector_long_erase(self, index))
				return 0;
			break;
		case DOUBLE:
			if (!c_vector_double_erase(self, index))
				return 0;
			break;
	}

	return 1;
}

int c_vector_long_set(vector* self, long elem, long index) {
	long* data = (long*) self->data;
	self->longSum += elem - data[index];
	data[index] = elem;
	return 1;
}

int c_vector_double_set(vector* self, double elem, long index) {
	double* data = (double*) self->data;
	self->doubleSum += elem - data[index];
	data[index] = elem;
	return 1;
}

int c_vector_set(vector* self, PyObject* elem, long index) {
	switch (self->data_type) {
		case INTEGER:
			if (!c_vector_long_set(self, PyLong_AsLong(elem), index))
				return 0;
			break;
		case DOUBLE:
			if (!c_vector_double_set(self, PyFloat_AsDouble(elem), index))
				return 0;
			break;
	}

	return 1;
}

static PyObject* vector_new(PyTypeObject* type) {
	vector* self;
	self = (vector*) type->tp_alloc(type, 0);

	self->alloc_size = 0;
	self->current = 0;
	self->data = NULL;
	self->data_type = 0;
	self->length = 0;

	return (PyObject*) self;
}

static int vector_init(vector* self, PyObject* args, PyObject* kwargs) {
	char* ind_type = "i";
	PyObject* list = NULL;

	if (!PyArg_ParseTuple(args, "s|O", &ind_type, &list)) {
		return -1;
	}

	switch (*ind_type) {
		case 'i':
			self->data_type = INTEGER;
			self->longSum = 0;
			break;
		case 'f':
			self->data_type = DOUBLE;
			self->doubleSum = 0.0;
			break;
		default:
			PyErr_SetString(PyExc_ValueError, "Unknown datatype, use 'f' to float and 'i' to integer");
			return -1;
	}

	self->length = 0;
	self->alloc_size = 1024;
	self->data = malloc(self->alloc_size * sizeof(double));
	if (self->data == NULL) {
		return -1;
	}


	if (list != NULL) {
		PyObject* iterator = PyObject_GetIter(list);
		if (iterator == NULL) {
			PyErr_SetString(PyExc_ValueError, "The object should be an iterable");
			return -1;
		}

		PyObject* item;
		while ((item = PyIter_Next(iterator)) != NULL) {
			if (!c_vector_append(self, item)) {
				PyErr_SetString(PyExc_ValueError, "All the elements in the iterable should be numbers of the vector type");
				return -1;
			}

			Py_XDECREF(item);
		}
		Py_XDECREF(iterator);
	}

	return 0;
}

static void vector_dealloc(vector* self) {
	free(self->data);
	Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* vector_getitem(vector* self, Py_ssize_t s_index) {
	long index = (long) s_index;

	if (INVALID_INDEX(index, self->length)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return NULL;
	}

	PyObject* obj;
	
	if (self->data_type == INTEGER) {
		long* data = (long*) self->data;
		obj = PyLong_FromLong(data[index]);
	} else {
		double* data = (double*) self->data;
		obj = PyFloat_FromDouble(data[index]);
	}

	Py_INCREF(obj);

	return obj;
}

int vector_object_is_valid(num_type data_type, PyObject* obj) {
	switch (data_type) {
		case INTEGER:
			if (!PyLong_Check(obj)) {
				PyErr_SetString(PyExc_ValueError, "The value should be an integer for an integer vector");
				return 0;
			}
			break;
		case DOUBLE:
			if (!PyFloat_Check(obj)) {
				PyErr_SetString(PyExc_ValueError, "The value should be a float for a float vector");
				return 0;
			}
			break;
	}

	return 1;
}

static int vector_setitem(vector* self, Py_ssize_t s_index, PyObject* obj) {
	long index = (long) s_index;

	if (obj == NULL) {
		c_vector_erase(self, index);
		return 0;
	}

	if (!vector_object_is_valid(self->data_type, obj))
		return -1;

	if (INVALID_INDEX(index, self->length)) {
		PyErr_SetString(PyExc_IndexError, "Index out of bounds");
		return -1;
	}
	 
	c_vector_set(self, obj, index);

	return 0;
}

static PyObject* vector_append(vector* self, PyObject* args) {
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_ValueError, "You should pass one number to the append method");
		return NULL;
	}

	if (!c_vector_append(self, obj))
		return NULL;

	Py_RETURN_NONE;
}

static PyObject* vector_pop(vector* self) {
	if (!self->length) {
		PyErr_SetString(PyExc_ValueError, "Cannot pop from an empty vector");
		return NULL;
	}

	if (!c_vector_erase(self, self->length - 1)) {
		PyErr_SetString(PyExc_ValueError, "Error erasing from vector");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject* vector_sum(vector* self) {
	if (!self->length) {
		Py_RETURN_NONE;
	}
	switch (self->data_type) {
		case INTEGER:
			return PyLong_FromLong(self->longSum);
		case DOUBLE:
			return PyFloat_FromDouble(self->doubleSum);
	}
}

static Py_ssize_t vector_size(vector* self) {
	return (Py_ssize_t) self->length;
}

static PyMethodDef vector_methods[] = {
	{ "append", (PyCFunction) vector_append, METH_VARARGS, "Append a number to the vector" },
	{ "pop", (PyCFunction) vector_pop, METH_NOARGS, "Pop the last number from the vector" },
	{ "sum", (PyCFunction) vector_sum, METH_NOARGS, "Returns the sum of the vector" },
	{ NULL, NULL, 0, NULL }
};

static PySequenceMethods vector_as_sequence_methods = {
	.sq_length = (lenfunc) vector_size,
	.sq_item = (ssizeargfunc) vector_getitem,
	.sq_ass_item = (ssizeobjargproc) vector_setitem,
};

static PyTypeObject vectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gstructs.vector",
	.tp_doc = "Provide low level vector operations",
	.tp_basicsize = sizeof(vector),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_init = (initproc) vector_init,
	.tp_new = (newfunc) vector_new,
	.tp_dealloc = (destructor) vector_dealloc,
	.tp_methods = vector_methods,
	.tp_as_sequence = &vector_as_sequence_methods,
};
