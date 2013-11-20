//+
// File : HistoAna.cc
// Description : Example to fill histograms with HistoManager
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 14 - Dec - 2012
//-

#include <daq/rfarm/run/modules/HistoAna/HistoAna.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HistoAna)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HistoAnaModule::HistoAnaModule() : HistoModule()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_ParallelProcessingCertified);
}

HistoAnaModule::~HistoAnaModule()
{
}

void HistoAnaModule::defineHisto()
{
  h_multi = new TH1F("Multi", "Multi", 100, 0.0, 100.0);
  h_mom[0] = new TH1F("Px", "Px", 100, -1.5, 1.5);
  h_mom[1] = new TH1F("Py", "Py", 100, -1.5, 1.5);
  h_mom[2] = new TH1F("Pz", "Pz", 100, -1.5, 1.5);
  h_mom[3] = new TH1F("E", "E", 100, 0.0, 10.0);
}

void HistoAnaModule::initialize()
{
  RbTupleManager::Instance().register_module(this);
}

void HistoAnaModule::event()
{
  StoreArray<MCParticle> mcarray;
  int nprt = mcarray.getEntries();

  h_multi->Fill((float)nprt);

  for (int i = 0; i < nprt; i++) {
    TVector3 mom = mcarray[i]->getMomentum();
    h_mom[0]->Fill(mom.X());
    h_mom[1]->Fill(mom.Y());
    h_mom[2]->Fill(mom.Z());
    float e = mcarray[i]->getEnergy();
    h_mom[3]->Fill(e);
  }
}

