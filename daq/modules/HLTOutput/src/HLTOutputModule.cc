/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/modules/HLTOutput/HLTOutputModule.h>

using namespace Belle2;

REG_MODULE(HLTOutput)

/* @brief HLTOutputModule constructor
 * This initializes member variables from given parameters
*/
HLTOutputModule::HLTOutputModule() : Module()
{
  setDescription("HLTOutput module");
  setPropertyFlags(c_Input);

  m_file = 0;

  addParam("nodeType", m_nodeType, std::string("Node type of the node"));
  addParam("inputFileName", m_inputFileName, "SeqRoot file name.", std::string("SeqRootInput.root"));
}

/// @brief HLTOutputModule destructor
HLTOutputModule::~HLTOutputModule()
{
}

/* @brief Initialize the module
 * This sets related components like ring buffer, MsgHandler, and DataStore iterators
*/
void HLTOutputModule::initialize()
{
  B2INFO("Module HLTOutput initializing...");

  m_eventsSent = 0;
  m_nEvents = 0;

  gSystem->Load("libdataobjects");

  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(gDataOutBufferKey).c_str(), gBufferSize);
  m_streamer = new DataStoreStreamer();

  // If the node is event separator, it reads data from file
  if (m_nodeType == "ES") {
    StoreObjPtr<EventMetaData>::registerPersistent();

    m_file = new SeqFile(m_inputFileName.c_str(), "r");
    if (m_file->status() <= 0)
      B2FATAL("HLTOutput : Error in opening input file : " << m_inputFileName);
  }

  gettimeofday(&m_t0, 0);

  m_size = 0.0;
  m_size2 = 0.0;
}

/// @brief Begin a run
void HLTOutputModule::beginRun()
{
  B2INFO("Module HLTOutput starts a run");
}

/// @brief Process an event
void HLTOutputModule::event()
{
  struct timeval t0;
  struct timeval t1;
  gettimeofday(&t0, 0);

  B2INFO("Module HLTOutput starts an event");

  putData();
  m_eventsSent++;
  B2DEBUG(100, "[HLTOutput] " << m_eventsSent << " events sent!");

  gettimeofday(&t1, 0);

  double etime = (double)((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec));
  m_timeEvent += etime;
}

/// @brief End a run
void HLTOutputModule::endRun()
{
  gettimeofday(&m_tEnd, 0);
  double etime = (double)((m_tEnd.tv_sec - m_t0.tv_sec) * 1000000 + (m_tEnd.tv_usec - m_t0.tv_usec));

  double flowmb = m_size / etime * 1000.0;
  double flowmbEvent = m_size / m_timeEvent * 1000.0;
  double flowmbSer = m_size / m_timeSerialized * 1000.0;
  double flowmbIO = m_size / m_timeIO * 1000.0;
  double avesize = m_size / (double)m_nEvents;
  double avesize2 = m_size2 / (double)m_nEvents;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  std::cout << "[HLTOutput] \x1b[32mStatistics\x1b[0m" << std::endl;;
  std::cout << "[HLTOutput] \x1b[32m\t" << m_nEvents << " events (Total size = " << m_size << " kB)\x1b[0m" << std::endl;
  std::cout << "[HLTOutput] \x1b[32m\tTotal elapsed time  = " << etime / 1000.0 << " ms\x1b[0m" << std::endl;
  std::cout << "[HLTOutput] \x1b[32m\tFlow rate (Overall) = " << flowmb << " (MB/s)\x1b[0m" << std::endl;
  std::cout << "[HLTOutput] \x1b[32m\tFlow rate (Event)   = " << flowmbEvent << " (MB/s)   Time = " << m_timeEvent << " (ms)\x1b[0m" << std::endl;
  if (m_nodeType == "WN") {
    std::cout << "[HLTOutput] \x1b[32m\tFlow rate (Serialization) = " << flowmbSer << " (MB/s)   Time = " << m_timeSerialized / 1000.0 << " (ms)\x1b[0m" << std::endl;
  } else {
    std::cout << "[HLTOutput] \x1b[32m\tFlow rate (File reading)  = " << flowmbIO << " (MB/s)   Time = " << m_timeIO / 1000.0 << " (ms)\x1b[0m" << std::endl;
  }
  std::cout << "[HLTOutput] \x1b[32m\tEvent size = " << avesize << " +- " << sigma << " (kB)\x1b[0m" << std::endl;

  B2INFO("Module HLTOutput ends a run");
}

/// @brief Terminate the module
void HLTOutputModule::terminate()
{
  sendTerminate();
  delete m_streamer;
  B2INFO("Module HLTOutput terminating...");
}

/// @brief Put an event data into ring buffer for outgoing communication
/// @param durability Durability of the event data
void HLTOutputModule::putData()
{
  int size = 0;

  struct timeval t0;
  struct timeval tn;

  if (m_nodeType == "ES") {
    gettimeofday(&t0, 0);

    char* buffer = new char[gDataMaxReceives];
    size = m_file->read(buffer, gDataMaxReceives);

    gettimeofday(&tn, 0);

    if (size > 0) {
      EvtMessage* msg = NULL;

      B2INFO("[HLTOutput] \x1b[33mNode type = ES: Reading data from " << m_inputFileName << "\x1b[0m");
      msg = new EvtMessage(buffer);

      while (m_buffer->insq((int*)msg->buffer(), msg->size() / 4 + 1) <= 0) {
        usleep(100);
      }

      m_nEvents++;
      double dsize = (double)size / 1000.0;
      B2INFO("ADD SIZE " << dsize);
      m_size += dsize;
      m_size2 += dsize * dsize;

      delete msg;

      // To keep event by event processing, insert dummy EventMetaData into DataStore
      // (This information will not actually be used)
      StoreObjPtr<EventMetaData> eventMetaDataPtr;
      eventMetaDataPtr.create();
      eventMetaDataPtr->setEvent(m_nEvents);

    } else
      delete m_file;

    delete[] buffer;
  } else {
    gettimeofday(&t0, 0);

    EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

    gettimeofday(&tn, 0);

    while (m_buffer->insq((int*)msg->buffer(), msg->size() / 4 + 1) <= 0) {
      usleep(100);
    }
    B2INFO("[HLTOutput] Put an event into the ring buffer (size = " << msg->size() << ")");

    m_nEvents++;
    size = msg->size();

    delete msg;
  }

  double etime1 = (double)((tn.tv_sec - t0.tv_sec) * 1000000 + (tn.tv_usec - t0.tv_usec));
  m_timeSerialized += etime1;
  m_timeIO += etime1;

  double dsize = (double)size / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
}

/// @brief Send terminate code to ring buffer
void HLTOutputModule::sendTerminate()
{
  B2DEBUG(100, "[HLTOutput] Termination code sending");
  while (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
    usleep(100);
  }
}
