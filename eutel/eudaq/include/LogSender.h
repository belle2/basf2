#ifndef EUDAQ_INCLUDED_LogSender
#define EUDAQ_INCLUDED_LogSender

#include <eutel/eudaq/TransportClient.h>
#include <eutel/eudaq/Serializer.h>
#include <eutel/eudaq/Status.h>
#include <string>

namespace eudaq {

  class LogMessage;

  class LogSender {
  public:
    LogSender();
    ~LogSender();
    void Connect(const std::string& type, const std::string& name, const std::string& server);
    void SendLogMessage(const LogMessage&, bool show = true);
    void SetLevel(int level) { m_level = level; }
    void SetLevel(const std::string& level) { SetLevel(Status::String2Level(level)); }
    void SetErrLevel(int level) { m_errlevel = level; }
    void SetErrLevel(const std::string& level) { SetErrLevel(Status::String2Level(level)); }
    bool IsLogged(const std::string& level) { return Status::String2Level(level) >= m_level; }
  private:
    std::string m_name;
    TransportClient* m_logclient;
    int m_level;
    int m_errlevel;
    bool m_shownotconnected;
  };

}

#endif // EUDAQ_INCLUDED_LogSender
