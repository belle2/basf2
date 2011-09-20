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
 * If the process is the framework, try to terminate EvtSender and EvtReceiver
 * For EvtSender, put "EOF" into the ring buffer then EvtSender terminates
 * For EvtReceiver, no idea so far (but should terminates somehow)
 * HLT buffers for IPC should be only managed by the framework
 * @todo Implement EvtReceiver termination part
*/
SignalMan::~SignalMan(void)
{
  if (m_pidEvtSender != 0 && m_pidEvtReceiver != 0) {
    //B2INFO("[PID] pidEvtSender = " << m_pidEvtSender << " / pidEvtReceiver = " << m_pidEvtReceiver);
    int status1;

    //std::string endOfRun("EOF");
    //B2INFO("Terminating EvtReceiver...");

    //B2INFO("Terminating EvtSender...");
    char* endOfRun = "EOF";
    m_outBuf->insq((int*)endOfRun, 3 / 4 + 1);
    //m_outBuf->insq((int*)(endOfRun.c_str()), endOfRun.size());

    waitpid(m_pidEvtSender, &status1, 0);
    //waitpid(m_pidEvtReceiver, &status2, 0);

    delete m_inBuf;
    delete m_outBuf;
  }
}

/* @brief Initialization of SignalMan
 * Buffer size is hard-coded now and it should be flexible somehow
 * @todo Buffer size parameterize
 * @return c_Success Initialization success (only framework returns this)
 * @return c_InitFailed Initialization failed
 * @return c_TermCalled Termination of forked processes (EvtSender and EvtReceiver)
*/
//EStatus SignalMan::init(const std::string inBufName, const std::string outBufName)
//EStatus SignalMan::init(EHLTPort inBufKey, EHLTPort outBufKey)
EStatus SignalMan::init(int inBufKey, int outBufKey)
{
  //B2INFO("Starting to initialize SignalMan");
  //m_inBuf = new HLTBuffer(inBufName.c_str(), gBufferSize);
  //m_outBuf = new HLTBuffer(outBufName.c_str(), gBufferSize);
  m_inBuf = new HLTBuffer(inBufKey, gBufferSize);
  m_outBuf = new HLTBuffer(outBufKey, gBufferSize);

  m_inBuf->clear();
  m_outBuf->clear();

  m_reconMessage = new char[MAXPACKETSIZE];
  memset(m_reconMessage, 0, MAXPACKETSIZE);
  m_reconOffset = 0;

  return c_Success;
}

/* @brief Run EvtSender and EvtReceiver both
 * @return c_Success Forking success (only framework returns this)
 * @return c_InitFailed Initialization failed (only EvtSender and EvtReceiver return this)
 * @return c_TermCalled Termination of forked processes (EvtSender and EvtReceiver)
*/
EStatus SignalMan::doCommunication(void)
{
  EStatus returnCode = runEvtReceiver();
  if (returnCode == c_Success)
    returnCode = runEvtSender();

  return returnCode;
}

/* @brief Run EvtSender only using forking
 * @return c_Success Forking success (only framework returns this)
 * @return c_InitFailed Initialization failed (only EvtSender returns this)
 * @return c_TermCalled Termination of EvtSender
 * @todo Implement a part of destination decider
*/
EStatus SignalMan::runEvtSender(void)
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

    B2INFO("EvtSender initialized to " << m_dest[0] << ":" << m_outPort << "(pid=" << m_pidEvtSender << ")");

    while (1) {
      //EStatus status = m_sender.broadCasting();
      m_sender.broadCasting();

      /*
      if (status == c_TermCalled) {
        B2INFO("Destroying EvtSender..");
        return c_TermCalled;
      }
      */

      usleep(100);
    }

    return c_ChildSuccess;
  }

  return c_Success;
}

