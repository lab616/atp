#ifndef ATP_MARKETDATA_H_
#define ATP_MARKETDATA_H_


#include <sstream>
#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include "log_levels.h"
#include "utils.hpp"
#include "zmq/ZmqUtils.hpp"


namespace atp {

using namespace std;


template <typename V>
class MarketData
{
 public:
  MarketData(const string& topic,
             const boost::uint64_t timestamp,
             const string& key,
             const V& value) :
      topic_(topic), timestamp_(timestamp), key_(key), value_(value)
  {
  }

  /// Dispatches the market data to the outbound socket.
  // Frame sequence:
  // 1. topic
  // 2. timestamp as a long (no ISO format)
  // 3. key
  // 4. value
  // 5. latency
  inline size_t dispatch(::zmq::socket_t* socket)
  {
    size_t total = 0;
    if (socket != NULL) {

      std::ostringstream ts;
      ts << timestamp_;

      std::ostringstream vs;
      vs << value_;

      std::ostringstream latency;
      latency << (now_micros() - timestamp_);

      total += atp::zmq::send_copy(*socket, topic_, true);
      total += atp::zmq::send_copy(*socket, ts.str(), true);
      total += atp::zmq::send_copy(*socket, key_, true);
      total += atp::zmq::send_copy(*socket, vs.str(), true);
      total += atp::zmq::send_copy(*socket, latency.str(), false);
    }
    return total;
  }

 private:
  string topic_;
  boost::uint64_t timestamp_;
  string key_;
  V value_;
};


class MarketDataSubscriber
{
 public:

  MarketDataSubscriber(const string& endpoint,
                       const vector<string>& subscriptions,
                       ::zmq::context_t* context = NULL) :
      endpoint_(endpoint),
      subscriptions_(subscriptions),
      contextPtr_(context),
      ownContext_(context == NULL),
      offsetLatency_(false)
  {
    if (ownContext_) {
      contextPtr_ = new ::zmq::context_t(1);
    }
    socketPtr_ = new ::zmq::socket_t(*contextPtr_, ZMQ_SUB);
    socketPtr_->connect(endpoint_.c_str());
    // Set subscriptions
    for (vector<string>::iterator sub = subscriptions_.begin();
         sub != subscriptions_.end();
         ++sub) {
      socketPtr_->setsockopt(ZMQ_SUBSCRIBE,
                             sub->c_str(), sub->length());
      MARKET_DATA_SUBSCRIBER_LOGGER << "subscribed to topic = " << *sub;
    }
  }

  ~MarketDataSubscriber()
  {
    if (socketPtr_ != NULL) {
      delete socketPtr_;
    }
    if (ownContext_) {
      delete contextPtr_;
    }
  }

  void setOffsetLatency(bool value)
  {
    offsetLatency_ = value;
  }

  // The event loop.
  void processInbound()
  {
    using namespace boost::posix_time;
    long count = 0;
    time_duration latencyOffset;

    while (1) {

      string frame1; // topic
      string frame2; // timestamp
      string frame3; // key
      string frame4; // value
      string frame5; // latency

      int more = 1;
      if (more) more = atp::zmq::receive(*socketPtr_, &frame1);
      if (more) more = atp::zmq::receive(*socketPtr_, &frame2);
      if (more) more = atp::zmq::receive(*socketPtr_, &frame3);
      if (more) more = atp::zmq::receive(*socketPtr_, &frame4);
      if (more) more = atp::zmq::receive(*socketPtr_, &frame5);

      // Special handling of extra frames without blowing up.
      while (more) {
        string extra;
        more = atp::zmq::receive(*socketPtr_, &extra);
        LOG(ERROR) << "Unexpected frame: " << extra;
        if (more == 0) break;
      }
      boost::uint64_t ts;
      boost::uint64_t latency;

      istringstream s_ts(frame2);  s_ts >> ts;
      istringstream s_latency(frame5); s_latency >> latency;

      // Convert timestamp to posix time.
      ptime t = from_time_t(ts / 1000000LL);
      time_duration micros(0, 0, 0, ts % 1000000LL);
      t += micros;

      // Compute the latency from the initial timestamp to now,
      // accounting for network transport, parsing, etc.
      ptime now = microsec_clock::universal_time();
      time_duration total_latency = now - t;

      if (offsetLatency_) {
        if (++count == 1) {
          // compute the offset
          latencyOffset = total_latency;
          LOG(INFO) << "Using latency offset " << latencyOffset;
        } else {
          // compute the true latency with the offset
          total_latency -= latencyOffset;
        }
      }

      process(t, frame1, frame3, frame4, total_latency);
    }
  }

 protected:

  /// Process an incoming event.
  /// utc - timestamp in UTC
  /// topic - the topic of the event e.g. AAPL.STK
  /// key - the event name e.g. ASK
  /// value - string value. Subclasses perform proper conversion based on event.
  virtual bool process(const boost::posix_time::ptime& utc, const string& topic,
                       const string& key, const string& value,
                       const boost::posix_time::time_duration& latency)
  {
    // override this to remove the warning
    LOG(WARNING) << topic << ' ' << utc << ' ' << key << '=' << value;
    return true;
  }


 private:
  string endpoint_;
  vector<string> subscriptions_;
  ::zmq::context_t* contextPtr_;
  ::zmq::socket_t* socketPtr_;
  bool ownContext_;
  bool offsetLatency_;
};


} // namespace atp

#endif //ATP_MARKETDATA_H_
