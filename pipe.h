#ifndef PIPE
#define PIPE
#include<stdio.h>
#include<string.h>
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
    /*struct element *entry, *next_entry;
    if(list_empty(&pipe->head.head)){
        list_add_head(&ele->head,&pipe->head.head);
        return;
    }
    list_for_each_entry(entry,&pipe->head.head,head){
        if(msg->cycle <= entry->msg->cycle){
            list_insert(&ele->head,entry->head.prev,&entry->head);
            return;
        }
    }*/
    list_add_tail(&ele->head,&pipe->head.head);
}; //arrange msg in cycle order

struct msg* read_pipe(struct pipe* pipe){                           //read the first msg from pipe
    if(list_empty(&pipe->head.head))    return NULL;
    struct list_head *head = pipe->head.head.next;
    struct element *ele = list_entry(head,struct element,head);
    struct msg* msg = ele->msg;
    list_delete_entry(head);
    free(ele);
    return msg;
};

struct msg* peek_at_msg(struct pipe* pipe){
    if(list_empty(&pipe->head.head))    return NULL;
    struct list_head *head = pipe->head.head.next;
    struct element *ele = list_entry(head,struct element,head);
    return ele->msg;
};
#endif // PIPE

