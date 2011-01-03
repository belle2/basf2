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

/* @brief SignalMan constructor
 * Is this really needed?
*/
SignalMan::SignalMan(void)
    : m_inPort(-1), m_outPort(-1)
{
}

/* @brief SignalMan constructor with port numbers
 * Is this really needed, again?
 * @param inPort Port number for incoming data
 * @param outPort Port number for outgoing data
*/
SignalMan::SignalMan(const int inPort, const int outPort)
    : m_inPort(inPort), m_outPort(outPort)
{
}

/* @brief SignalMan constructor with port numbers and single destination
 * @param inPort Port number for incoming data
 * @param outPort Port number for outgoing data
 * @param dest IP address of destination
*/
SignalMan::SignalMan(const int inPort, const int outPort, std::string dest)
    : m_inPort(inPort), m_outPort(outPort)
{
  m_dest.clear();
  m_dest.push_back(dest);
}

/* @brief SignalMan constructor with port numbers and multiple destination
 * @param inPort Port number for incoming data
 * @param outPort Port number for outgoing data
 * @param dest Vector container of IP addresses of destinations
 * @param mode I don't remember what this is...
*/
SignalMan::SignalMan(const int inPort, const int outPort, std::vector<std::string> dest, const std::string mode)
    : m_inPort(inPort), m_outPort(outPort)
{
  m_dest.clear();
  m_dest = dest;
}

/* @brief SignalMan destructor
 * Ring buffers for IPC should be only managed by the framework
*/
SignalMan::~SignalMan(void)
{
  if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
    int status1, status2;

    std::string endOfRun("EOF");
    B2INFO("Terminating EvtReceiver...");

    B2INFO("Terminating EvtSender...");
    m_outBuf->insq((int*)(endOfRun.c_str()), endOfRun.size());

    waitpid(m_pidEvtSender, &status1, 0);
    waitpid(m_pidEvtReceiver, &status2, 0);

    delete m_inBuf;
    delete m_outBuf;
  }
}

/* @brief Initialization of SignalMan
 * Buffer size is hard-coded now and it should be flexible somehow
 * @return c_Success Initialization success (only framework returns this)
 * @return c_InitFailed Initialization failed
 * @return c_TermCalled Termination of forked processes (EvtSender and EvtReceiver)
*/
EStatus SignalMan::init(void)
{
  char inBufName[] = "inBuffer";
  char outBufName[] = "outBuffer";

  m_inBuf = new RingBuffer(inBufName, 1024);
  m_outBuf = new RingBuffer(outBufName, 1024);

  return doCommunication();
}

/* @brief Fork EvtSender and EvtReceiver and let them work
 * @return c_Success Forking success (only framework returns this)
 * @return c_InitFailed Initialization failed (only EvtSender and EvtReceiver return this)
 * @return c_TermCalled Termination of forked processes (EvtSender and EvtReceiver)
*/
EStatus SignalMan::doCommunication(void)
{
  // Forking off an event sender to send data
  m_pidEvtSender = fork();
  if (m_pidEvtSender == 0) {
    m_pidEvtReceiver = 1;
    // It should take care of multiple destinations somehow..
    m_sender = EvtSender(m_dest[0], m_outPort);
    if (m_sender.init(m_outBuf) == c_InitFailed) {
      B2ERROR("Could not initialize EvtSender.");
      return c_InitFailed;
    }

    B2INFO("EvtSender initialized");

    while (1) {
      EStatus status = m_sender.broadCasting();

      if (status == c_TermCalled) {
        B2INFO("Destroying EvtSender..");
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

/* @brief Set port numbers to be used
 * @param inPort port number for incoming data
 * @param outPort port number for outgoing data
*/
const void SignalMan::setPorts(const int inPort, const int outPort)
{
  m_inPort = inPort;
  m_outPort = outPort;
}

/* @brief Set IP address of single destination
 * @param dest IP address of the destination
*/
const void SignalMan::setDest(const std::string dest)
{
  m_dest.clear();
  m_dest.push_back(dest);
}

/* @brief Set IP addresses of multiple destinations
 * @param dest IP addresses of the destinations
*/
const void SignalMan::setDest(std::vector<std::string> dest)
{
  m_dest.clear();
  m_dest = dest;
}

/* @brief Get incoming buffer
 * @return Pointer to the incoming buffer
*/
RingBuffer* SignalMan::getInBuffer()
{
  return m_inBuf;
}

/* @brief Get outgoing buffer
 * @return Pointer to the outgoing buffer
*/
RingBuffer* SignalMan::getOutBuffer()
{
  return m_outBuf;
}

/* @brief Get IP addresses of destinations
 * @return Vector container which has IP addresses of destinations
*/
std::vector<std::string> SignalMan::dest()
{
  return m_dest;
}

/* @brief Test if the process is the framework or not
 * @return 1 if the process is the framework
 * @return 0 if the process is not the framework
*/
int SignalMan::isMother()
{
  return (m_pidEvtSender && m_pidEvtReceiver);
}

/* @brief test if the process is EvtSender of not
 * @return 1 if the process is EvtSender
 * @return 0 if the process is not EvtSender
*/
int SignalMan::isEvtSender()
{
  return !(m_pidEvtSender);
}

/* @brief test if the process is EvtReceiver of not
 * @return 1 if the process is EvtReceiver
 * @return 0 if the process is not EvtReceiver
*/
int SignalMan::isEvtReceiver()
{
  return !(m_pidEvtReceiver);
}

/* @brief Get data from incoming buffer
 * @return Serialized data transferred
*/
std::string SignalMan::get(void)
{
  while (1) {
    if (m_inBuf->numq() > 0) {
      char* tmp = new char[255];
      m_inBuf->remq((int*)tmp);
      std::string input(tmp);

      return input;
    }
  }
}

/* @brief Put data into outgoing buffer
 * @param data Serialized data to be transferred
*/
void SignalMan::put(const std::string data)
{
  m_outBuf->insq((int*)(data.c_str()), data.size());
}

/* @brief I don't even remember the intension of this function now...
 * Maybe it should be removed
*/
void SignalMan::broadCasting(void)
{
  m_sender.connect();
}

/* @brief I don't even remember the intension of this function now...
 * Maybe it should be removed (even worse this is the same with get() function
*/
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
}

/* @brief Displaying information of SignalMan (only for debugging)
*/
const void SignalMan::Print(void)
{
  B2INFO("=================================================");
  B2INFO(" SignalMan Summary");
  B2INFO("   Incoming port# = " << m_inPort);
  B2INFO("   Outgoing port# = " << m_outPort);
  B2INFO("=================================================");
}
