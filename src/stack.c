typedef struct {
	PyObject_HEAD
	PyObject** data;
	long sp;
	long size;
} stack;

int c_stack_push(stack* self, PyObject* obj) {
	if (self->sp >= self->size) {
		self->size *= 4;
		self->data = (PyObject**) realloc(self->data, self->size * sizeof(PyObject));
		if (self->data == NULL) {
			return 0;
		}
	}

	self->data[self->sp++] = obj;

	return 1;
}

int c_stack_pop(stack* self) {
	if (self->sp == 0)
		return 0;
	
	self->sp--;
	return 1;
}

PyObject* c_stack_top(stack* self) {
	if (self->sp == 0)
		return NULL;

	return self->data[self->sp - 1];
}

PyObject* stack_new(PyTypeObject* type) {
	stack* self;
	self = (stack*) type->tp_alloc(type, 0);

	self->sp = 0;
	self->size = 512;
	self->data = (PyObject**) malloc(self->size * sizeof(PyObject*));
	if (self->data == NULL) {
		MEMORY_ERROR;
	}

	return (PyObject*) self;
}

void stack_dealloc(stack* self) {
	for (int i = 0; i < self->sp - 1; i++)
		Py_XDECREF(self->data[i]);

	free(self->data);
	Py_TYPE(self)->tp_free((PyObject*) self);
}

PyObject* py_stack_push(stack* self, PyObject* args) {
	PyObject* obj;

	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_ValueError, "Value should be an integer");
		return NULL;
	}

	if (!c_stack_push(self, obj)) {
		MEMORY_ERROR;
	}

	Py_INCREF(obj);
	Py_RETURN_NONE;
}

PyObject* py_stack_pop(stack* self) {
	PyObject* obj = c_stack_top(self);

	if (!c_stack_pop(self)) {
		PyErr_SetString(PyExc_ValueError, "Cannot pop from an empty stack");
		return NULL;
	}

	Py_XDECREF(obj);
	Py_RETURN_NONE;
}

PyObject* py_stack_top(stack* self) {
	PyObject* obj = c_stack_top(self);
	if (obj == NULL)
		Py_RETURN_NONE;
	
	return obj;
}

PyObject* py_stack_size(stack* self) {
	return PyLong_FromLong(self->sp);
}

PyMethodDef stack_methods[] = {
	{ "push", (PyCFunction) py_stack_push, METH_VARARGS, "Push an object to the stack" },
	{ "pop", (PyCFunction) py_stack_pop, METH_NOARGS, "Pop an object from the stack" },
	{ "top", (PyCFunction) py_stack_top, METH_NOARGS, "Returns the element at the top of the stack" },
	{ "size", (PyCFunction) py_stack_size, METH_NOARGS, "Returns the number of elements in the stack" },
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject stackType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gstructs.stack",
	.tp_doc = "Provide high performance stack operations",
	.tp_basicsize = sizeof(stack),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = (newfunc) stack_new,
	.tp_dealloc = (destructor) stack_dealloc,
	.tp_methods = stack_methods
};
