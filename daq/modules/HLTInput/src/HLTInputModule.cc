/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/modules/HLTInput/HLTInputModule.h>

using namespace Belle2;

REG_MODULE(HLTInput)

/* @brief HLTInputModule constructor
 * This initializes member variables from given parameters
*/
HLTInputModule::HLTInputModule() : Module()
{
  setDescription("HLTInput module");
  setPropertyFlags(c_Input);

  addParam("nodeType", m_nodeType, std::string("Node type of the node"));
  addParam("dataSources", m_nDataSources, std::string("# of data sources"), 1);
  addParam("outputFileName", m_outputFileName, "SeqRoot file name.", std::string("SeqRootOutput.root"));
}

/// @brief HLTInputModule destructor
HLTInputModule::~HLTInputModule()
{
}

/* @brief Initialize the module
 * This sets related components like ring buffer and MsgHandler
*/
void HLTInputModule::initialize()
{
  B2INFO("Module HLTInput initializing...");
  gSystem->Load("libdataobjects");

  gettimeofday(&m_t0, 0);

  m_size = 0.0;
  m_size2 = 0.0;
  m_nEvents = 0;

  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(gDataInBufferKey).c_str(), gBufferSize);

  m_streamer = new DataStoreStreamer();
  int size = 0;

  if (m_nodeType == "EM" || m_nodeType == "WN") {
    StoreObjPtr<EventMetaData>::registerPersistent();

    // Prefetch the first event
    if (m_nodeType == "WN") {
      char* buffer = new char[gDataMaxReceives];
      while ((size = m_inBuffer->remq((int*)buffer)) <= 0)
        usleep(100);
      EvtMessage* msg = new EvtMessage(buffer);

      m_streamer->restoreDataStore(msg);
      double dsize = ((double)msg->size() - gEOSTag.size()) / 1000.0;
      m_size += dsize;
      m_size2 += dsize * dsize;
    } else
      m_file = new SeqFile(m_outputFileName.c_str(), "w");
  }
}

/// @brief Begin a run
void HLTInputModule::beginRun()
{
  B2INFO("Module HLTInput starts a run");
}

/// @brief Process an event
void HLTInputModule::event()
{
  B2INFO("Module HLTInput starts an event");

  // Since the first event already fetched in initialize () function only at WN,
  // the first event processing skipped to make consistent # of runs
  if (m_nEvents == 0 && m_nodeType == "WN") {
    B2INFO("[HLTInput] The first event already fetched -- skipping");
    m_nEvents++;
    return;
  }

  struct timeval t0;
  struct timeval t1;
  gettimeofday(&t0, 0);

  getData();

  B2INFO("[HLTInput] " << m_nEvents << " events taken!");
  gettimeofday(&t1, 0);

  double etime = (double)((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec));
  m_timeTest += etime;
}

/// @brief End a run
void HLTInputModule::endRun()
{
  gettimeofday(&m_tEnd, 0);
  double etime = (double)((m_tEnd.tv_sec - m_t0.tv_sec) * 1000000 + (m_tEnd.tv_usec - m_t0.tv_usec));

  double flowmb = m_size / etime * 1000.0;
  double flowmbInit = m_size / m_timeInit * 1000.0;
  double flowmbTest = m_size / m_timeTest * 1000.0;
  double flowmbDeser = m_size / m_timeDeserialized * 1000.0;
  double flowmbStore = m_size / m_timeStore * 1000.0;
  double avesize = m_size / (double)m_nEvents;
  double avesize2 = m_size2 / (double)m_nEvents;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  // Need to be modified (it doesn't work for some situations)
  std::cout << "[HLTInput] \x1b[32mStatistics\x1b[0m" << std::endl;;
  std::cout << "[HLTInput] \x1b[32m\t" << m_nEvents << " events (Total size = " << m_size << " kB)\x1b[0m" << std::endl;
  std::cout << "[HLTInput] \x1b[32m\tFlow rate = " << flowmb << " (MB/s)\x1b[0m" << std::endl;
  std::cout << "[HLTInput] \x1b[32m\tFlow rate (Test) = " << flowmbTest << " (MB/s)   Time = " << m_timeTest / 1000 << " (ms)\x1b[0m" << std::endl;
  std::cout << "[HLTInput] \x1b[32m\tFlow rate (Init) = " << flowmbInit << " (MB/s)   Time = " << m_timeInit / 1000 << " (ms)\x1b[0m" << std::endl;
  if (m_nodeType == "WN") {
    std::cout << "[HLTInput] \x1b[32m\tFlow rate (Deserialization) = " << flowmbDeser << " (MB/s)   Time = " << m_timeDeserialized / 1000 << " (ms)\x1b[0m" << std::endl;
    std::cout << "[HLTInput] \x1b[32m\tFlow rate (DataStore) = " << flowmbStore << " (MB/s)   Time = " << m_timeStore / 1000 << " (ms)\x1b[0m" << std::endl;
  }
  std::cout << "[HLTInput] \x1b[32m\tEvent size = " << avesize << " +- " << sigma << " (kB)\x1b[0m" << std::endl;

  B2INFO("Module HLTInput ends a run");
}

