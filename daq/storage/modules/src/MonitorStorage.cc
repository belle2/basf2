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

int MonitorStorageModule::g_expno = 0;
int MonitorStorageModule::g_runno = 0;
int MonitorStorageModule::g_subno = 0;
int MonitorStorageModule::g_evtno = 0;
int MonitorStorageModule::g_nevts = 0;
double MonitorStorageModule::g_starttime = 0;
double MonitorStorageModule::g_datasize = 0;
double MonitorStorageModule::g_curtime = 0;
double MonitorStorageModule::g_freq = 0;
double MonitorStorageModule::g_rate = 0;

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
  m_h_runinfo->SetBinContent(1, g_expno);
  m_h_runinfo->SetBinContent(2, g_runno);
  m_h_runinfo->SetBinContent(3, g_subno);
  m_h_runinfo->SetBinContent(4, g_evtno);
  m_h_runinfo->SetBinContent(5, g_nevts);
  m_h_runinfo->SetBinContent(6, g_starttime);
  m_h_runinfo->SetBinContent(7, g_curtime);
  m_h_runinfo->SetBinContent(8, g_datasize);
  m_h_runinfo->SetBinContent(9, g_freq);
  m_h_runinfo->SetBinContent(10, g_rate);
}
