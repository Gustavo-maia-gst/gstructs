#define B_RIGHT(i) (2 * i + 1)
#define B_LEFT(i) (2 * i)
#define B_PARENT(i) (i == 1 ? -1 : i / 2)

typedef struct {
    PyObject_HEAD
	long* data;
	long* children;
    long size;
	long length;
} maxHeap;

int c_maxHeap_push(maxHeap* self, long n, long i) {
	if (i >= self->size) {
		self->data = (long*) realloc(self->data, 4 * self->size * sizeof(long));
		if (self->data == NULL)
			return 0;
		self->children = (long*) realloc(self->children, 4 * self->size * sizeof(long));
		if (self->data == NULL)
			return 0;

		for (long i = self->size; i < 4 * self->size; i++) self->data[i] = LONG_MIN;
		for (long i = self->size; i < 4 * self->size; i++) self->children[i] = 0;
		self->size *= 4;
	}

	if (self->data[i] == LONG_MIN) {
		self->data[i] = n;
		self->length++;
		self->children[i]++;
		return 1;
	}

	long current = self->data[i];

	if (n > current) {
		self->data[i] = n;
		n = current;
	}

	if (self->children[B_RIGHT(i)] < self->children[B_LEFT(i)]) {
		self->children[B_RIGHT(i)]++;
		c_maxHeap_push(self, n, B_RIGHT(i));
	} else {
		self->children[B_LEFT(i)]++;
		c_maxHeap_push(self, n, B_LEFT(i));
	}
}

int c_maxHeap_pop(maxHeap* self, long i) {
	long left = self->data[B_LEFT(i)];
	long right = self->data[B_RIGHT(i)];

	if (right == LONG_MIN) {
		self->data[i] = left;
		self->length--;
		return 1;
	}

	long new_i = left > right ? B_LEFT(i) : B_RIGHT(i);
	self->data[i] = self->data[new_i];

	return c_maxHeap_pop(self, new_i);
}

long c_maxHeap_front(maxHeap* self) {
    return self->data[1];
}

static PyObject* maxHeap_new(PyTypeObject* type) {
    maxHeap* self;
    self = (maxHeap*) type->tp_alloc(type, 0);

    self->size = 4 * 1024;
	self->length = 0;
	self->data = (long*) malloc(self->size * sizeof(long));
	if (self->data == NULL) {
		MEMORY_ERROR;
		return NULL;
	}
	for (long i = 0; i < self->size; i++) self->data[i] = LONG_MIN;

	self->children = (long*) malloc(self->size * sizeof(long));
	if (self->data == NULL) {
		MEMORY_ERROR;
		return NULL;
	}
	for (long i = 0; i < self->size; i++) self->children[i] = 0;

    return (PyObject*) self;
}

static void maxHeap_dealloc(maxHeap* self) {
	free(self->data);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* py_maxHeap_push(maxHeap* self, PyObject* args) {
    long n;
    if (!PyArg_ParseTuple(args, "i", &n)) {
        PyErr_SetString(PyExc_MemoryError, "You should pass an integer to enmaxHeap*");
        return NULL;
    }

    if (!c_maxHeap_push(self, n, 1)) {
        MEMORY_ERROR;
		return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* py_maxHeap_front(maxHeap* self) {
    long n = c_maxHeap_front(self);
    if (n == LONG_MIN) {
        Py_RETURN_NONE;
    }

    return PyLong_FromLong(n);
}

static PyObject* py_maxHeap_pop(maxHeap* self) {
    if (!c_maxHeap_pop(self, 1)) {
        PyErr_SetString(PyExc_ValueError, "Cannot pop from an empty maxHeap*");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* py_maxHeap_size(maxHeap* self) {
    return PyLong_FromLong(self->length);
}

static PyObject* py_maxHeap_arrSize(maxHeap* self) {
    return PyLong_FromLong(self->size);
}

static PyMethodDef maxHeap_methods[] = {
        {"push", (PyCFunction) py_maxHeap_push, METH_VARARGS, "Push an element to the maxHeap"},
        {"pop", (PyCFunction) py_maxHeap_pop, METH_NOARGS, "Pop an element from the maxHeap"},
        {"front", (PyCFunction) py_maxHeap_front, METH_NOARGS, "Returns the element in the front of the maxHeap"},
        {"size", (PyCFunction) py_maxHeap_size, METH_NOARGS, "Returns the count of elements in the maxHeap"},
        {"arrSize", (PyCFunction) py_maxHeap_arrSize, METH_NOARGS, "Returns the count of elements in the maxHeap"},
        {NULL, NULL, 0, NULL}
};

static PyTypeObject maxHeapType = {
        PyVarObject_HEAD_INIT(NULL, 0)
		.tp_name = "gstructs.maxHeap",
        .tp_doc = "Provide low level maxHeap operations",
        .tp_basicsize = sizeof(maxHeap),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_new = (newfunc) maxHeap_new,
        .tp_dealloc = (destructor) maxHeap_dealloc,
        .tp_methods = maxHeap_methods
};
