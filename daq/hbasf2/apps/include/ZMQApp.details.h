/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/hbasf2/apps/ZMQApp.h>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Belle2 {

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::initFromConsole(const std::string& description, int argc, char* argv[])
  {

    po::options_description desc(description);
    std::string connection_file;
    int debugLevel(0);
    desc.add_options()
    ("connection-file", boost::program_options::value<std::string>(&connection_file),
     "if given print the connection information for input/output and monitoring socket to the given filename "
     "in json format")
    ("debug", boost::program_options::value<int>(&debugLevel), "Enable debug logging");
    addOptions(desc);

    po::positional_options_description p;

    po::variables_map vm;
    try {
      po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    } catch (std::exception& e) {
      B2FATAL(e.what());
    }

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      exit(1);
    }

    try {
      po::notify(vm);
    } catch (std::exception& e) {
      B2FATAL(e.what());
    }

    if (debugLevel > 0) {
      auto& logging = LogSystem::Instance();
      logging.getLogConfig()->setLogLevel(LogConfig::c_Debug);
      logging.getLogConfig()->setDebugLevel(debugLevel);
      B2DEBUG(1, "Enabled debug logging");
    }

    initialize();

    if (not connection_file.empty()) {
      B2DEBUG(1, "Write connection file" << LogVar("connection_file", connection_file));
      nlohmann::json json;
      try {
        json["input"] = m_input->getEndPoint();
      } catch (zmq::error_t& e) {
        B2WARNING(e.what());
      }
      try {
        json["output"] = m_output->getEndPoint();
      } catch (zmq::error_t& e) {
        B2WARNING(e.what());
      }
      std::ofstream connections(connection_file, std::ofstream::trunc);
      if (!connections) {
        B2FATAL("Cannot write connection file" << LogVar("connection_file", connection_file));
      }
      connections << std::setw(4) << json << std::endl;
    }
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::main()
  {
    resetTimer();

    while (not terminated()) {
      if (not m_output->isReady()) {
        // if the output is not ready, we can not sent anything. So lets just poll on output
        // and monitoring until it becomes ready
        m_monitor->log("output_state", "not_ready");
        pollEvent(false);
      } else {
        // if it is ready, we can also include the input socket as long as output stays ready
        m_monitor->log("output_state", "ready");
        pollEvent(true);
      }
    }
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::pollEvent(bool pollOnInput)
  {
    auto reactToOutput = [this]() {
      // Get all messages from output socket
      while (ZMQConnection::hasMessage(m_output.get()) and not terminated()) {
        handleOutput();
        if (m_monitorHasPriority) {
          break;
        }
      }
    };

    auto reactToMonitor = [this]() {
      // Get all messages from monitoring socket
      while (ZMQConnection::hasMessage(m_monitor.get()) and not terminated()) {
        handleMonitoring();
      }
    };

    auto reactToInput = [this]() {
      // Get all messages from input as long output is ready
      while (ZMQConnection::hasMessage(m_input.get()) and m_output->isReady() and not terminated()) {
        handleInput();
        if (m_monitorHasPriority) {
          break;
        }
      }
    };

    m_monitor->logTime("waiting_since");

    if (pollOnInput) {
      ZMQConnection::poll({{m_output.get(), reactToOutput}, {m_monitor.get(), reactToMonitor}, {m_input.get(), reactToInput}},
      m_remainingTime);
    } else {
      ZMQConnection::poll({{m_output.get(), reactToOutput}, {m_monitor.get(), reactToMonitor}}, m_remainingTime);
    }

    if (checkTimer() and not terminated()) {
      B2ASSERT("There is no timeout set, but we still call the timeout() function? A bug!", m_timeout != 0);
      m_monitor->increment("timeouts");
      handleTimeout();
      resetTimer();
    } else {
      updateTimer();
    }
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::initialize()
  {
    m_parent.reset(new ZMQParent);
    m_monitor.reset(new ZMQSimpleConnection(m_monitoringAddress, m_parent));
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::addOptions(po::options_description& desc)
  {
    desc.add_options()("help,h", "Print this help message")("monitor", po::value<std::string>(&m_monitoringAddress)->required(),
                                                            "where to listen for monitoring");
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::handleExternalSignal(EMessageTypes)
  {
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::handleInput()
  {
    m_input->handleIncomingData();
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::handleOutput()
  {
    m_output->handleIncomingData();
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::handleTimeout()
  {
  }

  template <class AInputConnection, class AOutputConnection>
  bool ZMQStandardApp<AInputConnection, AOutputConnection>::terminated() const
  {
    return not m_mainLoop.isRunning() or m_terminate;
  }

  template <class AInputConnection, class AOutputConnection>
  bool ZMQStandardApp<AInputConnection, AOutputConnection>::checkTimer()
  {
    updateTimer();
    return m_remainingTime == 0;
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::updateTimer()
  {
    // if there is no timeout, we should never update the remaining time
    if (m_timeout == 0) {
      m_remainingTime = -1;
      return;
    }

    auto currentTime = std::chrono::system_clock::now();
    auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_start);
    m_remainingTime = m_timeout * 1000 - timeDifference.count();
    if (m_remainingTime < 0) {
      m_remainingTime = 0;
    }
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::resetTimer()
  {
    // if there is no timeout, we should never set the remaining time
    if (m_timeout == 0) {
      m_remainingTime = -1;
      return;
    }

    m_start = std::chrono::system_clock::now();
    m_remainingTime = m_timeout * 1000;
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::handleMonitoring()
  {
    auto monitoringMessage = m_monitor->handleIncomingData();

    if (monitoringMessage->isMessage(EMessageTypes::c_newRunMessage)) {
      handleExternalSignal(EMessageTypes::c_newRunMessage);
      return;
    } else if (monitoringMessage->isMessage(EMessageTypes::c_lastEventMessage)) {
      handleExternalSignal(EMessageTypes::c_lastEventMessage);
      return;
    } else if (monitoringMessage->isMessage(EMessageTypes::c_terminateMessage)) {
      handleExternalSignal(EMessageTypes::c_terminateMessage);
      return;
    }

    std::stringstream buffer;
    fillMonitoringJSON(buffer);

    auto message = ZMQMessageFactory::createMessage(monitoringMessage->getIdentity(),
                                                    EMessageTypes::c_confirmMessage, buffer.str());
    m_monitor->handleEvent(std::move(message));
  }

  template <class AInputConnection, class AOutputConnection>
  void ZMQStandardApp<AInputConnection, AOutputConnection>::fillMonitoringJSON(std::stringstream& buffer) const
  {
    buffer << "{" << std::endl;
    buffer << "\"monitor\": " << m_monitor->getMonitoringJSON() << "," << std::endl;
    buffer << "\"input\": " << m_input->getMonitoringJSON() << "," << std::endl;
    buffer << "\"output\": " << m_output->getMonitoringJSON() << std::endl;
    buffer << "}" << std::endl;
  }

} // namespace Belle2
