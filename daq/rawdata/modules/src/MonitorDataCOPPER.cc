/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rawdata/modules/MonitorDataCOPPER.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawECL.h>

#include <TDirectory.h>

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
  TDirectory* oldDir = gDirectory;

  TDirectory* dirDAQ = NULL;
  dirDAQ = oldDir->mkdir("DAQExample");
  dirDAQ->cd();
  h_nevt = new TH1F("h_nevt", "Number of Events / COPPER; COPPER ID; # of Events", 20, 0, 20);

  h_size = new TH1F("h_size", "Data size / COPPER; Data size [Byte]; entries", 50, 0, 10000);

  h_rate = new TH1F("h_rate", "Event rate; Time [s]; Event Rate [Hz]", 1000, 0, 100000);
  h_diff = new TH1F("h_diff", "Event inetrval; Event interval[s]; # of Events", 2000, 0, 0.2);
  oldDir->cd();

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

    m_loop = 0;
    m_nevt = 0;
  }


  //StoreArray<RawCOPPER> rawcprarray;
  //StoreArray<RawCDC> raw_dblkarray;
  //StoreArray<RawDataBlock> raw_dblkarray;
  StoreArray<RawECL> raw_dblkarray;

  int utime = 0;
  int ncpr = raw_dblkarray.getEntries();
  timeval prev_tv;
  prev_tv = m_tv;
  for (int j = 0; j < ncpr; j++) {
    m_nevt++;
    for (int i = 0; i < raw_dblkarray[j]->GetNumEntries(); i++) {
      RawCOPPER temp_rawcopper;
      temp_rawcopper.SetBuffer(raw_dblkarray[j]->GetBuffer(i),
                               raw_dblkarray[j]->GetBlockNwords(i), 0, 1, 1);

      if (j == 0 && i == 0) {
        temp_rawcopper.GetTTTimeVal(i, &m_tv);
        utime = temp_rawcopper.GetTTUtime(i);
        if (m_loop == 0) {
          m_start_time = (double)m_tv.tv_sec;
        }
      }
      int size_byte = raw_dblkarray[j]->GetBlockNwords(i) * sizeof(int);
      h_nevt->SetBinContent(j + 1, m_nevt);
      h_size->SetBinContent(j + 1, h_size->GetBinContent(i + 1) + size_byte);

    }
  }

  h_diff->Fill(m_tv.tv_sec - prev_tv.tv_sec + (m_tv.tv_usec - prev_tv.tv_usec) * 1.e-6);



  int tdiff_cur = m_tv.tv_sec - (int)m_start_time;

  if (m_loop % 1000 == 99) {
    //   printf("utime %d timeval %d %lf time %d\n", utime, m_tv.tv_sec, m_start_time,tdiff_cur );
  }
  if (tdiff_cur < 100000  && tdiff_cur > 0) {
    h_rate->SetBinContent(tdiff_cur / 100, h_rate->GetBinContent(tdiff_cur / 100) + 0.01);
  }


  m_loop++;



}
