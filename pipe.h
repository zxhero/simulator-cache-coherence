#ifndef PIPE
#define PIPE
#include<stdio.h>
#include"message.h"
#include"list.h"

struct element{
    struct list_head head;
    struct msg *msg;
};
struct pipe{
    struct element head;
    //int num;
};

void init_pipe(struct pipe *pipe){
    memset(pipe,0,sizeof(struct pipe));
    init_list_head(&pipe->head.head);
}
void write_pipe(struct pipe* pipe, struct msg* msg){
    struct element *ele = malloc(sizeof(struct element));
    ele->msg = msg;
    struct element *entry, *next_entry;
    list_for_each_entry_safe(entry,next_entry,&pipe->head.head,head){
        if(msg->cycle >= entry->msg->cycle && (next_entry->msg == NULL || msg->cycle < next_entry->msg->cycle)){
            list_insert(&ele->head,&entry->head,&next_entry->head);
            break;
        }
    }
}; //arrange msg in cycle order

struct msg* read_pipe(struct pipe* pipe){                           //read the first msg from pipe
    struct list_head *head = pipe->head.head.next;
    struct element *ele = list_entry(head,struct element,head)
    struct msg* msg = ele->msg;
    list_delete_entry(head);
    free(ele);
    return msg;
};

struct msg* peek_at_msg(struct pipe* pipe){
    struct list_head *head = pipe->head.head.next;
    struct element *ele = list_entry(head,struct element,head)
    return ele->msg;
};
#endif // PIPE

