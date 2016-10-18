#pragma once

#include <framework/pcore/zeromq/ZeroMQTxModule.h>
#include <framework/pcore/zeromq/ZeroMQRoutedMessage.h>
#include <deque>

namespace Belle2 {
  class ZeroMQTxInputModule : public ZeroMQTxModule {
  public:
    ZeroMQTxInputModule() :
      ZeroMQTxModule()
    {
      m_workers.reserve(40);
    }

    void event() override;
    void terminate() override;

  protected:
    virtual std::unique_ptr<ZeroMQRoutedMessage> readEvent();

  private:
    std::deque<unsigned int> m_nextWorker;
    std::vector<unsigned int> m_workers;

    void createSocket() override;
    unsigned int getNextWorker();
    void getWorkersReadyMessages(bool blocking);
  };
}