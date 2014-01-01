
//+
// File : StorageDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/storage/modules/StorageDeserializer.h>
#include <TSystem.h>

#include "daq/storage/modules/StorageWorker.h"
#include "daq/storage/modules/DataStorePackage.h"

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/system/Time.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageDeserializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Time t0;
double datasize = 0;

StorageDeserializerModule::StorageDeserializerModule() : Module()
{
  setDescription("Encode DataStore into RingBuffer");

  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("InputBufferName", m_inputbufname, "Name of RingBuffer", std::string(""));
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);
  addParam("NumThreads", m_numThread, "Number of threads for object decoding", 1);

  m_inputbuf = NULL;
  m_nrecv = 0;
  m_running = false;

  B2INFO("StorageDeserializer: Constructor done.");
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  B2INFO("StorageDeserializer: initialize() started.");
  B2INFO(m_inputbufname.c_str());

  m_inputbuf = new RingBuffer(m_inputbufname.c_str());
  if (m_buf == NULL) {
    m_buf = new StorageRBufferManager(m_inputbuf);
  }
  StoreArray<RawPXD>::registerPersistent();
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_status.open(m_nodename, m_nodeid);
      m_status.reportReady();
    }
  }
  char* evtbuf = new char[10000000];
  m_data.setBuffer(evtbuf);
  int size = 0;
  while ((size = m_inputbuf->remq((int*)evtbuf)) == 0) {
    usleep(20);
  }
  DataStorePackage package;
  MsgHandler handler(m_compressionLevel);
  package.decode(handler, m_data);
  package.restore();
  for (int n = 0; n < m_numThread; n++) {
    PThread(new StorageWorker(m_buf, m_compressionLevel));
  }

  if (m_shmflag > 0) {
    m_status.reportRunning();
    m_running = true;
  }
  B2INFO("StorageDeserializer: initialize() done.");
}

void StorageDeserializerModule::event()
{
  m_nrecv++;
  while (m_package_q.empty()) {
    StorageWorker::lock();
    std::queue<DataStorePackage*>& package_q(StorageWorker::getQueue());
    while (package_q.empty()) {
      StorageWorker::wait();
    }
    while (!package_q.empty()) {
      m_package_q.push(package_q.front());
      package_q.pop();
    }
    StorageWorker::notify();
    StorageWorker::unlock();
  }
  DataStorePackage* package = m_package_q.front();
  m_package_q.pop();
  int length = m_package_q.size();
  datasize += package->getData().getByteSize();
  package->restore();
  if (m_nrecv % 100000 == 0) {
    Time t;
    double freq = 100. / (t.get() - t0.get());
    double rate = datasize / (t.get() - t0.get()) / 1000.;
    printf("Serial = %d Freq = %f [kHz], Rate = %f [kB/s], Queue = %d\n",
           package->getSerial(), freq, rate, length);
    t0 = t;
    datasize = 0;
  }
  delete package;
}

void StorageDeserializerModule::beginRun()
{
  B2INFO("StorageDeserializer: beginRun called.");
}

void StorageDeserializerModule::endRun()
{
  //fill Run data
  B2INFO("StorageDeserializer: endRun done.");
}


void StorageDeserializerModule::terminate()
{
  B2INFO("StorageDeserializer: terminate called")
}


