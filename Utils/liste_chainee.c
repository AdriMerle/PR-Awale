#include "liste_chainee.h"

void display(Match* head){
    Match* p = head;
    while (p!=NULL){//check if list is empty or if we are at the end
        printf("%d\r\n", p->game->score[0]);
        p = p->next;
    }
    if(p==head) printf("List is empty.\r\n");
    else printf("End of list.\r\n");
}

void add_head(Match* head, Match* new){
    new->next=head; 
    head=new;
}

void delete(Match* head, Match* to_delete){
    int position =0;

    //Deleting
    Match* p = head;
    Match* previous;
    Match* temp;
    if(p==NULL){
        printf("Empty list, contains no values\r\n");
    }else{
        while(p!=NULL){//while not at the end
            if(p==head && p==to_delete) {//if we have to delete head
                temp = p;
                head = p->next;
                free(temp);
            } else if(p==to_delete) {//if at the middle or end
                previous->next=p->next;
                temp = p;
                free(temp);
            }
            previous = p;
            p = p->next;
        }
    }
}

void clear_all_matches(Match* head){
    Match* p = head;
    Match* temp;
    while(p!=NULL){//while not at the end
        temp = p;
        p = p->next;
        free(temp->game));
        free(temp);
    }
}