/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>
#include <framework/core/HistoModule.h>
#include <TH1F.h>

using namespace Belle2;


REG_MODULE(CDCDedxDQM)


//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{

  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for dE/dx: means and resolutions for bhabha samples, band plots for lepton/hadron samples.");
  addParam("UsingHadronfiles", isHadronfile, "Switch to hadron (false) vs bhabha files", kFALSE);

}

//---------------------------------
CDCDedxDQMModule::~CDCDedxDQMModule() { }


//---------------------------------
void CDCDedxDQMModule::defineHisto()
{

  if (hPerRunHisto) {
    TH1F* temp1D = new TH1F(Form("hdEdx_Run%d", fCurrentEventNum), Form("Run Number = %d", fCurrentEventNum), nBinsdedx, nBinsdedxLE,
                            nBinsdedxUE);
    temp1D->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
    temp1D->GetYaxis()->SetTitle("Entries");
    temp1D->Reset();
    fOutput->Add(temp1D);

    TH2F* temp2D = new TH2F(Form("hdEdxVsP_Run%d", fCurrentEventNum), Form("Run Number = %d", fCurrentEventNum), nBinsP, nBinsPLE,
                            nBinsPUE, nBinsdedx, nBinsdedxLE, nBinsdedxUE);
    temp2D->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
    temp2D->GetYaxis()->SetTitle("dEdx");
    temp1D->Reset();
    fOutput->Add(temp2D);

    i1DHistoV.push_back(temp1D);
    i2DHistoV.push_back(temp2D);

  } else if (!hPerRunHisto) {

    TH1F* hdEdx_allRun = new TH1F("hdEdx_allRun", "dE/dx Distribution all Runs", nBinsdedx, nBinsdedxLE, nBinsdedxUE);
    hdEdx_allRun->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
    hdEdx_allRun->GetYaxis()->SetTitle("Entries");
    fOutput->Add(hdEdx_allRun);

    TH2F* hdEdxVsP_allRun = new TH2F("hdEdxVsP_allRun", "dE/dx vs P Distribution all Runs", nBinsP, nBinsPLE, nBinsPUE, nBinsdedx,
                                     nBinsdedxLE, nBinsdedxUE);
    hdEdxVsP_allRun->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
    hdEdxVsP_allRun->GetYaxis()->SetTitle("dEdx");
    fOutput->Add(hdEdxVsP_allRun);

  }

}


//---------------------------------
void CDCDedxDQMModule::initialize()
{

  fOutput = new TList();
  fOutput->SetOwner();
  fOutput->SetName("dEdxHistosPlusTrends");

  hPerRunHisto = kFALSE;

  nBinsdedx = 200; nBinsdedxLE = 0.; nBinsdedxUE = 4.;
  nBinsP = 500; nBinsPLE = 0.; nBinsPUE = 10.;
  fCollType = "bhabhaCand";

  if (isHadronfile) {
    nBinsdedx = 500; nBinsdedxLE = 0.; nBinsdedxUE = 10.;
    nBinsP = 150; nBinsPLE = 0.; nBinsPUE = 3.;
    fCollType = "charged";
  }

  m_cdcDedxTracks.isRequired();
  REG_HISTOGRAM

}



//---------------------------------
void CDCDedxDQMModule::beginRun()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  fCurrentEventNum = eventMetaDataPtr->getRun();
  hPerRunHisto = kTRUE;
  if (hPerRunHisto)defineHisto();

}


//---------------------------------
void CDCDedxDQMModule::event()
{

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    //per run
    ((TH1F*)(i1DHistoV.at(0)))->Fill(float(dedxTrack->getDedx()));
    ((TH2F*)(i2DHistoV.at(0)))->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedx()));
    //all runs
    ((TH1F*)fOutput->FindObject(Form("hdEdx_allRun")))->Fill(float(dedxTrack->getDedx()));
    ((TH1F*)fOutput->FindObject(Form("hdEdxVsP_allRun")))->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedx()));

  }

}

//---------------------------------
void CDCDedxDQMModule::endRun()
{

  i1DHistoV.pop_back();
  i2DHistoV.pop_back();

}



//---------------------------------
void CDCDedxDQMModule::terminate()
{

}
