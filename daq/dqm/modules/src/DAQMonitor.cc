//+
// File : DAQMonitor.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/dqm/modules/DAQMonitor.h>


using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DAQMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DAQMonitorModule::DAQMonitorModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Raw Data");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_nevt = -1;

}



DAQMonitorModule::~DAQMonitorModule()
{
}

void DAQMonitorModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("DAQ")->cd();

  h_nevt = new TH1F("Nevent", "Number of Events", 10, 0.0, 10.0);

  h_size = new TH1F("TotalDataSize", "Total Data Size", 100, 0.0, 300000.0);
  h_hltsize = new TH1F("HLTDataSize", "HLT Data Size", 100, 0.0, 300000.0);

  h_pxdsize = new TH1F("PXDDataSize", "PXD Data Size", 100, 0.0, 100000.0);
  h_svdsize = new TH1F("SVDDataSize", "SVD Data Size", 100, 0.0, 100000.0);
  h_cdcsize = new TH1F("CDCDataSize", "CDC Data Size", 100, 0.0, 100000.0);
  h_topsize = new TH1F("TOPDataSize", "TOP Data Size", 100, 0.0, 100000.0);
  h_arichsize = new TH1F("ARICHDataSize", "ARICH Data Size", 100, 0.0, 100000.0);
  h_eclsize = new TH1F("ECLDataSize", "ECL Data Size", 100, 0.0, 100000.0);
  h_klmsize = new TH1F("KLMDataSize", "KLM Data Size", 100, 0.0, 100000.0);
  h_trgsize = new TH1F("TRGDataSize", "TRG Data Size", 100, 0.0, 200000.0);

  oldDir->cd();
}


void DAQMonitorModule::initialize()
{
  REG_HISTOGRAM
}


void DAQMonitorModule::beginRun()
{
  B2INFO("beginRun called.");
}




void DAQMonitorModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void DAQMonitorModule::terminate()
{
  B2INFO("terminate called");
}



//
// User defined functions
//


void DAQMonitorModule::event()
{
  h_nevt->Fill((float)m_nevt);

  // PXD
  StoreArray<RawPXD> rawpxd;
  int npxd = rawpxd.getEntries();
  int pxdsize = 0;
  for (int i = 0; i < npxd; i++) {
    pxdsize += (rawpxd[i]->size()) * sizeof(unsigned int);
    //    printf ( "PXD size [%d] = %d bytes\n", i, size );
  }
  h_pxdsize->Fill((float)pxdsize);

  // SVD
  StoreArray<RawSVD> rawsvd;
  int nsvd = rawsvd.getEntries();
  int svdsize = 0;
  for (int i = 0; i < nsvd; i++) { // Loop over COPPERs
    int nbytes = rawsvd[i]->GetBlockNwords(0) * sizeof(unsigned int);
    svdsize += nbytes;
  }
  //  printf ( "SVD size = %d\n", svdsize );
  h_svdsize->Fill((float)svdsize);

  // CDC
  StoreArray<RawCDC> rawcdc;
  int ncdc = rawcdc.getEntries();
  int cdcsize = 0;
  for (int i = 0; i < ncdc; i++) { // Loop over COPPERs
    int nbytes = rawcdc[i]->GetBlockNwords(0) * sizeof(unsigned int);
    cdcsize += nbytes;
  }
  //  printf ( "CDC size = %d\n", cdcsize );
  h_cdcsize->Fill((float)cdcsize);

  // TOP
  StoreArray<RawTOP> rawtop;
  int ntop = rawtop.getEntries();
  int topsize = 0;
  for (int i = 0; i < ntop; i++) { // Loop over COPPERs
    int nbytes = rawtop[i]->GetBlockNwords(0) * sizeof(unsigned int);
    topsize += nbytes;
  }
  //  printf ( "TOP size = %d\n", topsize );
  h_topsize->Fill((float)topsize);

  // ARICH
  StoreArray<RawARICH> rawarich;
  int narich = rawarich.getEntries();
  int arichsize = 0;
  for (int i = 0; i < narich; i++) { // Loop over COPPERs
    int nbytes = rawarich[i]->GetBlockNwords(0) * sizeof(unsigned int);
    arichsize += nbytes;
  }
  //  printf ( "ARICH size = %d\n", arichsize );
  h_arichsize->Fill((float)arichsize);

  // ECL
  StoreArray<RawECL> rawecl;
  int necl = rawecl.getEntries();
  int eclsize = 0;
  for (int i = 0; i < necl; i++) { // Loop over COPPERs
    int nbytes = rawecl[i]->GetBlockNwords(0) * sizeof(unsigned int);
    eclsize += nbytes;
  }
  //  printf ( "ECL size = %d\n", eclsize );
  h_eclsize->Fill((float)eclsize);

  // KLM
  StoreArray<RawKLM> rawklm;
  int nklm = rawklm.getEntries();
  int klmsize = 0;
  for (int i = 0; i < nklm; i++) { // Loop over COPPERs
    int nbytes = rawklm[i]->GetBlockNwords(0) * sizeof(unsigned int);
    klmsize += nbytes;
  }
  //  printf ( "KLM size = %d\n", klmsize );
  h_klmsize->Fill((float)klmsize);

  // TRG
  StoreArray<RawTRG> rawtrg;
  int ntrg = rawtrg.getEntries();
  int trgsize = 0;
  for (int i = 0; i < ntrg; i++) { // Loop over COPPERs
    int nbytes = rawtrg[i]->GetBlockNwords(0) * sizeof(unsigned int);
    trgsize += nbytes;
  }
  //  printf ( "TRG size = %d\n", svdsize );
  h_trgsize->Fill((float)trgsize);

  // Total size
  int hltsize = svdsize + cdcsize + topsize + arichsize + eclsize + klmsize + trgsize;
  int totalsize = pxdsize + hltsize;
  //  printf ( "sizes = %d %d %d %d %d %d %d;  %d %d\n", svdsize, cdcsize, topsize, arichsize, eclsize, klmsize, trgsize, hltsize, totalsize );
  h_hltsize->Fill((float)hltsize);
  h_size->Fill((float)totalsize);


  m_nevt++;

}
