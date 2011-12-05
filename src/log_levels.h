#ifndef ATP_LOG_LEVELS_H_
#define ATP_LOG_LEVELS_H_

#include <glog/logging.h>

// Verbose level.  Use flag --v=N where N >= VLOG_LEVEL_* to see.

#define IBAPI_APPLICATION_LOGGER VLOG(1)

#define ECLIENT_LOGGER VLOG(1)
#define EWRAPPER_LOGGER VLOG(1)

#define ASIO_ECLIENT_SOCKET_LOGGER VLOG(10)
#define ASIO_ECLIENT_SOCKET_DEBUG VLOG(30)

#define IBAPI_SOCKET_INITIATOR_LOGGER VLOG(5)
#define IBAPI_SOCKET_CONNECTOR_LOGGER VLOG(50)

#define IBAPI_SOCKET_CONNECTOR_STRATEGY_LOGGER VLOG(10)
#define IBAPI_ABSTRACT_SOCKET_CONNECTOR_LOGGER VLOG(20)

#define ZMQ_MEM_FREE_DEBUG VLOG(100)
#define ZMQ_SOCKET_DEBUG VLOG(100)
#define ZMQ_REACTOR_LOGGER VLOG(20)
#define ZMQ_PUBLISHER_LOGGER VLOG(20)

// ApiMessageBase
#define API_MESSAGE_BASE_LOGGER VLOG(40)
#define API_MESSAGE_STRATEGY_LOGGER VLOG(40)

// LogReader
#define LOG_READER_LOGGER VLOG(10)
#define LOG_READER_DEBUG VLOG(20)

// firehose
#define FIREHOSE_LOGGER VLOG(10)
#define FIREHOSE_DEBUG VLOG(20)

#endif //ATP_LOG_LEVELS_H_
