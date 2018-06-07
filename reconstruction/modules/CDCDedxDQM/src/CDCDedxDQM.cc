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


  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("CDCDedx")->cd();

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  fCurrentEventNum = eventMetaDataPtr->getRun();

  temp1D = new TH1F("hdEdx_PerRun", Form("hdEdx_PerRun%d", fCurrentEventNum), nBinsdedx, nBinsdedxLE, nBinsdedxUE);
  temp1D->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
  temp1D->GetYaxis()->SetTitle("Entries");

  temp2D = new TH2F("hdEdxVsP_PerRun", Form("hdEdxVsP_PerRun%d", fCurrentEventNum), nBinsP, nBinsPLE, nBinsPUE, nBinsdedx,
                    nBinsdedxLE, nBinsdedxUE);
  temp2D->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
  temp2D->GetYaxis()->SetTitle("dEdx");

  oldDir->cd();

}


//---------------------------------
void CDCDedxDQMModule::initialize()
{

  //hPerRunHisto = kFALSE;
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

  temp1D->Reset();
  temp2D->Reset();

}


//---------------------------------
void CDCDedxDQMModule::event()
{

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    //per run
    temp1D->Fill(float(dedxTrack->getDedx()));
    temp2D->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedx()));
  }

}


//---------------------------------
void CDCDedxDQMModule::endRun()
{

}


//---------------------------------
void CDCDedxDQMModule::terminate()
{

}
