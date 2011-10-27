
#include <algorithm>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include "ib/Application.hpp"
#include "ib/GenericTickRequest.hpp"
#include "ib/AsioEClientSocket.hpp"
#include "ib/EWrapperFactory.hpp"
#include "ib/SessionSetting.hpp"
#include "ib/SocketConnector.hpp"
#include "ib/SocketConnectorImpl.hpp"
#include "ib/SocketInitiator.hpp"

#include "ib/TestHarness.hpp"
#include "ib/ticker_id.hpp"
#include "zmq/ZmqUtils.hpp"


using namespace IBAPI;

enum Event {
  // Strategy
  ON_CONNECT,
  ON_ERROR,
  ON_DISCONNECT,
  ON_TIMEOUT,

  // Application
  ON_LOGON,
  ON_LOGOUT,
  TO_ADMIN,
  TO_APP,
  FROM_ADMIN,
  FROM_APP
};

class TestApplication : public IBAPI::ApplicationBase,
                        public TestHarnessBase<Event>
{
 public:

  TestApplication() {}
  ~TestApplication() {}

  void onLogon(const SessionID& sessionId)
  {
    incr(ON_LOGON);
  }

  void onLogout(const SessionID& sessionId)
  {
    incr(ON_LOGOUT);
  }

};

class TestStrategy :
    public IBAPI::StrategyBase,
    public TestHarnessBase<Event>
{
 public :
  TestStrategy() {}
  ~TestStrategy() { }

  void onConnect(SocketConnector& sc, int clientId)
  {
    incr(ON_CONNECT);
  }
};

using ib::internal::EWrapperFactory;

class TestSocketConnectorImpl : public ib::internal::SocketConnectorImpl
{
 public:
  TestSocketConnectorImpl(Application& app, int timeout,
                          const string& responderAddress) :
      SocketConnectorImpl(app, timeout, responderAddress),
      publishContext_(1)
  {}

 protected:

  bool readSocketAndProcess(zmq::socket_t& socket)
  {
    int more = atp::zmq::receive(socket, &msg);
    LOG(INFO) << "Received " << msg << std::endl;

    try {

      zmq::socket_t* sink = getSink();

      // This is run in a thread separate from the AsioEClientSocket's
      // block() thread.  That thread has access to the publish socket.
      // We need to make sure that no other thread has access to the socket.

      EXPECT_EQ(NULL, sink);

      // just echo back to the client
      size_t sent = atp::zmq::send_zero_copy(socket, msg);

      return sent > 0;

    } catch (zmq::error_t e) {
      LOG(ERROR) << "Exception " << e.what() << std::endl;
      return false;
    }
  }

  zmq::socket_t* createPublishSocket()
  {
    std::string endpoint = "tcp://127.0.0.1:5555";

    LOG(INFO) << "Creating publish socket @ " << endpoint << std::endl;
    zmq::socket_t* socket = new zmq::socket_t(publishContext_, ZMQ_PUB);
    socket->bind(endpoint.c_str());
    return socket;
  }

 private:
  std::string msg;
  zmq::context_t publishContext_;
};


TEST(SocketConnectorTest, SocketConnectorImplConnectionTest)
{
  TestApplication app;
  TestStrategy strategy;

  const string& bindAddr = "ipc://SocketConnectorImplConnectionTest";

  TestSocketConnectorImpl socketConnector(app, 10, bindAddr);

  int clientId = 1;
  int status = socketConnector.connect("127.0.0.1", 4001, clientId,
                                       &strategy);

  EXPECT_EQ(clientId, status); // Expected, actual
  EXPECT_EQ(1, strategy.getCount(ON_CONNECT));

  LOG(INFO) << "Checked on_connect = " << strategy.getCount(ON_CONNECT)
            << std::endl;

  app.waitForFirstOccurrence(ON_LOGON, 10);

  EXPECT_EQ(app.getCount(ON_LOGON), 1);

  LOG(INFO) << "Test complete." << std::endl;
  socketConnector.stop();
}


/// Basic test for instantiation and destroy
TEST(SocketConnectorTest, SendMessageTest)
{
  TestApplication app;
  TestStrategy strategy;

  const string& bindAddr = "ipc://SocketConnectorImplTest";

  TestSocketConnectorImpl socketConnector(app, 10, bindAddr);

  LOG(INFO) << "Starting client."  << std::endl;

  // Client
  zmq::context_t context(1);
  zmq::socket_t client(context, ZMQ_REQ);
  client.connect(bindAddr.c_str());

  LOG(INFO) << "Client connected."  << std::endl;

  int status = socketConnector.connect("127.0.0.1", 4001, 0,
                                       &strategy);
  EXPECT_EQ(0, status); // Expected, actual
  EXPECT_EQ(1, strategy.getCount(ON_CONNECT));

  size_t messages = 10;
  for (unsigned int i = 0; i < messages; ++i) {
    std::ostringstream oss;
    oss << "Message-" << i;

    std::string message(oss.str());
    bool exception = false;
    try {
      VLOG(40) << "sending " << message << std::endl;

      atp::zmq::send_zero_copy(client, message);

      std::string reply;
      atp::zmq::receive(client, &reply);

      ASSERT_EQ(message, reply);
    } catch (zmq::error_t e) {
      LOG(ERROR) << "Exception: " << e.what() << std::endl;
      exception = true;
    }
    ASSERT_FALSE(exception);
  }

  LOG(INFO) << "Finishing." << std::endl;
  socketConnector.stop();
}
