/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/EventT0DQM/EventT0DQM.h>
#include <framework/core/HistoModule.h>

using namespace Belle2;

REG_MODULE(EventT0DQM);

//---------------------------------
EventT0DQMModule::EventT0DQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for EventT0 for bhabha, mu mu, and hadron samples for different trigger (time) sources.");
}

//---------------------------------
EventT0DQMModule::~EventT0DQMModule() { }


//---------------------------------
void EventT0DQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("EventT0DQMdir")->cd();

  int nBins = 400 ;
  double minT0 = -100 ;
  double maxT0 =  100 ;

  m_histEventT0_ECL_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_ECL_bhabha_L1_ECLTRG",
                                                "ECL EventT0, L1TRG from ECL, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_CDC_bhabha_L1_ECLTRG",
                                                "CDC EventT0, L1TRG from ECL, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_TOP_bhabha_L1_ECLTRG",
                                                "TOP EventT0, L1TRG from ECL, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_SVD_bhabha_L1_ECLTRG",
                                                "SVD EventT0, L1TRG from ECL, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_hadron_L1_ECLTRG = new TH1F("m_histEventT0_ECL_hadron_L1_ECLTRG",
                                                "ECL EventT0, L1TRG from ECL, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_hadron_L1_ECLTRG = new TH1F("m_histEventT0_CDC_hadron_L1_ECLTRG",
                                                "CDC EventT0, L1TRG from ECL, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_hadron_L1_ECLTRG = new TH1F("m_histEventT0_TOP_hadron_L1_ECLTRG",
                                                "TOP EventT0, L1TRG from ECL, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_hadron_L1_ECLTRG = new TH1F("m_histEventT0_SVD_hadron_L1_ECLTRG",
                                                "SVD EventT0, L1TRG from ECL, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_ECL_mumu_L1_ECLTRG",
                                                "ECL EventT0, L1TRG from ECL, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_CDC_mumu_L1_ECLTRG",
                                                "CDC EventT0, L1TRG from ECL, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_TOP_mumu_L1_ECLTRG",
                                                "TOP EventT0, L1TRG from ECL, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_SVD_mumu_L1_ECLTRG",
                                                "SVD EventT0, L1TRG from ECL, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);


  m_histEventT0_ECL_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_ECL_bhabha_L1_CDCTRG",
                                                "ECL EventT0, L1TRG from CDC, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_CDC_bhabha_L1_CDCTRG",
                                                "CDC EventT0, L1TRG from CDC, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_TOP_bhabha_L1_CDCTRG",
                                                "TOP EventT0, L1TRG from CDC, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_SVD_bhabha_L1_CDCTRG",
                                                "SVD EventT0, L1TRG from CDC, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_hadron_L1_CDCTRG = new TH1F("m_histEventT0_ECL_hadron_L1_CDCTRG",
                                                "ECL EventT0, L1TRG from CDC, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_hadron_L1_CDCTRG = new TH1F("m_histEventT0_CDC_hadron_L1_CDCTRG",
                                                "CDC EventT0, L1TRG from CDC, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_hadron_L1_CDCTRG = new TH1F("m_histEventT0_TOP_hadron_L1_CDCTRG",
                                                "TOP EventT0, L1TRG from CDC, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_hadron_L1_CDCTRG = new TH1F("m_histEventT0_SVD_hadron_L1_CDCTRG",
                                                "SVD EventT0, L1TRG from CDC, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_ECL_mumu_L1_CDCTRG",
                                                "ECL EventT0, L1TRG from CDC, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_CDC_mumu_L1_CDCTRG",
                                                "CDC EventT0, L1TRG from CDC, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_TOP_mumu_L1_CDCTRG",
                                                "TOP EventT0, L1TRG from CDC, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_SVD_mumu_L1_CDCTRG",
                                                "SVD EventT0, L1TRG from CDC, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);


  m_histEventT0_ECL_bhabha_L1_TOPTRG = new TH1F("m_histEventT0_ECL_bhabha_L1_TOPTRG",
                                                "ECL EventT0, L1TRG from TOP, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_bhabha_L1_TOPTRG = new TH1F("m_histEventT0_CDC_bhabha_L1_TOPTRG",
                                                "CDC EventT0, L1TRG from TOP, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_bhabha_L1_TOPTRG = new TH1F("m_histEventT0_TOP_bhabha_L1_TOPTRG",
                                                "TOP EventT0, L1TRG from TOP, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_bhabha_L1_TOPTRG = new TH1F("m_histEventT0_SVD_bhabha_L1_TOPTRG",
                                                "SVD EventT0, L1TRG from TOP, HLT Bhabha;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_hadron_L1_TOPTRG = new TH1F("m_histEventT0_ECL_hadron_L1_TOPTRG",
                                                "ECL EventT0, L1TRG from TOP, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_hadron_L1_TOPTRG = new TH1F("m_histEventT0_CDC_hadron_L1_TOPTRG",
                                                "CDC EventT0, L1TRG from TOP, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_hadron_L1_TOPTRG = new TH1F("m_histEventT0_TOP_hadron_L1_TOPTRG",
                                                "TOP EventT0, L1TRG from TOP, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_hadron_L1_TOPTRG = new TH1F("m_histEventT0_SVD_hadron_L1_TOPTRG",
                                                "SVD EventT0, L1TRG from TOP, HLT Hadron;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);

  m_histEventT0_ECL_mumu_L1_TOPTRG   = new TH1F("m_histEventT0_ECL_mumu_L1_TOPTRG",
                                                "ECL EventT0, L1TRG from TOP, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_mumu_L1_TOPTRG   = new TH1F("m_histEventT0_CDC_mumu_L1_TOPTRG",
                                                "CDC EventT0, L1TRG from TOP, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_mumu_L1_TOPTRG   = new TH1F("m_histEventT0_TOP_mumu_L1_TOPTRG",
                                                "TOP EventT0, L1TRG from TOP, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_SVD_mumu_L1_TOPTRG   = new TH1F("m_histEventT0_SVD_mumu_L1_TOPTRG",
                                                "SVD EventT0, L1TRG from TOP, HLT mumu;EventT0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);


  oldDir->cd();

}


//---------------------------------
void EventT0DQMModule::initialize()
{

  m_TrgResult.isOptional();
  m_eventT0.isOptional();

  REG_HISTOGRAM

}



//---------------------------------
void EventT0DQMModule::beginRun()
{
  if (!m_eventT0.isValid()) {
    B2WARNING("Missing EventT0, EventT0DQM is skipped.");
    return;
  }

  if (!m_TrgResult.isValid()) {
    B2WARNING("Missing SoftwareTriggerResult, EventT0DQM is skipped.");
    return;
  }

  m_histEventT0_ECL_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_CDC_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_TOP_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_SVD_bhabha_L1_ECLTRG->Reset();

  m_histEventT0_ECL_hadron_L1_ECLTRG->Reset();
  m_histEventT0_CDC_hadron_L1_ECLTRG->Reset();
  m_histEventT0_TOP_hadron_L1_ECLTRG->Reset();
  m_histEventT0_SVD_hadron_L1_ECLTRG->Reset();

  m_histEventT0_ECL_mumu_L1_ECLTRG->Reset();
  m_histEventT0_CDC_mumu_L1_ECLTRG->Reset();
  m_histEventT0_TOP_mumu_L1_ECLTRG->Reset();
  m_histEventT0_SVD_mumu_L1_ECLTRG->Reset();

  m_histEventT0_ECL_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_CDC_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_TOP_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_SVD_bhabha_L1_CDCTRG->Reset();

  m_histEventT0_ECL_hadron_L1_CDCTRG->Reset();
  m_histEventT0_CDC_hadron_L1_CDCTRG->Reset();
  m_histEventT0_TOP_hadron_L1_CDCTRG->Reset();
  m_histEventT0_SVD_hadron_L1_CDCTRG->Reset();

  m_histEventT0_ECL_mumu_L1_CDCTRG->Reset();
  m_histEventT0_CDC_mumu_L1_CDCTRG->Reset();
  m_histEventT0_TOP_mumu_L1_CDCTRG->Reset();
  m_histEventT0_SVD_mumu_L1_CDCTRG->Reset();

  m_histEventT0_ECL_bhabha_L1_TOPTRG->Reset();
  m_histEventT0_CDC_bhabha_L1_TOPTRG->Reset();
  m_histEventT0_TOP_bhabha_L1_TOPTRG->Reset();
  m_histEventT0_SVD_bhabha_L1_TOPTRG->Reset();

  m_histEventT0_ECL_hadron_L1_TOPTRG->Reset();
  m_histEventT0_CDC_hadron_L1_TOPTRG->Reset();
  m_histEventT0_TOP_hadron_L1_TOPTRG->Reset();
  m_histEventT0_SVD_hadron_L1_TOPTRG->Reset();

  m_histEventT0_ECL_mumu_L1_TOPTRG->Reset();
  m_histEventT0_CDC_mumu_L1_TOPTRG->Reset();
  m_histEventT0_TOP_mumu_L1_TOPTRG->Reset();
  m_histEventT0_SVD_mumu_L1_TOPTRG->Reset();

}


//---------------------------------
void EventT0DQMModule::event()
{
  if (!m_objTrgSummary.isValid()) {
    B2WARNING("TRGSummary object not available but required to indicate which detector provided the L1 trigger time");
    return;
  }

  // Skip this event if there is no event t0, to avoid crashing other DQM
  if (!m_eventT0.isOptional()) {
    B2WARNING("Missing EventT0, EventT0DQM is skipped.");
    return;
  }

  // Determine if there is a valid event t0 to use and then extract the information about it
  if (!m_eventT0.isValid()) {
    return ;
  }

  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha/mumu/hadron events for this module");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if ((fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_mumu_2trk") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())) {
    B2WARNING("EventT0DQMModule: Can't find required bhabha or mumu or hadron trigger identifier");
    return;
  }

  // for L1 timing source is "ecl trigger" -> TRGSummary::ETimingType is 0
  const bool IsECLL1TriggerSource = (m_objTrgSummary->getTimType() == TRGSummary::ETimingType::TTYP_ECL);
  // for L1 timing source is "top trigger" -> TRGSummary::ETimingType is 1
  const bool IsTOPL1TriggerSource = (m_objTrgSummary->getTimType() == TRGSummary::ETimingType::TTYP_TOP);
  // for L1 timing source is "cdc trigger" -> TRGSummary::ETimingType is 3
  const bool IsCDCL1TriggerSource = (m_objTrgSummary->getTimType() == TRGSummary::ETimingType::TTYP_CDC);

  B2DEBUG(20, "IsECLL1TriggerSource = " << IsECLL1TriggerSource);
  B2DEBUG(20, "IsTOPL1TriggerSource = " << IsTOPL1TriggerSource);
  B2DEBUG(20, "IsCDCL1TriggerSource = " << IsCDCL1TriggerSource);

  // determine if the event was part of the hadron skim or bhabha skim or mumu skim
  const bool IsEvtAcceptedBhabha = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsEvtAcceptedHadron = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsEvtAcceptedMumu = (m_TrgResult->getResult("software_trigger_cut&skim&accept_mumu_2trk") ==
                                  SoftwareTriggerCutResult::c_accept);


  B2DEBUG(20, "bhabha trigger result = " << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha"))) ;
  B2DEBUG(20, "hadron trigger result = " << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron"))) ;
  B2DEBUG(20, "mu mu trigger result = "  << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_mumu_2trk"))) ;
  B2DEBUG(20, "bhabha trigger comparison bool = " << IsEvtAcceptedBhabha) ;
  B2DEBUG(20, "hadron trigger comparison bool = " << IsEvtAcceptedHadron) ;
  B2DEBUG(20, "mumu trigger comparison bool = "   << IsEvtAcceptedMumu) ;


  // Set the different EventT0 values, default is -1000 in case there are no information based on a given detector
  const double eventT0ECL =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL) ? m_eventT0->getBestECLTemporaryEventT0()->eventT0 : -1000;
  const double eventT0CDC =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC) ? m_eventT0->getBestCDCTemporaryEventT0()->eventT0 : -1000;
  const double eventT0TOP =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP) ? m_eventT0->getBestTOPTemporaryEventT0()->eventT0 : -1000;
  const double eventT0SVD =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD) ? m_eventT0->getBestSVDTemporaryEventT0()->eventT0 : -1000;

  // Fill the plots that used the ECL trigger as the L1 timing source
  if (IsECLL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_ECLTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_bhabha_L1_ECLTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_bhabha_L1_ECLTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_bhabha_L1_ECLTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_ECLTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_hadron_L1_ECLTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_hadron_L1_ECLTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_hadron_L1_ECLTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_ECLTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_mumu_L1_ECLTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_mumu_L1_ECLTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_mumu_L1_ECLTRG->Fill(eventT0SVD);
    }
  }
  // Fill the plots that used the TOP trigger as the L1 timing source
  else if (IsTOPL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_TOPTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_bhabha_L1_TOPTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_bhabha_L1_TOPTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_bhabha_L1_TOPTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_TOPTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_hadron_L1_TOPTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_hadron_L1_TOPTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_hadron_L1_TOPTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_TOPTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_mumu_L1_TOPTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_mumu_L1_TOPTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_mumu_L1_TOPTRG->Fill(eventT0SVD);
    }
  }
  // Fill the plots that used the CDC trigger as the L1 timing source
  else if (IsCDCL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_CDCTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_bhabha_L1_CDCTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_bhabha_L1_CDCTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_bhabha_L1_CDCTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_CDCTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_hadron_L1_CDCTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_hadron_L1_CDCTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_hadron_L1_CDCTRG->Fill(eventT0SVD);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_CDCTRG->Fill(eventT0ECL);
      m_histEventT0_CDC_mumu_L1_CDCTRG->Fill(eventT0CDC);
      m_histEventT0_TOP_mumu_L1_CDCTRG->Fill(eventT0TOP);
      m_histEventT0_SVD_mumu_L1_CDCTRG->Fill(eventT0SVD);
    }
  }

  B2DEBUG(20, "eventT0ECL = " << eventT0ECL << " ns") ;
  B2DEBUG(20, "eventT0CDC = " << eventT0CDC << " ns") ;
  B2DEBUG(20, "eventT0TOP = " << eventT0TOP << " ns") ;
  B2DEBUG(20, "eventT0SVD = " << eventT0SVD << " ns") ;
}
