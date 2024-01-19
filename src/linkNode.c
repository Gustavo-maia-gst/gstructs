typedef struct linkNode {
	struct linkNode* next;
	struct linkNode* previous;
	void* val;
} linkNode;

linkNode* linkNode_new(void* val) {
	linkNode* node = (linkNode*) malloc(sizeof(linkNode));
	if (node == NULL)
		return NULL;

	node->next = NULL;
	node->previous = NULL;
	node->val = val;

	return node;
}

void linkNode_union(linkNode* previousNode, linkNode* nextNode) {
	previousNode->next = nextNode;
	nextNode->previous = previousNode;
}
