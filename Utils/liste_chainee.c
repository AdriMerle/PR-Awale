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

<<<<<<< HEAD
void add_head(Match** head, Match* new){
    new->next=*head; 
    *head=new;
=======
void add_head(Match* head, Match* elem){
    new->next=head; 
    head=new;
>>>>>>> f1deaaf3a3b74b495de423d45a3bbee51fb097a7
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