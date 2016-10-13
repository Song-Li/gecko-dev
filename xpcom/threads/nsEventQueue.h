/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsEventQueue_h__
#define nsEventQueue_h__

#include <stdlib.h>
#include "mozilla/CondVar.h"
#include "mozilla/Mutex.h"
#include "nsIRunnable.h"
#include "nsCOMPtr.h"
#include "mozilla/AlreadyAddRefed.h"

class nsThreadPool;

// A threadsafe FIFO event queue...
class nsEventQueue
{
public:
  typedef mozilla::MutexAutoLock MutexAutoLock;

  explicit nsEventQueue(mozilla::Mutex& aLock);
  ~nsEventQueue();

  // This method adds a new event to the pending event queue.  The queue holds
  // a strong reference to the event after this method returns.  This method
  // cannot fail.
  void PutEvent(nsIRunnable* aEvent, MutexAutoLock& aProofOfLock);
  void PutEvent(already_AddRefed<nsIRunnable>&& aEvent,
                MutexAutoLock& aProofOfLock);

  // This method gets an event from the event queue.  If mayWait is true, then
  // the method will block the calling thread until an event is available.  If
  // the event is null, then the method returns immediately indicating whether
  // or not an event is pending.  When the resulting event is non-null, the
  // caller is responsible for releasing the event object.  This method does
  // not alter the reference count of the resulting event.
  bool GetEvent(bool aMayWait, nsIRunnable** aEvent,
                MutexAutoLock& aProofOfLock);

  // This method returns true if there is a pending event.
  bool HasPendingEvent(MutexAutoLock& aProofOfLock)
  {
    return GetEvent(false, nullptr, aProofOfLock);
  }

  // This method returns the next pending event or null.
  bool GetPendingEvent(nsIRunnable** aRunnable, MutexAutoLock& aProofOfLock)
  {
    return GetEvent(false, aRunnable, aProofOfLock);
  }

  size_t Count(MutexAutoLock&);

private:
//SECLAB Thu 13 Oct 2016 02:55:25 PM EDT START

  long long int secCounter = 0;
  void GetRunNow() {
    if(IsEmpty()) return ;
    Page* head = mHead, *tail = mTail, *nextRun = NULL;
    uint16_t offset = -1;
    int first_end = 0;
    unsigned int minExpTime = 2147483648;// larger than the normal int
    printf("%d headheadhead\n", mHead->mEvents[mOffsetHead]->expTime);

    if(head != tail) first_end = EVENTS_PER_PAGE;
    else first_end = mOffsetTail;
    for(int i = mOffsetHead;i < first_end;++ i){
      if(head->mEvents[i]->expTime < minExpTime && head->mEvents[i]->expTime >= 0){
        nextRun = head;
        offset = i;
        minExpTime = head->mEvents[i]->expTime;
      }
    }

    if(head != tail) head = head->mNext;

    while(head != tail) {
      for(int i = 0;i < EVENTS_PER_PAGE;++ i)
        if(head->mEvents[i]->expTime < minExpTime && head->mEvents[i]->expTime >= 0){
          nextRun = head;
          offset = i;
          minExpTime = head->mEvents[i]->expTime;
        }
      head = head->mNext;
    }

    if(mHead != mTail) {
      for(int i = 0;i < mOffsetTail;++ i) {
        if(tail->mEvents[i]->expTime < minExpTime && tail->mEvents[i]->expTime >= 0){
          nextRun = tail;
          offset = i;
          minExpTime = head->mEvents[i]->expTime;
        }
      }
    }

    if(nextRun != mHead || offset != mOffsetHead) 
      printf("Error %lld %lld %d %d\n", mHead->mEvents[mOffsetHead]->expTime, nextRun->mEvents[offset]->expTime, offset, mOffsetHead);

    nsIRunnable* tmp = mHead->mEvents[mOffsetHead];
    mHead->mEvents[mOffsetHead] = nextRun->mEvents[offset];
    nextRun->mEvents[offset] = tmp;
  }
  //SECLAB Thu 13 Oct 2016 02:55:29 PM EDT END
  bool IsEmpty()
  {
    return !mHead || (mHead == mTail && mOffsetHead == mOffsetTail);
  }

  enum
  {
    EVENTS_PER_PAGE = 255
  };

  // Page objects are linked together to form a simple deque.

  struct Page
  {
    struct Page* mNext;
    nsIRunnable* mEvents[EVENTS_PER_PAGE];
  };

  static_assert((sizeof(Page) & (sizeof(Page) - 1)) == 0,
      "sizeof(Page) should be a power of two to avoid heap slop.");

  static Page* NewPage()
  {
    return static_cast<Page*>(moz_xcalloc(1, sizeof(Page)));
  }

  static void FreePage(Page* aPage)
  {
    free(aPage);
  }

  Page* mHead;
  Page* mTail;

  uint16_t mOffsetHead;  // offset into mHead where next item is removed
  uint16_t mOffsetTail;  // offset into mTail where next item is added
  mozilla::CondVar mEventsAvailable;

  // These methods are made available to nsThreadPool as a hack, since
  // nsThreadPool needs to have its threads sleep for fixed amounts of
  // time as well as being able to wake up all threads when thread
  // limits change.
  friend class nsThreadPool;
  void Wait(PRIntervalTime aInterval)
  {
    mEventsAvailable.Wait(aInterval);
  }
  void NotifyAll()
  {
    mEventsAvailable.NotifyAll();
  }
};

#endif  // nsEventQueue_h__
