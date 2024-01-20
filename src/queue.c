#include "linkNode.c"

typedef struct {
	PyObject_HEAD
	linkNode* root;
	linkNode* last;
	long size;
} queue;

int c_queue_push(queue* self, PyObject* obj) {
	linkNode* new_node = linkNode_new((void*) obj);
	if (new_node == NULL)
		return 0;

	if (self->root == NULL) {
		self->root = new_node;
		self->last = new_node;
	} else {
		linkNode_union(self->last, new_node);
		self->last = new_node;
	}

	self->size++;
	return 1;
}

int c_queue_pop(queue* self) {
	linkNode* node = self->root;
	if (node == NULL)
		return 0;
	
	self->root = node->next;
	self->size--;
	return 1;
}

static PyObject* c_queue_front(queue* self) {
	if (self->root == NULL)
		return NULL;

	return (PyObject*) self->root->val;
}

static PyObject* queue_new(PyTypeObject* type) {
	queue* self;
	self = (queue*) type->tp_alloc(type, 0);

	self->size = 0;
	self->root = NULL;
	self->last = NULL;

	return (PyObject*) self;
}

static void queue_dealloc(queue* self) {
	for (linkNode* node = self->root; node != NULL;) {
		Py_XDECREF((PyObject*) node->val);
		linkNode* prevNode = node;
		node = node->next;
		free(prevNode);
	}

	Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* py_queue_push(queue* self, PyObject* args) {
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		PyErr_SetString(PyExc_MemoryError, "Error processing arguments");
		return NULL;
	}

	if (!c_queue_push(self, obj)) {
		MEMORY_ERROR;
	}

	Py_INCREF(obj);
	Py_RETURN_NONE;
}

static PyObject* py_queue_front(queue* self) {
	PyObject* obj = c_queue_front(self);
	if (obj == NULL) {
		Py_RETURN_NONE;
	}

	return obj;
}

static PyObject* py_queue_pop(queue* self) {
	PyObject* obj = c_queue_front(self);
	if (!c_queue_pop(self)) {
		PyErr_SetString(PyExc_ValueError, "Cannot pop from an empty queue");
		return NULL;
	}
	Py_XDECREF(obj);
	Py_RETURN_NONE;
}

static PyObject* py_queue_size(queue* self) {
	return PyLong_FromLong(self->size);
}

static PyMethodDef queue_methods[] = {
	{"push", (PyCFunction) py_queue_push, METH_VARARGS, "Push an element to the queue"},
	{"pop", (PyCFunction) py_queue_pop, METH_NOARGS, "Pop an element from the queue"},
	{"front", (PyCFunction) py_queue_front, METH_NOARGS, "Returns the element in the front of the queue"},
	{"size", (PyCFunction) py_queue_size, METH_NOARGS, "Returns the count of elements in the queue"},
	{NULL, NULL, 0, NULL}
};

static PyTypeObject queueType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "gstructs.queue",
	.tp_doc = "Provide low level queue operations",
	.tp_basicsize = sizeof(queue),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = (newfunc) queue_new,
	.tp_dealloc = (destructor) queue_dealloc,
	.tp_methods = queue_methods
};
