//+
// File : DepfetPFDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/depfet/modules/DepfetPFDeserializer.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/Time.h>

#include <TSystem.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DepfetPFDeserializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DepfetPFDeserializerModule::DepfetPFDeserializerModule() : Module()
{
  setDescription("Encode DataStore into RingBuffer");

  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("InputBufferName", m_ibuf_name, "Input buffer name", std::string(""));
  addParam("InputBufferSize", m_ibuf_size, "Input buffer size", 100);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);
  B2DEBUG(100, "DepfetPFDeserializer: Constructor done.");
  m_evtbuf = new int [2000000];
}


DepfetPFDeserializerModule::~DepfetPFDeserializerModule()
{
  delete [] m_evtbuf;
}

void DepfetPFDeserializerModule::initialize()
{
  std::cout << "DepfetPFDeserializer: initialize() started." << std::endl;
  StringList s = StringUtil::split(m_ibuf_name, ',');
  m_ibuf.resize(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i].size() > 0 && m_ibuf_size > 0) {
      m_ibuf[i].open(s[i], m_ibuf_size * 1000000);
    } else {
      B2FATAL("Failed to load arguments for shared buffer (" <<
              m_ibuf_name.c_str() << ":" << m_ibuf_size << ")");
    }
  }
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      B2INFO("Open runinfo : nodename = " << m_nodename << " nodeid = " << m_nodeid);
      m_info.open(m_nodename, m_nodeid);
    }
  }
  m_rawdata.registerInDataStore();
  m_evtmeta.registerInDataStore();
  if (m_info.isAvailable()) {
    m_info.reportReady();
  }
  m_count = 0;
  event();
  if (m_info.isAvailable()) {
    m_info.reportRunning();
  }
  std::cout << "DepfetPFDeserializer: initialize() done." << std::endl;
}

Time g_t0;
double g_datasize = 0;

void DepfetPFDeserializerModule::event()
{
  m_count++;
  if (m_count == 1) return;
  unsigned int nword = 0;
  m_ibuf[m_count % m_ibuf.size()].read(m_evtbuf, false, false);
  nword = m_evtbuf[0];
  if (m_info.isAvailable()) {
    m_info.addInputNBytes(m_evtbuf[0]*sizeof(int));
    m_info.setInputCount(m_count);
  }
  BinData data;
  data.setBuffer(m_evtbuf);
  m_evtmeta.create();
  m_evtmeta->setExperiment(data.getExpNumber());
  m_evtmeta->setRun(data.getRunNumber());
  m_evtmeta->setEvent(data.getEventNumber());
  int nboard = m_evtbuf[2];//data.getNBoard();
  int offset = sizeof(BinHeader) / sizeof(int);
  for (int i = 0; i < nboard; i++) {
    BinData cdata;
    cdata.setBuffer(m_evtbuf + offset);
    m_rawdata.appendNew(cdata.getBuffer(), cdata.getWordSize());
    offset += cdata.getWordSize();
  }
  g_datasize += nword * sizeof(int);
  static int interval = 10000;
  if (m_count % interval == 0) {
    Time t;
    double dt = (t.get() - g_t0.get());
    double freq = interval / dt  / 1000.;
    double rate = g_datasize / dt / 1000. / 1000. / 1000. * 8;
    printf("nboard = %d, Serial = %u Freq = %f [kHz], Rate = %f [Gbps], DataSize = %f [kB/event]\n",
           nboard, m_count, freq, rate, g_datasize / 1000. / interval);
    g_t0 = t;
    g_datasize = 0;
  }
  if (m_count < 10000 && (m_count < 10 || (m_count > 10 && m_count < 100 && m_count % 10 == 0) ||
                          (m_count > 100 && m_count < 1000 && m_count % 100 == 0) ||
                          (m_count > 1000 && m_count < 10000 && m_count % 1000 == 0))) {
    std::cout << "[DEBUG] " << m_nodename << " count = " << m_count << " nword = " << nword << std::endl;
  }
  m_count++;
  if (m_count % 10 == 0) {
    if (m_info.isAvailable()) {
      m_info.setExpNumber(data.getExpNumber());
      m_info.setRunNumber(data.getRunNumber());
      m_info.setInputCount(m_count);
      m_info.addInputNBytes(nword * sizeof(int));
    }
  }
}

void DepfetPFDeserializerModule::beginRun()
{
  std::cout << "DepfetPFDeserializer: beginRun called." << std::endl;
}

void DepfetPFDeserializerModule::endRun()
{
  std::cout << "DepfetPFDeserializer: endRun done." << std::endl;
}


void DepfetPFDeserializerModule::terminate()
{
  std::cout << "DepfetPFDeserializer: terminate called" << std::endl;
}


