//+
// File : GenAnaModule.cc
// Description : Module to show particle info
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Feb - 2015
//-

#include <daq/dqm/modules/TrackAna.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackAna)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackAnaModule::TrackAnaModule() : HistoModule()
{
  //Set module properties
  setDescription("The simplest physics analysis");
  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition
  B2INFO("TrackAna: Constructor done.");
}


TrackAnaModule::~TrackAnaModule()
{
}

void TrackAnaModule::initialize()
{
  REG_HISTOGRAM
}

void TrackAnaModule::defineHisto()
{
  h_multi = new TH1F("Multi", "Particle Multiplicity", 50, 0.0, 50.0);
  h_p[0] = new TH1F("Px", "Particle Momentum X", 100, -5.0, 5.0);
  h_p[1] = new TH1F("Py", "Particle Momentum Y", 100, -5.0, 5.0);
  h_p[2] = new TH1F("Pz", "Particle Momentum Z", 100, -5.0, 5.0);
  h_p[3] = new TH1F("E", "Particle Energy", 100, 0.0, 10.0);
}

void TrackAnaModule::beginRun()
{
  B2INFO("TrackAna: started to measure elapsed time.");
}


void TrackAnaModule::event()
{
  // Get Event Info
  StoreObjPtr<EventMetaData> evt;
  int expno = evt->getExperiment();
  int runno = evt->getRun();
  int subrno = evt->getSubrun();
  int evtno = evt->getEvent();

  // Get List of Tracks
  StoreArray<Track> trklist;

  int ntrk = trklist.getEntries();
  h_multi->Fill((float)ntrk);
  //  printf ( "***** Generator Event List : Event %d ; multi = %d *****\n", evtno, npart );
  for (int i = 0; i < ntrk; i++) {
    Track* trk = trklist[i];
    const TrackFitResult* fit = trk->getTrackFitResult(Const::pion);
    TLorentzVector p4 = fit->get4Momentum();
    for (int j = 0; j < 4; j++) {
      h_p[j]->Fill(p4[j]);
    }
  }
}

void TrackAnaModule::endRun()
{
}


void TrackAnaModule::terminate()
{
  B2INFO("TrackAna: terminate called");
}
