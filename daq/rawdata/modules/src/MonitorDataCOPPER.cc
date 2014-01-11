//+
// File : MonitorDataCOPPER.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/MonitorDataCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>

#include <cstdio>

using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MonitorDataCOPPER)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MonitorDataCOPPERModule::MonitorDataCOPPERModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Raw Data");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_loop = -1;

}



MonitorDataCOPPERModule::~MonitorDataCOPPERModule()
{
}

void MonitorDataCOPPERModule::defineHisto()
{

//   double t_min = 0.;
//   double t_max = 600.;
//   int t_nbin = 600;

  h_nevt = new TH1F("h_nevt", "Number of Events / COPPER; COPPER ID; # of Events", 20, 0, 20);

  h_size = new TH1F("h_size", "Data size / COPPER; Data size [Byte]; entries", 50, 0, 10000);

  h_rate = new TH1F("h_rate", "Data rate / COPPER; COPPER ID; Data rate [Bytes/s]", 20, 0, 20);


  /*
  h_hslb_size[0] = new TH1F("h_hslb_size_0", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
  h_hslb_size[1] = new TH1F("h_hslb_size_1", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
  h_hslb_size[2] = new TH1F("h_hslb_size_2", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
  h_hslb_size[3] = new TH1F("h_hslb_size_3", "Data size / HSLB; Data size [Byte]; entries", 50, 0, 10000);
  h_hslb_nevt = new TH1F("h_hslb_nevt", "Number of Events / HSLB; HSLB slot; # of Events", 4, 0, 4);
  h_hslb_rate = new TH1F("h_hslb_rate", "Data rate / HSLB; HSLB slot; Data rate [Bytes/s]", 4, 0, 4);
  */
}


void MonitorDataCOPPERModule::initialize()
{
  REG_HISTOGRAM
}


void MonitorDataCOPPERModule::beginRun()
{

}




void MonitorDataCOPPERModule::endRun()
{
  //fill Run data


}


void MonitorDataCOPPERModule::terminate()
{

}

double MonitorDataCOPPERModule::getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}



//
// User defined functions
//


void MonitorDataCOPPERModule::event()
{
  if (m_loop <= 0) {
    m_start_time = getTimeSec();
    m_loop = 0;
    m_nevt = 0;
  }


  //StoreArray<RawCOPPER> rawcprarray;
  //StoreArray<RawCDC> raw_dblkarray;
  //StoreArray<RawDataBlock> raw_dblkarray;
  StoreArray<RawSVD> raw_dblkarray;

  int ncpr = raw_dblkarray.getEntries();
  for (int j = 0; j < ncpr; j++) {
    m_nevt++;
    for (int i = 0; i < raw_dblkarray[j]->GetNumEntries(); i++) {
      RawCOPPER* temp_rawcopper = new RawCOPPER;
      temp_rawcopper->SetBuffer(raw_dblkarray[j]->GetBuffer(i),
                                raw_dblkarray[j]->GetBlockNwords(i), 0, 1, 1);
      int size_byte = raw_dblkarray[j]->GetBlockNwords(i) * sizeof(int);
      //h_ncpr->Fill(i);
      h_nevt->SetBinContent(i + 1, m_nevt);
      h_size->SetBinContent(i + 1, h_size->GetBinContent(i + 1) + size_byte);
    }
  }

  if (m_loop % 100 == 99) {
    double cur_time = getTimeSec();
    double tdiff_cur = cur_time - m_start_time;
    double tdiff_prev = m_prev_time - m_start_time;
    double rate = (m_nevt - m_prev_nevt) / (tdiff_cur - tdiff_prev);
    if (m_nevt - m_prev_nevt != 0) {
      h_rate->Fill(rate);
    }
    m_prev_nevt = m_nevt;
    m_prev_time = cur_time;
  }
  m_loop++;
}
