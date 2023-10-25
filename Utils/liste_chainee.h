#ifndef LISTECHAINEE_H
#define LISTECHAINEE_H

#include "ustruct.h"

void display(Match* head);
<<<<<<< HEAD
void add_head(Match** head, Match* new);
=======
void add_head(Match* head, Match* elem);
void delete_element(Match* head, Match* to_delete);
>>>>>>> f1deaaf3a3b74b495de423d45a3bbee51fb097a7
void clear_all_matches(Match* head);

#endif