/// @brief Terminate the module
void HLTInputModule::terminate()
{
  if (m_nodeType == "EM")
    delete m_file;
  delete m_streamer;
  B2INFO("Module HLTInput terminating...");
}

/// @brief Get data from incoming ring buffer
/// @return c_Success Data received
/// @return c_TermCalled Termination code received and there's no more data sources left
EHLTStatus HLTInputModule::getData()
{
  struct timeval ti;
  struct timeval tt;
  struct timeval t0;
  struct timeval t1;
  struct timeval tn;

  char* buffer = new char[gDataMaxReceives + gEOSTag.size()];

  gettimeofday(&ti, 0);

  int size = 0;
  while ((size = m_inBuffer->remq((int*)buffer)) <= 0)
    usleep(100);
  B2INFO("[HLTInput] Event data taken from RB!");

  gettimeofday(&tt, 0);

  std::string termChecker(buffer);
  while (termChecker == gTerminate) {
    B2DEBUG(100, "\x1b[34m[HLTInput] Termination requested\x1b[0m");
    m_nDataSources--;
    if (m_nDataSources == 0) {
      B2DEBUG(100, "\x1b[34m[HLTInput] All data taken. Terminating...\x1b[0m");
      return c_TermCalled;
    } else {
      B2DEBUG(100, "\x1b[34m[HLTInput] " << m_nDataSources << " more data sources are left...\x1b[0m");
      while ((size = m_inBuffer->remq((int*)buffer)) <= 0)
        usleep(100);
      termChecker = std::string(buffer);
    }
  }
  B2DEBUG(150, "\x1b[31m[HLTInput] Worked by now\x1b[0m");
  B2DEBUG(150, "\x1b[31m[HLTInput] Widthrawal size = " << size * 4 << "\x1b[0m");

  EvtMessage* msg = new EvtMessage(buffer);

  if (m_nodeType == "EM") {
    gettimeofday(&t0, 0);
    gettimeofday(&t1, 0);
    gettimeofday(&tn, 0);
    m_file->write(msg->buffer());
    B2INFO("[HLTInput] Writing event " << msg->size() << " bytes to the file");

    // To keep event by event processing, insert dummy EventMetaData into DataStore
    // (This information will not actually be used)
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    eventMetaDataPtr.create();
    eventMetaDataPtr->setEvent(m_nEvents);

    size = msg->size();
  } else {
    gettimeofday(&t0, 0);
    gettimeofday(&t1, 0);

    m_streamer->restoreDataStore(msg);

    gettimeofday(&tn, 0);

    size = msg->size();
  }

  double etime0 = (double)((tt.tv_sec - ti.tv_sec) * 1000000 + (tt.tv_usec - ti.tv_usec));
  double etime1 = (double)((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec));
  double etime2 = (double)((tn.tv_sec - t1.tv_sec) * 1000000 + (tn.tv_usec - t1.tv_usec));
  m_timeInit += etime0;
  m_timeDeserialized += etime1;
  m_timeStore += etime2;

  double dsize = ((double)size - gEOSTag.size()) / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nEvents++;

  delete msg;
  delete[] buffer;

  return c_Success;
}
