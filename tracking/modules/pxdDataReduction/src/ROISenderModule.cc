/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROISenderModule.h>
#include <sys/stat.h>
#include <chrono>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ROISender)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROISenderModule::ROISenderModule() :
  Module(), m_messageQueueNameCstring(nullptr)
{
  //Set module properties
  setDescription("Send the ROI payload to the external ring buffer");

  addParam("MessageQueueName", m_messageQueueName, "name of the output message queue", std::string("/roi"));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));
  addParam("MessageQueueDepth", m_messageQueueDepth, "depth of the output message queue", 10);
  addParam("MessageSize", m_messageQueueMsgSize, "max size of themessage", 8192);

  B2INFO("ROI Sender created");
}


void
ROISenderModule::initialize()
{
  // Required input
  m_eventMetaData.isRequired();
  m_roiPayload.isRequired(m_ROIpayloadName);

  m_messageQueueNameCstring =  m_messageQueueName.c_str();

  bool slashFree = (nullptr == strchr(m_messageQueueNameCstring + 1 , '/'));

  if (! slashFree || m_messageQueueNameCstring[0] != '/')
    B2FATAL(__FILE__ << ":" << __LINE__ <<
            m_messageQueueName << " invalid. cfr: man mq_overview ");

  m_histo.resize(101, 0); // 0-99, 100 is used as overflow bin

  //  unlinkMessageQueue("on initialize");
  openMessageQueue("on initialize");

}


void
ROISenderModule::event()
{

  int length = m_roiPayload->getPacketLengthByte();
  const char* data = (const char*) m_roiPayload->getRootdata();

  mqd_t ret;

  if (length <= m_messageQueueMsgSize) {
    ret = mq_send(m_messageQueue, data, length, 0 /* priority */);

    if (ret == (mqd_t) - 1) {
      B2FATAL(std::string(__FILE__) << ":" << __LINE__ <<
              ": error: " <<
              strerror(errno) <<
              " on mq_send");
    }
  } else {
    B2FATAL(std::string(__FILE__) << ":" << __LINE__  <<
            "ROI payload too long." << endl <<
            "Payload length     = " << length << endl <<
            "Message max length = " << m_messageQueueMsgSize << endl <<
            "We stop here, as this will result in event mismatch on EB! Please increase mqueue message length on HLT and/or check size limit in ROIPayload Assembler"
            << endl);
  }

  // Calculate the time difference between now and the trigger time
  // This tells you how much delay we have summed up (it is NOT the processing time!)
  /** Time(Tag) from MetaInfo, ns since epoch */
  unsigned long long int meta_time = m_eventMetaData->getTime();

  using namespace std::chrono;
  nanoseconds ns = duration_cast< nanoseconds >(system_clock::now().time_since_epoch());
  Float_t deltaT = (std::chrono::duration_cast<seconds> (ns - (nanoseconds)meta_time)).count();
  if (deltaT < 0) {
    m_histo[0]++;// just in case the clocks are slightly out of sync
  } else if (deltaT < 100) {
    m_histo[int(deltaT)]++;
  } else {
    m_histo[100]++;// overflow bin
  }
  if (deltaT > 60) {
    B2ERROR("Event took too long on HLT, PXD data for Event might be lost!" << LogVar("deltaT in s", deltaT));
  } else if (deltaT > 30) {
    B2WARNING("Event took too long on HLT, PXD data for Event might be lost!" << LogVar("deltaT in s", deltaT));
  }
}




void
ROISenderModule::terminate()
{
  closeMessageQueue("on terminate");
  //  unlinkMessageQueue("on terminate");
  string str = "HLT Delay time distribution: ( ";
  for (auto& a : m_histo) str += to_string(a) + ";";
  str += " )";
  B2RESULT(str);
}

void
ROISenderModule::openMessageQueue(const char* log_string)
{
  struct mq_attr attr;
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = m_messageQueueDepth;
  attr.mq_msgsize = m_messageQueueMsgSize; // bytes

  int oflags = O_WRONLY ; //| O_CREAT | O_EXCL;
  mode_t mode =  S_IRUSR | S_IWUSR | S_IROTH |  S_IRGRP ;


  mqd_t ret = mq_open(m_messageQueueNameCstring, oflags, mode, &attr);


  if (ret == (mqd_t) - 1)
    B2FATAL(std::string(__FILE__) << ":" <<
            __LINE__ << ": error: " <<
            strerror(errno) <<
            " on mq_open " << log_string);

  m_messageQueue = ret;

}

void
ROISenderModule::closeMessageQueue(const char* log_string)
{
  mqd_t ret;

  ret = mq_close(m_messageQueue);
  if (ret == (mqd_t) - 1)
    B2WARNING(std::string(__FILE__) << ":" <<
              __LINE__ << ": error: " <<
              strerror(errno) <<
              " on mq_close " << log_string);

}


/// Not sure if we ever need to unlink the queue, but lets keep the code for reference
void
ROISenderModule::unlinkMessageQueue(const char* log_string)
{
  mqd_t ret;
  ret = mq_unlink(m_messageQueueNameCstring);
  if (ret == (mqd_t) - 1)
    B2WARNING(std::string(__FILE__) << ":" <<
              __LINE__ << ": error: " <<
              strerror(errno) <<
              " on mq_unlink " << log_string);

}

