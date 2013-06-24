#ifndef MessageQueueSorter_h
#define MessageQueueSorter_h

#include "MessageQueueItem.h"


class MessageQueueSorter
{
public:
  MessageQueueSorter();
  void init(MessageQueueItem * queue);

  /*
   * Sets item to point to the next available item
   * Returns true on succes
   * 
   */
  bool getUnusedItem(MessageQueueItem ** item);
  
  /*
   * moves inactive items to the back
   * must be called after destroy() on any of the MessageQueueItems in this queue
   * 
   */
  MessageQueueItem ** reorder();
  
  MessageQueueItem ** getSortedQueue() {return m_sorted;};
private:
  int m_length;
  MessageQueueItem * m_original;
  MessageQueueItem * m_buf1[MAXMESSAGECOUNT]; //Sorted list with items removed
  MessageQueueItem * m_buf2[MAXMESSAGECOUNT]; //List to transfer active items into front to back and inactive items back to front
  MessageQueueItem ** m_sorted; //points to the current state
  MessageQueueItem ** m_from; //points to the current state
  MessageQueueItem ** m_to; //points to the current state
 
  int m_firstUnusedItemIndex;
  
};
#endif