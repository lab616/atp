#ifndef ATP_COMMON_OHLC_H_
#define ATP_COMMON_OHLC_H_

#include "common.hpp"
#include "common/time_series.hpp"
#include "common/moving_window.hpp"
#include "common/moving_window_samplers.hpp"
#include "proto/ostream.hpp"


using boost::posix_time::time_duration;
using atp::common::time_series;


namespace atp {
namespace common {
namespace callback {

using atp::common::microsecond_t;
using atp::common::Id;


template <typename V>
struct ohlc_post_process
{
  virtual ~ohlc_post_process() {}

  virtual void operator()(const size_t count,
                          const Id& id,
                          const time_series<microsecond_t, V>& open,
                          const time_series<microsecond_t, V>& high,
                          const time_series<microsecond_t, V>& low,
                          const time_series<microsecond_t, V>& close) = 0;
};


template <typename V>
struct ohlc_post_process_noop : public ohlc_post_process<V>
{
  virtual void operator()(const size_t count,
                          const Id& id,
                          const time_series<microsecond_t, V>& open,
                          const time_series<microsecond_t, V>& high,
                          const time_series<microsecond_t, V>& low,
                          const time_series<microsecond_t, V>& close)
  {
    UNUSED(count); UNUSED(id);
    UNUSED(open); UNUSED(high); UNUSED(low); UNUSED(close);
  }
};
} // callback



using namespace atp::common::sampler;

template <typename V>
class ohlc
{

 public:

  typedef moving_window<V, sampler::open<V> > mw_open;
  typedef moving_window<V, sampler::close<V> > mw_close;
  typedef moving_window<V, sampler::max<V> > mw_high;
  typedef moving_window<V, sampler::min<V> > mw_low;

  ohlc(time_duration h, time_duration s, V initial) :
      open_(h, s, initial)
      , close_(h, s, initial)
      , high_(h, s, initial)
      , low_(h, s, initial)
      , post_(NULL)
  {
  }

  void set(callback::ohlc_post_process<V>& pp)
  {
    post_ = &pp;
  }

  void set(const Id& id)
  {
    id_ = id;
    // set the id of the dependent moving_windows
    Id sub = id;
    sub.add_label(id_.label(0) + "$open");
    open_.set(sub);
    sub.add_label(id_.label(0) + "$high");
    high_.set(sub);
    sub.add_label(id_.label(0) + "$low");
    low_.set(sub);
    sub.add_label(id_.label(0) + "$close");
    close_.set(sub);
  }

  size_t operator()(const microsecond_t& timestamp, const V& value)
  {
    return on(timestamp, value);
  }

  size_t size()
  {
    return open_.size(); // any of the components
  }

  size_t on(const microsecond_t& timestamp, const V& value)
  {
    size_t open = open_(timestamp, value);
    close_(timestamp, value);
    high_(timestamp, value);
    low_(timestamp, value);

    // assume the sizes are all the same.
    if (post_ != NULL) {
      (*post_)(open, id_, open_, high_, low_, close_);
    }

    return open;
  }

  const time_series<microsecond_t, V>& open() const
  {
    return open_;
  }

  const time_series<microsecond_t, V>& close() const
  {
    return close_;
  }

  const time_series<microsecond_t, V>& high() const
  {
    return high_;
  }

  const time_series<microsecond_t, V>& low() const
  {
    return low_;
  }

  mw_open& mutable_open()
  {
    return open_;
  }

  mw_high& mutable_high()
  {
    return high_;
  }

  mw_low& mutable_low()
  {
    return low_;
  }

  mw_close& mutable_close()
  {
    return close_;
  }



  const Id& id() const
  {
    return id_;
  }

 private:

  mw_open open_;
  mw_close close_;
  mw_high high_;
  mw_low low_;
  callback::ohlc_post_process<V>* post_;
  Id id_;
};


} // common
} // atp


#endif //ATP_COMMON_OHLC_H_
