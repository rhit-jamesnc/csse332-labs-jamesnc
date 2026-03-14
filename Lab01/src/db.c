/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"

/**
 * Implementation of node_mk_node
 */
void
node_mk_node(struct oo_node *node)
{
  node->next = node->prev = node;
}

/**
 * Implementation of node_rm_node
 */
void
node_rm_node(struct oo_node *node)
{
  // TODO: Add your code here...
  node->prev->next = node->next;
  
  node->next->prev = node->prev;

  node->next = node->prev = node;
}

// This is a helper function, feel free to keep it here or remove it if you do
// not have a need for it.
static void
_node_add(struct oo_node *left, struct oo_node *node)
{
  left->next->prev = node;
  node->next       = left->next;
  node->prev       = left;
  left->next       = node;
}

/**
 * Implementation of node_add_tail
 */
void
node_add_tail(struct oo_node *head, struct oo_node *node)
{
  // TODO: Add your code here..
  _node_add(head->prev, node);
}

/**
 * Implementation of node_add_head
 */
void
node_add_head(struct oo_node *head, struct oo_node *node)
{
  // TODO: Add your code here..
  _node_add(head, node);
}

/**
 *Implementation of *node_to_dbstr
 */
struct db_string *
node_to_dbstr(struct oo_node *node)
{
  return (struct db_string *)node;
}

/**
 *Implementation of *node_to_dbulong
 */
struct db_ulong *
node_to_dbulong(struct oo_node *node)
{
  return (struct db_ulong *)node;
}

/**
 * Implementation of db_add_record
 */
void
db_add_record(struct db *db, struct oo_node *node)
{
  // TODO: Add your code here...
  node_add_tail(&db->head, node);
  db->rcount++;
}

/**
 * Implementation of mk_db
 */
struct db *
mk_db(const char *name, int rtype)
{
  struct db *db = malloc(sizeof(struct db));
  if(!db) {
    perror("mk_db:");
    exit(EXIT_FAILURE);
  }

  // fail if name is not null terminated.
  memccpy(db->name, name, 0, 256);
  db->rcount  = 0;
  db->db_type = rtype;
  node_mk_node(&db->head);

  return db;
}

/*
 * Implementation of join_str_db
 * */
char *
join_str_db(struct db *db)
{
  struct oo_node *n;
  struct db_string *s;
  char *r = 0, *p = 0;
  size_t len = 0;

  for(n = db->head.next; n != &db->head; n = n->next) {
    s = node_to_dbstr(n);
    len += strlen(s->str) + 1;
  }

  r = malloc(len + 1);
  p = r;
  for(n = db->head.next; n != &db->head; n = n->next) {
    s = node_to_dbstr(n);
    strcpy(p, s->str);
    p += strlen(s->str);
    *p++ = ' ';
  }
  *p = 0;

  return r;
}

/*
 * Implementation of remove_all_matching
 */
int
searching_seek_and_destroy(struct db *db, unsigned long value)
{
  // TODO: Add your code here....
  int count = 0;
  struct oo_node *curr = db->head.next;
  struct oo_node *next;

  while (curr != &db->head) {
    next = curr->next;

    struct db_ulong *record = node_to_dbulong(curr);
    if (record->value == value) {
      node_rm_node(curr);
      db->rcount--;
      count++;
    }
    curr = next;
  }
  
  return count;
}
