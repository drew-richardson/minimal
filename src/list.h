/* SPDX-License-Identifier: GPL-2.0 */
// Derived from https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/linux/list.h?h=v4.14

#if !defined(LINUX_LIST_H)
#define LINUX_LIST_H

#include <stddef.h>

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#define container_of_const(ptr, type, member) ((type *)((const char *)(ptr) - offsetof(type, member)))

struct list_head {
  struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LINUX_LIST_HEAD(name) \
  struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
  list->next = list;
  list->prev = list;
}

static inline void __list_add(struct list_head *new_lh, struct list_head *prev, struct list_head *next)
{
  next->prev = new_lh;
  new_lh->next = next;
  new_lh->prev = prev;
  prev->next = new_lh;
}

static inline void list_add(struct list_head *new_lh, struct list_head *head)
{
  __list_add(new_lh, head, head->next);
}

static inline void list_add_tail(struct list_head *new_lh, struct list_head *head)
{
  __list_add(new_lh, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
  next->prev = prev;
  prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
  __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
  __list_del_entry(entry);
  entry->next = NULL;
  entry->prev = NULL;
}

static inline void list_replace(struct list_head *old, struct list_head *new_lh)
{
  new_lh->next = old->next;
  new_lh->next->prev = new_lh;
  new_lh->prev = old->prev;
  new_lh->prev->next = new_lh;
}

static inline void list_replace_init(struct list_head *old, struct list_head *new_lh)
{
  list_replace(old, new_lh);
  INIT_LIST_HEAD(old);
}

static inline void list_del_init(struct list_head *entry)
{
  __list_del_entry(entry);
  INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
  __list_del_entry(list);
  list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
  __list_del_entry(list);
  list_add_tail(list, head);
}

static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
  return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
  return head->next == head;
}

static inline int list_empty_careful(const struct list_head *head)
{
  struct list_head *next = head->next;
  return (next == head) && (next == head->prev);
}

static inline void list_rotate_left(struct list_head *head)
{
  struct list_head *first;
  if (!list_empty(head)) {
    first = head->next;
    list_move_tail(first, head);
  }
}

static inline int list_is_singular(const struct list_head *head)
{
  return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry)
{
  struct list_head *new_first = entry->next;
  list->next = head->next;
  list->next->prev = list;
  list->prev = entry;
  entry->next = list;
  head->next = new_first;
  new_first->prev = head;
}

static inline void list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry)
{
  if (list_empty(head))
    return;
  if (list_is_singular(head) && (head->next != entry && head != entry))
    return;
  if (entry == head)
    INIT_LIST_HEAD(list);
  else
    __list_cut_position(list, head, entry);
}

static inline void __list_splice(const struct list_head *list, struct list_head *prev, struct list_head *next)
{
  struct list_head *first = list->next;
  struct list_head *last = list->prev;
  first->prev = prev;
  prev->next = first;
  last->next = next;
  next->prev = last;
}

static inline void list_splice(const struct list_head *list, struct list_head *head)
{
  if (!list_empty(list))
    __list_splice(list, head, head->next);
}

static inline void list_splice_tail(struct list_head *list, struct list_head *head)
{
  if (!list_empty(list))
    __list_splice(list, head->prev, head);
}

static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
  if (!list_empty(list)) {
    __list_splice(list, head, head->next);
    INIT_LIST_HEAD(list);
  }
}

static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{
  if (!list_empty(list)) {
    __list_splice(list, head->prev, head);
    INIT_LIST_HEAD(list);
  }
}

#define list_entry(ptr, type, member) \
  container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
  list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
  list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, type, member) \
  list_entry((pos)->member.next, type, member)

#define list_prev_entry(pos, type, member) \
  list_entry((pos)->member.prev, type, member)

#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); \
       pos = n, n = pos->next)

#define list_for_each_prev_safe(pos, n, head) \
  for (pos = (head)->prev, n = pos->prev; \
       pos != (head); \
       pos = n, n = pos->prev)

#define list_for_each_entry(pos, head, type, member) \
  for (pos = list_first_entry(head, type, member); \
       &pos->member != (head); \
       pos = list_next_entry(pos, type, member))

#define list_for_each_entry_reverse(pos, head, type, member) \
  for (pos = list_last_entry(head, type, member); \
       &pos->member != (head); \
       pos = list_prev_entry(pos, type, member))

#define list_prepare_entry(pos, head, type, member) \
  ((pos) ? : list_entry(head, type, member))

#define list_for_each_entry_continue(pos, head, type, member) \
  for (pos = list_next_entry(pos, type, member); \
       &pos->member != (head); \
       pos = list_next_entry(pos, type, member))

#define list_for_each_entry_continue_reverse(pos, head, type, member) \
  for (pos = list_prev_entry(pos, type, member); \
       &pos->member != (head); \
       pos = list_prev_entry(pos, type, member))

#define list_for_each_entry_from(pos, head, type, member) \
  for (; &pos->member != (head); \
       pos = list_next_entry(pos, type, member))

#define list_for_each_entry_from_reverse(pos, head, type, member) \
  for (; &pos->member != (head); \
       pos = list_prev_entry(pos, type, member))

#define list_for_each_entry_safe(pos, n, head, type, member) \
  for (pos = list_first_entry(head, type, member), \
	 n = list_next_entry(pos, type, member); \
       &pos->member != (head); \
       pos = n, n = list_next_entry(n, type, member))

#define list_for_each_entry_safe_continue(pos, n, head, type, member) \
  for (pos = list_next_entry(pos, type, member), \
	 n = list_next_entry(pos, type, member); \
       &pos->member != (head); \
       pos = n, n = list_next_entry(n, type, member))

#define list_for_each_entry_safe_from(pos, n, head, type, member) \
  for (n = list_next_entry(pos, type, member); \
       &pos->member != (head); \
       pos = n, n = list_next_entry(n, type, member))

#define list_for_each_entry_safe_reverse(pos, n, head, type, member) \
  for (pos = list_last_entry(head, type, member), \
	 n = list_prev_entry(pos, type, member); \
       &pos->member != (head); \
       pos = n, n = list_prev_entry(n, type, member))

#define list_safe_reset_next(pos, type, n, member) \
  n = list_next_entry(pos, type, member)

#endif // LINUX_LIST_H
