#ifndef LISTECHAINEE_H
#define LISTECHAINEE_H

#include "ustruct.h"

void display(Match* head);
void add_head(Match* head, Match* new);
void delete(Match* head, Match* to_delete);
void clear_matches(Match* head);

#endif