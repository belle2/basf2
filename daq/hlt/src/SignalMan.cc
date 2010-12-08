/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <signal.h>

#include "framework/dcore/SignalMan.h"
#include "framework/dcore/styledef.h"
#include <framework/logging/Logger.h>

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
  /*
  B2INFO ("Killing EvtSender " << m_pidEvtSender);
  kill (m_pidEvtSender, -9);
  B2INFO ("Killing EvtReceiver " << m_pidEvtReceiver);
  kill (m_pidEvtReceiver, -9);
  */

  if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
    delete m_inBuf;
    delete m_outBuf;
  }
}

int SignalMan::init(void)
{
  char inBufName[] = "inBuffer";
  char outBufName[] = "outBuffer";

  m_inBuf = new RingBuffer(inBufName, 1024);
  m_outBuf = new RingBuffer(outBufName, 1024);
  //m_inBuf = new RingBuffer ("inBuffer", 1024);
  //m_outBuf = new RingBuffer ("outBuffer", 1024);

  // Forking off an event sender to send data
  m_pidEvtSender = fork();
  if (m_pidEvtSender == 0) {
    m_sender = EvtSender(m_dest[0], m_outPort);
    m_sender.init(m_outBuf);
    B2INFO("EvtSender initialized");

    while (1) {
      //if (m_outBuf->ninsq () > 0)
      m_sender.broadCasting();
      usleep(100);
    }

    return 1;
  }

  // Forking off an event receiver to take data
  m_pidEvtReceiver = fork();
  if (m_pidEvtReceiver == 0) {
    m_receiver = EvtReceiver(m_inPort);
    m_receiver.init(m_inBuf);
    B2INFO("EvtReceiver initialized");

    m_receiver.listen();

    EvtReceiver new_sock;
    m_receiver.accept(new_sock);
    std::string data;

    while (1) {
      new_sock >> data;
      if (data.size() > 0) {
        m_inBuf->insq((int*)(data.c_str()), data.size());
        if (data == "EOF")
          B2INFO("Destroying EvtReceiver..");
        //break;
      }

      usleep(100);
    }

    return 1;
  }

  return 0;
}

int SignalMan::clearBuffer(void)
{
  return 0;
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

const void SignalMan::setBuffer(HLTBuffer* inBuffer, HLTBuffer* outBuffer)
{
  m_inBuffer = inBuffer;
  m_outBuffer = outBuffer;
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
  if ((m_pidEvtSender != 0) && (m_pidEvtReceiver != 0))
    return 1;
  else
    return 0;
}

std::string SignalMan::get(void)
{
  if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
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
  }
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

void SignalMan::broadCasting(void)
{
  m_sender.connect();
}

std::string SignalMan::listening(void)
{
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
