#include <daq/slc/apps/dqmviewd/modules/gaus/DAQDummyGaus.h>

#include <TDirectory.h>
#include <TRandom3.h>
#include <unistd.h>

using namespace Belle2;

REG_MODULE(DAQDummyGaus)

TRandom3 g_rand;

DAQDummyGausModule::DAQDummyGausModule() : HistoModule()
{
  setDescription("DQM histogram example with Gaus random");
  setPropertyFlags(c_ParallelProcessingCertified);
}



DAQDummyGausModule::~DAQDummyGausModule()
{
}

void DAQDummyGausModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  TDirectory* dirDAQ = NULL;
  dirDAQ = oldDir->mkdir("DAQ");
  dirDAQ->cd();
  for (int i = 0; i < 5; i++) {
    h_Gaus[i] = new TH1F(Form("h_Gaus_%d", i), Form("Gaus-%d; Value [a.u.];# of entries", i), 100, 0, 100);
  }
  h_GausSum = new TH1F("h_GausSum", "Gaus sum; Value [a.u.];# of entries", 100, 0, 100);
  h_Gaus2D = new TH2F("h_Gaus2D", "Gaus sum; Value [a.u.]; Value [a.u.]", 100, 0, 100, 100, 0, 100);
  oldDir->cd();
}


void DAQDummyGausModule::initialize()
{
  REG_HISTOGRAM
}


void DAQDummyGausModule::beginRun()
{
}

void DAQDummyGausModule::endRun()
{
}


void DAQDummyGausModule::terminate()
{
}

void DAQDummyGausModule::event()
{
  h_Gaus[0]->Fill(g_rand.Gaus(30, 20));
  h_Gaus[1]->Fill(g_rand.Gaus(40, 20));
  h_Gaus[2]->Fill(g_rand.Gaus(50, 20));
  h_Gaus[3]->Fill(g_rand.Gaus(60, 20));
  h_Gaus[4]->Fill(g_rand.Gaus(80, 20));
  h_GausSum->Fill(g_rand.Gaus(30, 20));
  h_GausSum->Fill(g_rand.Gaus(60, 20));
  h_GausSum->Fill(g_rand.Gaus(80, 20));
  h_Gaus2D->Fill(g_rand.Gaus(50, 20), g_rand.Gaus(50, 20));

  usleep(100);
}
