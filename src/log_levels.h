#ifndef ATP_LOG_LEVELS_H_
#define ATP_LOG_LEVELS_H_

// Verbose level.  Use flag --v=N where N >= VLOG_LEVEL_* to see.
#define VLOG_LEVEL_ECLIENT  2
#define VLOG_LEVEL_EWRAPPER 1

// Log level for AsioEClientSocket
#define VLOG_LEVEL_ASIO_ECLIENT_SOCKET 10
#define VLOG_LEVEL_ASIO_ECLIENT_SOCKET_DEBUG 20

#endif //ATP_LOG_LEVELS_H_