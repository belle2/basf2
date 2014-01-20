#include <eutel/eudaq/LogSender.h>
#include <eutel/eudaq/LogMessage.h>
#include <eutel/eudaq/TransportFactory.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/BufferSerializer.h>

namespace eudaq {

  LogSender::LogSender() :
    m_logclient(0), m_errlevel(Status::LVL_DEBUG), m_shownotconnected(false) {}

  void LogSender::Connect(const std::string& type, const std::string& name, const std::string& server)
  {
    m_shownotconnected = true;
    delete m_logclient;
    m_name = type + " " + name;
    m_logclient = TransportFactory::CreateClient(server);

    std::string packet;
    if (!m_logclient->ReceivePacket(&packet, 1000000)) EUDAQ_THROW("No response from LogCollector server");
    size_t i0 = 0, i1 = packet.find(' ');
    if (i1 == std::string::npos) EUDAQ_THROW("Invalid response from LogCollector server");
    std::string part(packet, i0, i1);
    if (part != "OK") EUDAQ_THROW("Invalid response from LogCollector server: " + packet);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    if (i1 == std::string::npos) EUDAQ_THROW("Invalid response from LogCollector server");
    part = std::string(packet, i0, i1 - i0);
    if (part != "EUDAQ") EUDAQ_THROW("Invalid response from LogCollector server, part=" + part);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    if (i1 == std::string::npos) EUDAQ_THROW("Invalid response from LogCollector server");
    part = std::string(packet, i0, i1 - i0);
    if (part != "LOG") EUDAQ_THROW("Invalid response from LogCollector server, part=" + part);
    i0 = i1 + 1;
    i1 = packet.find(' ', i0);
    part = std::string(packet, i0, i1 - i0);
    if (part != "LogCollector") EUDAQ_THROW("Invalid response from LogCollector server, part=" + part);

    m_logclient->SendPacket("OK EUDAQ LOG " + m_name);
    packet = "";
    if (!m_logclient->ReceivePacket(&packet, 1000000)) EUDAQ_THROW("No response from LogCollector server");
    i1 = packet.find(' ');
    if (std::string(packet, 0, i1) != "OK") EUDAQ_THROW("Connection refused by LogCollector server: " + packet);
  }

  void LogSender::SendLogMessage(const LogMessage& msg, bool show)
  {
    //std::cout << "Sending: " << msg << std::endl;
    if (!m_logclient) {
      if (m_shownotconnected) std::cerr << "### Logger not connected ###\n";
    } else {
      BufferSerializer ser;
      msg.Serialize(ser);
      m_logclient->SendPacket(ser);
    }

    if (msg.GetLevel() >= m_level && show) {
      if (msg.GetLevel() >= m_errlevel) {
        if (m_name != "") std::cerr << "[" << m_name << "] ";
        std::cerr << msg << std::endl;
      } else {
        if (m_name != "") std::cout << "[" << m_name << "] ";
        std::cout << msg << std::endl;
      }
    }
  }

  LogSender::~LogSender()
  {
    delete m_logclient;
  }

}
