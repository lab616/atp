#ifndef TEST_TEST_HARNESS_H_
#define TEST_TEST_HARNESS_H_

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <gflags/gflags.h>
#include <glog/logging.h>

#ifndef IB_USE_STD_STRING
#define IB_USE_STD_STRING
#endif

#include <Shared/Contract.h>
#include <Shared/EWrapper.h>



class TestHarness {

 public:
  enum EVENT {
    NEXT_VALID_ID = 1,
    TICK_PRICE,
    TICK_SIZE,
    TICK_GENERIC,
    TICK_OPTION_COMPUTATION,
    UPDATE_MKT_DEPTH,
    CONTRACT_DETAILS,
    CONTRACT_DETAILS_END // for option chain
  };

  TestHarness() : optionChain_(NULL) {

  }

  ~TestHarness() {}

  // Returns the count of an event.
  int getCount(EVENT event) {
    if (eventCount_.find(event) != eventCount_.end()) {
      return eventCount_[event];
    } else {
      return 0;
    }
  }

  // Returns true if the ticker id was seen.
  bool hasSeenTickerId(TickerId id) {
    return tickerIds_.find(id) != tickerIds_.end();
  }

  void setOptionChain(std::vector<Contract>* optionChain) {
    optionChain_ = optionChain;
  }

 protected:

  void incr(EVENT event, int count) {
    if (eventCount_.find(event) == eventCount_.end()) {
      eventCount_[event] = count;
    } else {
      eventCount_[event] += count;
    }
  }

  inline void incr(EVENT event) {
    incr(event, 1);
  }

  void seen(TickerId tickerId) {
    if (!hasSeenTickerId(tickerId)) {
      tickerIds_.insert(tickerId);
    }
  }
  
  std::map<EVENT, int> eventCount_;
  std::set<TickerId> tickerIds_;
  std::vector<Contract>* optionChain_;

};

#endif // TEST_TEST_HARNESS_H_
