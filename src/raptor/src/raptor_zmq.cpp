#include <Rcpp.h>

#include <stdio.h>
#include <zmq.hpp>
#include "raptor_zmq.h"


using namespace std;
using namespace Rcpp ;


SEXP raptor_zmq_connect(SEXP addr, SEXP socketType){
  std::string zmqAddr = Rcpp::as<std::string>(addr);
  std::string zmqSocketType = Rcpp::as<std::string>(socketType);

  zmq::context_t *context = new zmq::context_t(1);
  zmq::socket_t *socket;

  if (zmqSocketType == "ZMQ_REQ") {
    Rprintf("ZMQ_REP socket");
    socket = new zmq::socket_t(*context, ZMQ_REQ);
  } else if (zmqSocketType == "ZMQ_PUSH") {
    Rprintf("ZMQ_PUSH socket");
    socket = new zmq::socket_t(*context, ZMQ_PUSH);
  }

  socket->connect(zmqAddr.c_str());
  Rprintf(" connected @ %s", zmqAddr.c_str());

  XPtr<zmq::context_t> contextPtr(context);
  XPtr<zmq::socket_t> socketPtr(socket);

  List result = List::create(Named("context", contextPtr),
                             Named("socket", socketPtr),
                             Named("addr", zmqAddr));
  return result;
}

/// Send by memcpy.  This doesn't avoid the cost of memcpy but is safer
/// because the buffer is copied to the zmq message buffer.
inline static size_t send_copy(::zmq::socket_t& socket,
                               const std::string& input,
                               bool sendMore = false)
{
  size_t size = input.size();
  ::zmq::message_t frame(size);
  memcpy(frame.data(), input.data(), size);
  int more = (sendMore) ? ZMQ_SNDMORE : 0;
  socket.send(frame, more);
  return size;
}

inline static size_t send(::zmq::socket_t& socket,
                          const std::string& input)
{
  size_t size = input.size();
  ::zmq::message_t frame(size);
  memcpy(frame.data(), input.data(), size);
  socket.send(frame);
  return size;
}

SEXP raptor_zmq_send(SEXP handle, SEXP message){
  List handleList(handle);
  XPtr<zmq::socket_t> socket(handleList["socket"], R_NilValue, R_NilValue);

  std::string messageStr = Rcpp::as<std::string>(message);
  size_t sent = send(*socket, messageStr);
  Rprintf("Sent %d bytes, message = %s\n", sent, messageStr.c_str());

  return R_NilValue;
}


