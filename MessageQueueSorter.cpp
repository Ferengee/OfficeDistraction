#include "MessageQueueSorter.h"

MessageQueueSorter::MessageQueueSorter()
{
  m_length = MAXMESSAGECOUNT;
  m_from=m_buf1;
  m_to=m_buf2;
  m_firstUnusedItemIndex=0;
  m_sorted = m_from;

 }

void MessageQueueSorter::init(MessageQueueItem* queue)
{
  m_original = queue;
  int i=0;
  for (i=0; i < m_length ; i++){
    m_from[i] = &m_original[i];
  }
  m_sorted = m_from;
  reorder();
 
}
  
/*
 * the first candidate is m_sorted[m_firstUnusedItemIndex]
 * while( candidate.isDestroyed() == false) m_firstUnusedItemIndex++
 */
bool MessageQueueSorter::getUnusedItem(MessageQueueItem** item)
{
  MessageQueueItem * candidate = m_sorted[m_firstUnusedItemIndex];
  while(!candidate->isDestroyed() && m_firstUnusedItemIndex < m_length - 1){
     m_firstUnusedItemIndex++;
     candidate = m_sorted[m_firstUnusedItemIndex];
  }
  *item = candidate;
  return candidate->isDestroyed();
}

MessageQueueItem ** MessageQueueSorter::reorder()
{
  /*
   * copy items from -> to
   * destroyed to back;
   */
  int i=0;
  int ta=0;
  int td = m_length;
  
  for (i=0; i < m_length; i++){
    MessageQueueItem * item = m_from[i];
    if (item->isDestroyed()){
      m_to[--td] = item;
      m_firstUnusedItemIndex=td;
    }else{
      m_to[ta++] = item;
    }
  }

  m_sorted = m_to;
  m_to = m_from;
  m_from = m_sorted;
  
  return m_sorted;
}
