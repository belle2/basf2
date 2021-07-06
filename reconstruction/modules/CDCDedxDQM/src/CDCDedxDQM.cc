/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>
#include <framework/core/HistoModule.h>

#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(CDCDedxDQM)

//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for dE/dx: means and resolutions for bhabha skim, dedx band plots for bhabha/hadron skim.");
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

  temp1D = new TH1D("hdEdx_PerRun", Form("hdEdx_PerRun%d", fCurrentEventNum), 300, 0., 3.);
  temp1D->GetXaxis()->SetTitle("dEdx trucMean of bhabha tracks");
  temp1D->GetYaxis()->SetTitle("Entries");

  temp2D = new TH2D("hdEdxVsP_PerRun", Form("hdEdxVsP_PerRun%d", fCurrentEventNum), 400, 0.02, 8.0, 500, 0.10, 15.0);
  temp2D->GetXaxis()->SetTitle("p (GeV) of hadron tracks");
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
    B2WARNING("SoftwareTriggerResult object not available but require to select skim events for this module: going back");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if (fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end()
      and fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end()) {
    //B2WARNING("CDCDedxDQMModule: Can't find required trigger identifiers: going back ");
    return;
  }

  const bool IsBhabhaEvtAccepted = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsHadronEvtAccepted = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                                    SoftwareTriggerCutResult::c_accept);
  if (!IsBhabhaEvtAccepted and !IsHadronEvtAccepted) {
    //B2WARNING("CDCDedxDQMModule: not an bhabha or hadron event: going back");
    return;
  }
  // fill histograms for bhabha-events only
  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    //per run
    if (IsBhabhaEvtAccepted)temp1D->Fill(float(dedxTrack->getDedxNoSat()));
    if (IsHadronEvtAccepted)temp2D->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedxNoSat()));
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
