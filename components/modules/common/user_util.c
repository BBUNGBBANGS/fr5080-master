#include "user_utils.h"

/*---------------------------------------------------------------------------
 *            insert_tail_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Insert an entry at the tail of the list specified by head.
 *
 * Return:    void
 */
void insert_tail_list(struct list_entry_t* head, struct list_entry_t* entry)
{
  entry->Flink = head;
  entry->Blink = head->Blink;
  head->Blink->Flink = entry;
  head->Blink = entry;
}

/*---------------------------------------------------------------------------
 *            insert_head_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Insert an entry at the head of the list specified by head.
 *
 * Return:    void
 */
void insert_head_list(struct list_entry_t* head, struct list_entry_t* entry)
{
  entry->Flink = head->Flink;
  entry->Blink = head;
  head->Flink->Blink = entry;
  head->Flink = entry;

}

/*---------------------------------------------------------------------------
 *            remove_head_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Remove the first entry on the list specified by head.
 *
 * Return:    void
 */
struct list_entry_t* remove_head_list(struct list_entry_t* head)
{
  struct list_entry_t* first;

  first = head->Flink;
  first->Flink->Blink = head;
  head->Flink = first->Flink;
  return(first);

}

/*---------------------------------------------------------------------------
 *           remove_entry_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Remove the given entry from the list.
 *
 * Return:    void
 *
 */
void remove_entry_list(struct list_entry_t* entry)
{
  entry->Blink->Flink = entry->Flink;
  entry->Flink->Blink = entry->Blink;
  init_list_entry(entry);

}

/*---------------------------------------------------------------------------
 *            is_node_on_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Determine if an entry is on the list specified by head.
 *
 * Return:    TRUE - the entry is on the list.
 *            FALSE - the entry is not on the list.
 */
bool is_node_on_list(struct list_entry_t* head, struct list_entry_t* node)
{
  struct list_entry_t* tmpNode;

  tmpNode = get_head_list(head);

  while (tmpNode != head)
  {
    if (tmpNode == node)
      return(true);

    tmpNode = tmpNode->Flink;
  }
  return(false);

}

/*---------------------------------------------------------------------------
 *            move_list()
 *---------------------------------------------------------------------------
 *
 * Synopsis:  Moves a list to a new list head (which need not be initialized)
 *            The source list is left empty. 
 *
 * Return:    none
 */
void move_list(struct list_entry_t* dest, struct list_entry_t* src)
{

    if (is_list_empty(src)) {
        init_list_head(dest);

    } else {
        dest->Flink = src->Flink;
        dest->Blink = src->Blink;
        src->Flink->Blink = dest;
        src->Blink->Flink = dest;
        init_list_head(src);
    }
}


