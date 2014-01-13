//+
// File : MonitorStorage.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/storage/modules/MonitorStorage.h>
#include <rawdata/dataobjects/RawSVD.h>

#include <cstdio>

using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MonitorStorage)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

storager_data MonitorStorageModule::g_data;

MonitorStorageModule::MonitorStorageModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor of data storage status");
  setPropertyFlags(c_ParallelProcessingCertified);

}



MonitorStorageModule::~MonitorStorageModule()
{
}

void MonitorStorageModule::defineHisto()
{
  m_h_runinfo = new TH1F("h_runinfo", "Starage run status;RunStatus;", 10, 0, 10);
}


void MonitorStorageModule::initialize()
{
  REG_HISTOGRAM
}


void MonitorStorageModule::beginRun()
{

}

void MonitorStorageModule::endRun()
{

}


void MonitorStorageModule::terminate()
{

}

//
// User defined functions
//


void MonitorStorageModule::event()
{
  m_h_runinfo->SetBinContent(1, g_data.expno);
  m_h_runinfo->SetBinContent(2, g_data.runno);
  m_h_runinfo->SetBinContent(3, g_data.subno);
  m_h_runinfo->SetBinContent(4, g_data.evtno);
  m_h_runinfo->SetBinContent(5, g_data.nevts);
  m_h_runinfo->SetBinContent(6, g_data.starttime);
  m_h_runinfo->SetBinContent(7, g_data.curtime);
  m_h_runinfo->SetBinContent(8, g_data.datasize);
  m_h_runinfo->SetBinContent(9, g_data.freq);
  m_h_runinfo->SetBinContent(10, g_data.rate);
}
