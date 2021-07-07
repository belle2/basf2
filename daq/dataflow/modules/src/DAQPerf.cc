/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/dataflow/modules/DAQPerf.h>

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawCOPPER.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DAQPerf)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DAQPerfModule::DAQPerfModule() : Module()
{
  //Set module properties
  setDescription("Monitor DAQ transfer performance");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Cycle", m_ncycle, "Monitor cycle", 10000);
  addParam("MonitorSize", m_mon, "SW to monitor data flow", false);

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


DAQPerfModule::~DAQPerfModule()
{
}

void DAQPerfModule::initialize()
{
  m_nevent = 0;
  m_t0.tv_sec = 0;
  m_t0.tv_usec = 0;
  m_totbytes = 0;
}


void DAQPerfModule::beginRun()
{
}


void DAQPerfModule::event()
{
  if (m_nevent % m_ncycle == 0) {
    if (m_t0.tv_sec == 0) {
      gettimeofday(&m_t0, NULL);
      m_totbytes = 0;
    } else {
      timeval t;
      gettimeofday(&t, NULL);
      double now = (double)t.tv_sec + (double)t.tv_usec * 0.000001;
      double t0 = (double)m_t0.tv_sec + (double)m_t0.tv_usec * 0.000001;
      double elapsed = now - t0;
      printf("Event = %d : took %5.2f sec for %d evts -> %5.2f kHz",
             m_nevent, elapsed, m_ncycle, (double)m_ncycle / elapsed / 1000);
      if (m_mon) {
        double aveflow = m_totbytes / elapsed / 1000000.0;
        m_totbytes = 0.0;
        printf(",  %3.2f (MB/s)", aveflow);
      }
      printf("\n");
      gettimeofday(&m_t0, NULL);
    }
  }
  // Calculate data size
  if (m_mon) {
    StoreArray<RawCOPPER> rawcprary;
    double rawsize = 0;
    for (unsigned int i = 0; i < rawcprary.getEntries(); i++) {
      rawsize += (double)(rawcprary[i]->TotalBufNwords());
    }
    m_totbytes += rawsize * 4;
  }
  m_nevent++;
  return;
  //  return type;
}

void DAQPerfModule::endRun()
{
  //fill Run data
}


void DAQPerfModule::terminate()
{
}

