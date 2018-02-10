/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreObjPtr.h>
#include <tracking/modules/pxdDataReduction/ROISenderModule.h>
#include <tracking/dataobjects/ROIpayload.h>
#include <sys/stat.h>

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
  Module(), m_messageQueueNameCstring(NULL)
{
  //Set module properties
  setDescription("Send the ROI payload to the external ring buffer");

  addParam("MessageQueueName", m_messageQueueName, "name of the output message queue", std::string("/roi"));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));
  addParam("MessageQueueDepth", m_messageQueueDepth, "depth of the output message queue", 10);
  addParam("MessageSize", m_messageQueueMsgSize, "max size of themessage", 8192);

  B2INFO("ROI Sender created");
}


ROISenderModule::~ROISenderModule()
{

}


void
ROISenderModule::initialize()
{
  StoreObjPtr<ROIpayload> roiPayloads;
  roiPayloads.isRequired(m_ROIpayloadName);

  m_messageQueueNameCstring =  m_messageQueueName.c_str();

  bool slashFree = (NULL == strchr(m_messageQueueNameCstring + 1 , '/'));

  if (! slashFree || m_messageQueueNameCstring[0] != '/')
    B2FATAL(__FILE__ << ":" << __LINE__ <<
            m_messageQueueName << " invalid. cfr: man mq_overview ");


  //  unlinkMessageQueue("on initialize");
  openMessageQueue("on initialize");

}

void
ROISenderModule::beginRun()
{


}


void
ROISenderModule::event()
{

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);
  int length = payloadPtr->getPacketLengthByte();
  const char* data = (const char*) payloadPtr->getRootdata();

  mqd_t ret;

  if (length <= m_messageQueueMsgSize) {
    ret = mq_send(m_messageQueue, data, length, 0 /* priority */);

    if (ret == (mqd_t) - 1)
      B2FATAL(std::string(__FILE__) << ":" << __LINE__ <<
              ": error: " <<
              strerror(errno) <<
              " on mq_send");
  } else
    B2WARNING(std::string(__FILE__) << ":" << __LINE__  <<
              " ROI payload too long." << endl <<
              " Payload length     = " << length << endl <<
              " Message max lengtt = " << m_messageQueueMsgSize << endl);

}


void
ROISenderModule::endRun()
{
}


void
ROISenderModule::terminate()
{
  closeMessageQueue("on terminate");
  //  unlinkMessageQueue("on terminate");
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

