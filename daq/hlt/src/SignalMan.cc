/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/SignalMan.h>

using namespace Belle2;

SignalMan::SignalMan(void)
    : m_inPort(-1), m_outPort(-1)
{
}

SignalMan::SignalMan(const int inPort, const int outPort)
    : m_inPort(inPort), m_outPort(outPort)
{
}

SignalMan::SignalMan(const int inPort, const int outPort, std::string dest)
    : m_inPort(inPort), m_outPort(outPort)
{
  m_dest.clear();
  m_dest.push_back(dest);
}

SignalMan::SignalMan(const int inPort, const int outPort, std::vector<std::string> dest, const std::string mode)
    : m_inPort(inPort), m_outPort(outPort)
{
  m_dest.clear();
  m_dest = dest;
}

SignalMan::~SignalMan(void)
{
  if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
    delete m_inBuf;
    delete m_outBuf;
  }
}

EStatus SignalMan::init(void)
{
  char inBufName[] = "inBuffer";
  char outBufName[] = "outBuffer";

  m_inBuf = new RingBuffer(inBufName, 1024);
  m_outBuf = new RingBuffer(outBufName, 1024);

  return doCommunication();
}

EStatus SignalMan::doCommunication(void)
{
  // Forking off an event sender to send data
  m_pidEvtSender = fork();
  if (m_pidEvtSender == 0) {
    m_pidEvtReceiver = 1;
    m_sender = EvtSender(m_dest[0], m_outPort);
    if (m_sender.init(m_outBuf) == c_InitFailed) {
      B2ERROR("Could not initialize EvtSender.");
      return c_InitFailed;
    }

    B2INFO("EvtSender initialized");

    while (1) {
      EStatus status = m_sender.broadCasting();

      if (status == c_TermCalled) {
        B2INFO("EvtSender terminates...");
        return c_TermCalled;
      }

      usleep(100);
    }

    return c_Success;
  }

  // Forking off an event receiver to take data
  m_pidEvtReceiver = fork();
  if (m_pidEvtReceiver == 0) {
    m_pidEvtSender = 1;
    m_receiver = EvtReceiver(m_inPort);
    if (m_receiver.init(m_inBuf) == c_InitFailed) {
      B2ERROR("Could not initialize EvtReceiver.");
      return c_InitFailed;
    }
    B2INFO("EvtReceiver initialized");

    if (m_receiver.listen() != c_Success) {
      B2ERROR("Listening failed)");
      return c_FuncError;
    }

    EvtReceiver new_sock;
    m_receiver.accept(new_sock);
    std::string data;

    while (1) {
      new_sock >> data;
      if (data.size() > 0) {
        m_inBuf->insq((int*)(data.c_str()), data.size());
        if (data == "EOF") {
          B2INFO("Destroying EvtReceiver..");
          return c_TermCalled;
        }
      }

      usleep(100);
    }

    return c_Success;
  }

  return c_Success;
}

const void SignalMan::setPorts(const int inPort, const int outPort)
{
  m_inPort = inPort;
  m_outPort = outPort;
}

const void SignalMan::setDest(const std::string dest)
{
  m_dest.clear();
  m_dest.push_back(dest);
}

const void SignalMan::setDest(std::vector<std::string> dest)
{
  m_dest.clear();
  m_dest = dest;
}

RingBuffer* SignalMan::getInBuffer()
{
  return m_inBuf;
}

RingBuffer* SignalMan::getOutBuffer()
{
  return m_outBuf;
}

std::vector<std::string> SignalMan::dest()
{
  return m_dest;
}

int SignalMan::isMother()
{
  return (m_pidEvtSender && m_pidEvtReceiver);
}

int SignalMan::isEvtSender()
{
  return !(m_pidEvtSender);
}

int SignalMan::isEvtReceiver()
{
  return !(m_pidEvtReceiver);
}

std::string SignalMan::get(void)
{
  //if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
  while (1) {
    if (m_inBuf->numq() > 0) {
      char* tmp = new char[255];
      m_inBuf->remq((int*)tmp);
      std::string input(tmp);

      /*
         if (input == "EOF")
         break;
         */

      return input;
    }
  }
  //}
  /*
  if (m_outBuffer->ninsq () > 0) {
    return m_inBuf->get ();
  }
  else
    return "NOTHING";
    */
}

void SignalMan::put(const std::string data)
{
  m_outBuf->insq((int*)(data.c_str()), data.size());
  //m_outBuffer.push_back (data);
  //m_inBuffer->put (data);
  /*
  try {
    m_sender.connect ();
    m_sender << data;
  }
  catch (B2SocketException& e) {
    std::cout << "[\033[22;31mError\033[0m] " << e.description () << std::endl;
  }
  */
}

// unused?
void SignalMan::broadCasting(void)
{
  m_sender.connect();
}

// unused?
std::string SignalMan::listening(void)
{
  B2INFO("m_pidEvtSender = " << m_pidEvtSender);
  B2INFO("m_pidEvtReceiver = " << m_pidEvtReceiver);
  while (1) {
    if (m_inBuf->numq() > 0) {
      char* tmp = new char[255];
      m_inBuf->remq((int*)tmp);
      std::string input(tmp);

      return input;
    }
  }
  /*
    std::string data;
    bool flag = false;
    while (!flag) {
      EvtReceiver new_sock;
      m_receiver.accept (new_sock);

      try {
        while (1) {
          new_sock >> data;

          if (data != "")
            flag = true;

          return data;
        }
      }
      catch (B2SocketException&) {}
    }

    return data;
    */
}

const void SignalMan::Print(void)
{
  std::cout << "--SignalMan info" << std::endl;
  std::cout << "  [Incoming]" << std::endl;
  std::cout << "    port# = " << m_inPort << std::endl;
  std::cout << "  [Outgoing]" << std::endl;
  std::cout << "    port# = " << m_outPort << std::endl;
  //std::cout << "    dest  = " << m_dest << std::endl;
}
