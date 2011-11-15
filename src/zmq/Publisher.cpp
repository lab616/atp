
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <glog/logging.h>
#include <zmq.hpp>
#include <stdlib.h>

#include "utils.hpp"
#include "common.hpp"
#include "zmq/Publisher.hpp"


namespace atp {
namespace zmq {

Publisher::Publisher(const string& addr,
                     const string& publishAddr,
                     ::zmq::context_t* context) :
    addr_(addr),
    publishAddr_(publishAddr),
    context_(context),
    ready_(false)
{
  // start thread
  thread_ = boost::shared_ptr<boost::thread>(new boost::thread(
      boost::bind(&Publisher::process, this)));

  // Wait here for the connection to be ready.
  boost::unique_lock<boost::mutex> lock(mutex_);
  while (!ready_) {
    isReady_.wait(lock);
  }

  ZMQ_PUBLISHER_LOGGER << "Publisher is ready.";
}

Publisher::~Publisher()
{
}

const std::string& Publisher::addr()
{
  return addr_;
}

const std::string& Publisher::publishAddr()
{
  return publishAddr_;
}

void Publisher::block()
{
  thread_->join();
}

void Publisher::process()
{
  bool localContext = false;
  if (context_ == NULL) {
    // Create own context
    context_ = new ::zmq::context_t(1);
    localContext = true;
    ZMQ_PUBLISHER_LOGGER << "Created local context.";
  }

  ::zmq::socket_t inbound(*context_, ZMQ_PULL);

  try {
    inbound.bind(addr_.c_str());
  } catch (::zmq::error_t e) {
    LOG(FATAL) << "Cannot bind inbound at " << addr_ << ":"
               << e.what();
  }
  // start publish socket
  ::zmq::socket_t publish(*context_, ZMQ_PUB);

  try {
    publish.bind(publishAddr_.c_str());
  } catch (::zmq::error_t e) {
    LOG(FATAL) << "Cannot bind publish at " << publishAddr_ << ":"
               << e.what();
  }

  ZMQ_PUBLISHER_LOGGER
      << "Inbound @ " << addr_
      << ", Publish @ " << publishAddr_;

  {
    boost::lock_guard<boost::mutex> lock(mutex_);
    ready_ = true;
  }
  isReady_.notify_all();

  bool stop = false;
  while (!stop) {
    while (!stop) {
      ::zmq::message_t message;
      int64_t more;
      size_t more_size = sizeof(more);
      //  Process all parts of the message
      try {

        inbound.recv(&message);
        inbound.getsockopt( ZMQ_RCVMORE, &more, &more_size);

        ZMQ_PUBLISHER_LOGGER << "Published " << message.size() << " bytes: "
                             << static_cast<char*>(message.data());

        publish.send(message, more? ZMQ_SNDMORE: 0);

      } catch (::zmq::error_t e) {
        ZMQ_PUBLISHER_LOGGER << "Stopping on exception: " << e.what();
        stop = true;
      }

      if (!more)
        break;      //  Last message part
    }
  }

  if (localContext) delete context_;
  LOG(ERROR) << "Publisher thread stopped.";
}


} // namespace zmq
} // namespace atp