#ifndef LISTECHAINEE_H
#define LISTECHAINEE_H

#include "ustruct.h"

void display(Match* head);
void add_head(Match* head, Match* elem);
void delete_element(Match* head, Match* to_delete);
void clear_all_matches(Match* head);

#endif