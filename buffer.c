#include <stdio.h>
#include <stdlib.h>

#define BufferSize 1024 // 合适的大小你知道的

typedef struct node
{
	char *buffer;
	struct node *next;
} NODE;

NODE *InitNode(char *data) {
	NODE *head;
	head = (NODE *)malloc(sizeof(NODE));
	head->buffer = data;
	head->next = NULL;
	return head;
}

void AddData(NODE *head, char *data) { // 将新数据添加到表尾
	NODE *p = head;
	NODE *anode = (NODE *)malloc(sizeof(NODE));
	anode->buffer = (char *)malloc(sizeof(BufferSize));
	strncpy(anode->buffer,data,BufferSize);
	anode->next = NULL;
	while(p->next) p = p->next;
	p->next = anode;
}

void DealData(LinkList *head) {
LinkList *p = head->next;
if(p) {
head->next = p->next;
// p->buffer指向的数据待处理
free(p->buffer); // 处理完毕，释放空间
free(p);
}
}