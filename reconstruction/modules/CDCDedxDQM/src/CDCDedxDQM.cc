/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jitendra Kumar, Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>
#include <framework/core/HistoModule.h>

using namespace Belle2;

REG_MODULE(CDCDedxDQM)

//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for dE/dx: means and resolutions for bhabha samples, band plots for lepton/hadron samples.");
  addParam("UsingHadronfiles", isHadronfile, "Switch to hadron (false) vs bhabha files", kFALSE);
  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select bhabha events in this module", std::string("software_trigger_cut&skim&accept_bhabha"));
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

  temp1D = new TH1D("hdEdx_PerRun", Form("hdEdx_PerRun%d", fCurrentEventNum), nBinsdedx, nBinsdedxLE, nBinsdedxUE);
  temp1D->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
  temp1D->GetYaxis()->SetTitle("Entries");

  temp2D = new TH2D("hdEdxVsP_PerRun", Form("hdEdxVsP_PerRun%d", fCurrentEventNum), nBinsP, nBinsPLE, nBinsPUE, nBinsdedx,
                    nBinsdedxLE, nBinsdedxUE);
  temp2D->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
  temp2D->GetYaxis()->SetTitle("dEdx");

  oldDir->cd();

}


//---------------------------------
void CDCDedxDQMModule::initialize()
{

  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }
  //hPerRunHisto = kFALSE;
  nBinsdedx = 200; nBinsdedxLE = 0.; nBinsdedxUE = 2.;
  nBinsP = 500; nBinsPLE = 0.05; nBinsPUE = 10.;
  fCollType = "bhabha skim";

  if (isHadronfile) {
    nBinsdedx = 750; nBinsdedxLE = 0.; nBinsdedxUE = 15.;
    nBinsP = 500; nBinsPLE = 0.05; nBinsPUE = 10.0;
    fCollType = "hadron skim";
  }

  m_TrgResult.isOptional();
  m_cdcDedxTracks.isRequired();
  REG_HISTOGRAM

}



//---------------------------------
void CDCDedxDQMModule::beginRun()
{
  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }

  temp1D->Reset();
  temp2D->Reset();

}


//---------------------------------
void CDCDedxDQMModule::event()
{

  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha events for this module");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if (fresults.find(m_triggerIdentifier) == fresults.end()) {
    B2WARNING("CDCDedxDQMModule: Can't find required trigger identifier: " << m_triggerIdentifier);
    return;
  }

  const bool IsEvtAccepted = (m_TrgResult->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
  if (IsEvtAccepted == false) return;

  // fill histograms for bhabha-events only
  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    //per run
    temp1D->Fill(float(dedxTrack->getDedxNoSat()));
    temp2D->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedxNoSat()));
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