/* @brief Run EvtReceiver only using forking
 * @return c_Success Forking success (only framework returns this)
 * @return c_InitFailed Initialization failed (only Receiver returns this)
 * @return c_TermCalled Termination of EvtReceiver
*/
EStatus SignalMan::runEvtReceiver(void)
{
  // Forking off an event receiver to take data
  m_pidEvtReceiver = fork();
  if (m_pidEvtReceiver == 0) {
    m_pidEvtSender = 1;
    m_receiver = EvtReceiver(m_inPort);
    if (m_receiver.init(m_inBuf) == c_InitFailed) {
      B2ERROR("Could not initialize EvtReceiver.");
      return c_InitFailed;
    }

    B2INFO("EvtReceiver initialized to port " << m_inPort << "(pid=" << m_pidEvtSender << ")");

    if (m_receiver.listen() != c_Success) {
      B2ERROR("Listening failed)");
      return c_FuncError;
    }

    EvtReceiver new_sock;
    m_receiver.accept(new_sock);
    char* data = new char [MAXPACKETSIZE];
    memset(data, 0, MAXPACKETSIZE);
    //std::string data;

    B2INFO("listening incomings...");

    while (1) {
      int size = new_sock.recv(data);
      //std::cout << "[SignalMan]: INTERMIDIATE CHECK = " << data[size - 12] << std::endl;

      char filename[255];
      sprintf(filename, "receiveLog%ld_%d", time(NULL), size);
      FILE* fp = fopen(filename, "w");

      for (int i = 0; i < size; i++) {
        fprintf(fp, "%c", data[i]);
      }

      fclose(fp);

      if (footerChecker(data, size)) {
        if (m_reconOffset == 0) {
          m_inBuf->insq((int*)data, (size - 1) / 4 + 1);
        } else {
          B2INFO("SignalMan: Merging " << size << " at offset " << m_reconOffset);
          printf("%x\n", m_reconMessage + m_reconOffset);
          //memcpy (*(&m_reconMessage + m_reconOffset), data, size);
          memcpy(m_reconMessage + m_reconOffset, data, size);

          m_inBuf->insq((int*)m_reconMessage, ((m_reconOffset + size) - 1) / 4 + 1);

          char filename[255];
          sprintf(filename, "concatenatedLog%ld", time(NULL));
          FILE* fp = fopen(filename, "w");

          for (int i = 0; i < m_reconOffset + size; i++) {
            fprintf(fp, "%c", m_reconMessage[i]);
          }

          fclose(fp);

          memset(m_reconMessage, 0, MAXPACKETSIZE);
          m_reconOffset = 0;
        }
        B2INFO("SignalMan: INTERNAL BUFFER FLUSHING");
      } else {
        B2INFO("SignalMan: Storing partial data into internal container");
        B2INFO("SignalMan: Merging " << size << " at offset " << m_reconOffset);
        printf("%x\n", m_reconMessage + m_reconOffset);
        memcpy(m_reconMessage + m_reconOffset, data, size);
        m_reconOffset += size;
      }

      memset(data, 0, MAXPACKETSIZE);
      //memset (data + (size - 11), 0, 11);
      //new_sock >> data;
      //if (data.size() > 0) {
      /*
      if (size > 0) {
        //char* dataTaken = new char [MAXPACKETSIZE];
        //strcpy(dataTaken, data.c_str());
        //memcpy (dataTaken, data, size);
        //strcpy(dataTaken, data);
        //B2INFO("Message taken: " << data << " (size = " << size << ")");
        //m_inBuf->insq((int*)dataTaken, (data.size() / 4 + 1));
        //m_inBuf->insq((int*)dataTaken, ((data.size () - 1) / 4 + 1));
        m_inBuf->insq((int*)data, (size - 1) / 4 + 1);

        //m_inBuf->insq((int*)(data.c_str()), data.size());
        if (data == "EOF") {
          B2INFO("Destroying EvtReceiver..");
          return c_TermCalled;
        }
      }
        */

      usleep(100);
    }

    return c_ChildSuccess;
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
HLTBuffer* SignalMan::getInBuffer()
{
  return m_inBuf;
}

/* @brief Get outgoing buffer
 * @return Pointer to the outgoing buffer
*/
HLTBuffer* SignalMan::getOutBuffer()
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
      char tmp[MAXPACKETSIZE];
      B2INFO("AHYO");
      m_inBuf->remq((int*)tmp);
      std::string input(tmp);
      B2INFO(input);

      return input;
    }
  }
}

/* @brief Put data into outgoing buffer
 * @param data Serialized data to be transferred
*/
EStatus SignalMan::put(const std::string data)
{
  char* dataPut = new char [data.size() + 1];
  strcpy(dataPut, data.c_str());
  if (m_outBuf->insq((int*)dataPut, data.size() / 4 + 1) == -1)
    //if (m_outBuf->insq((int*)(data.c_str()), data.size()) == -1)
    return c_FuncError;
  else
    return c_Success;
}

/* @brief I don't even remember the intension of this function now...
 * Maybe it should be removed
*/
void SignalMan::broadCasting(void)
{
  m_sender.connect();
}

/* @brief I don't even remember the intension of this function now...
 * Maybe it should be removed (even worse this is the same with get() function)
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

bool SignalMan::footerChecker(char* data, int size)
{
  bool flag = false;

  for (int i = 0; i < size; i++) {
    // broken EOS handing should be included here
    if (data[i] == 'E' && data[i + 1] == 'O' && data[i + 2] == 'S') {
      flag = true;
    }
  }

  return flag;
}
