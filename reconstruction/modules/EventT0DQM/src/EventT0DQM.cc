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
  oldDir->mkdir("EventT0")->cd();

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

  m_histAlgorithmSourceFractionsHadronL1ECLTRG =
    new TH1D("AlgorithmSourceFractionsHadronL1ECLTRG",
             "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by ECL;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsHadronL1CDCTRG =
    new TH1D("AlgorithmSourceFractionsHadronL1CDCTRG",
             "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by CDC;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsHadronL1TOPTRG =
    new TH1D("AlgorithmSourceFractionsHadronL1TOPTRG",
             "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by TOP;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsBhaBhaL1ECLTRG =
    new TH1D("AlgorithmSourceFractionsBhaBhaL1ECLTRG",
             "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by ECL;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsBhaBhaL1CDCTRG =
    new TH1D("AlgorithmSourceFractionsBhaBhaL1CDCTRG",
             "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by CDC;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsBhaBhaL1TOPTRG =
    new TH1D("AlgorithmSourceFractionsBhaBhaL1TOPTRG",
             "Fraction of events with EventT0 from each algorithm for Bhabha events triggerd by TOP;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsMuMuL1ECLTRG =
    new TH1D("AlgorithmSourceFractionsMuMuL1ECLTRG",
             "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by ECL;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsMuMuL1CDCTRG =
    new TH1D("AlgorithmSourceFractionsMuMuL1CDCTRG",
             "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by CDC;Algorithm;Fraction",
             5, 0, 5);
  m_histAlgorithmSourceFractionsMuMuL1TOPTRG =
    new TH1D("AlgorithmSourceFractionsMuMuL1TOPTRG",
             "Fraction of events with EventT0 from each algorithm for #mu#mu events triggerd by TOP;Algorithm;Fraction",
             5, 0, 5);

  for (uint i = 0; i < 5; i++) {
    m_histAlgorithmSourceFractionsHadronL1ECLTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsHadronL1CDCTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsHadronL1TOPTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsBhaBhaL1ECLTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsBhaBhaL1CDCTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsBhaBhaL1TOPTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsMuMuL1ECLTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsMuMuL1CDCTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    m_histAlgorithmSourceFractionsMuMuL1TOPTRG->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
  }

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

  m_histAlgorithmSourceFractionsHadronL1ECLTRG->Reset();
  m_histAlgorithmSourceFractionsHadronL1CDCTRG->Reset();
  m_histAlgorithmSourceFractionsHadronL1TOPTRG->Reset();
  m_histAlgorithmSourceFractionsBhaBhaL1ECLTRG->Reset();
  m_histAlgorithmSourceFractionsBhaBhaL1CDCTRG->Reset();
  m_histAlgorithmSourceFractionsBhaBhaL1TOPTRG->Reset();
  m_histAlgorithmSourceFractionsMuMuL1ECLTRG->Reset();
  m_histAlgorithmSourceFractionsMuMuL1CDCTRG->Reset();
  m_histAlgorithmSourceFractionsMuMuL1TOPTRG->Reset();

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


  // default values of the event t0 given that there may not be a value for every event depending on the detector measuring it.
  double eventT0_ECL = -1000 ;
  double eventT0_CDC = -1000 ;
  double eventT0_TOP = -1000 ;
  double eventT0_SVD = -1000 ;

  // Set the CDC event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    auto evtT0List_CDC = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC) ;

    // set the CDC event t0 value for filling into the histogram
    //    The most accurate CDC event t0 value is the last one in the list.
    eventT0_CDC = evtT0List_CDC.back().eventT0 ;
  }

  // Set the ECL event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL)) {
    // Get the list of ECL event t0 values.  There are several event t0 values, not just one.
    auto evtT0List_ECL = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL) ;

    auto eclBestT0 = std::min_element(evtT0List_ECL.begin(), evtT0List_ECL.end(), [](EventT0::EventT0Component c1,
    EventT0::EventT0Component c2) {return c1.quality < c2.quality;});

    // set the ECL event t0 value for filling into the histogram
    //    It is the value found to have the small chi square
    eventT0_ECL = eclBestT0->eventT0 ;
  }

  // Set the TOP event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP)) {
    auto evtT0List_TOP = m_eventT0->getTemporaryEventT0s(Const::EDetector::TOP) ;

    // set the TOP event t0 value for filling into the histogram
    //    There should only be at most one value in the list per event
    eventT0_TOP = evtT0List_TOP.back().eventT0 ;
  }

  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD)) {
    auto evtT0List_SVD = m_eventT0->getTemporaryEventT0s(Const::EDetector::SVD) ;
    //    There is only one estimate of SVD EVentT0 for the moment
    eventT0_SVD = evtT0List_SVD.back().eventT0 ;
  }

  const auto checkForCDCAlgorithm = [cdcEventT0s = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC)](
  const std::string & algorithm) {
    for (const auto& evtt0 : cdcEventT0s) {
      if (evtt0.algorithm == algorithm) {
        return true;
      }
    }
    return false;
  };

  const bool hasCDCHitBasedEventT0 = checkForCDCAlgorithm("hit based");
  const bool hasCDCFullGridEventT0 = checkForCDCAlgorithm("chi2");
  const bool hasECLEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL);
  const bool hasSVDEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD);
  const bool hasTOPEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP);

  // Fill the plots that used the ECL trigger as the L1 timing source
  if (IsECLL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_bhabha_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_bhabha_L1_ECLTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_bhabha_L1_ECLTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsBhaBhaL1ECLTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_hadron_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_hadron_L1_ECLTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_hadron_L1_ECLTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsHadronL1ECLTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_mumu_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_mumu_L1_ECLTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_mumu_L1_ECLTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsMuMuL1ECLTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }
  }
  // Fill the plots that used the TOP trigger as the L1 timing source
  else if (IsTOPL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_TOPTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_bhabha_L1_TOPTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_bhabha_L1_TOPTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_bhabha_L1_TOPTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsBhaBhaL1TOPTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_TOPTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_hadron_L1_TOPTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_hadron_L1_TOPTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_hadron_L1_TOPTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsHadronL1TOPTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_TOPTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_mumu_L1_TOPTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_mumu_L1_TOPTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_mumu_L1_TOPTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsMuMuL1TOPTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }
  }
  // Fill the plots that used the CDC trigger as the L1 timing source
  else if (IsCDCL1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_bhabha_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_bhabha_L1_CDCTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_bhabha_L1_CDCTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsBhaBhaL1CDCTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_hadron_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_hadron_L1_CDCTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_hadron_L1_CDCTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsHadronL1CDCTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_mumu_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_mumu_L1_CDCTRG->Fill(eventT0_TOP);
      m_histEventT0_SVD_mumu_L1_CDCTRG->Fill(eventT0_SVD);
      fillHistogram(m_histAlgorithmSourceFractionsMuMuL1CDCTRG, hasECLEventT0, hasSVDEventT0, hasCDCHitBasedEventT0,
                    hasCDCFullGridEventT0, hasTOPEventT0);
    }
  }

  B2DEBUG(20, "eventT0_ECL = " << eventT0_ECL << " ns") ;
  B2DEBUG(20, "eventT0_CDC = " << eventT0_CDC << " ns") ;
  B2DEBUG(20, "eventT0_TOP = " << eventT0_TOP << " ns") ;
  B2DEBUG(20, "eventT0_SVD = " << eventT0_SVD << " ns") ;
}

void EventT0DQMModule::fillHistogram(TH1D* hist, const bool hasECLT0, const bool hasSVDT0, const bool hasCDCHitT0,
                                     const bool hasCDCGridT0, const bool hasTOPT0)
{
  hist->Fill(-1); // counting events for normalisation
  hist->Fill(c_eventT0Algorithms[0], hasECLT0);
  hist->Fill(c_eventT0Algorithms[1], hasSVDT0);
  hist->Fill(c_eventT0Algorithms[2], hasCDCHitT0);
  hist->Fill(c_eventT0Algorithms[3], hasCDCGridT0);
  hist->Fill(c_eventT0Algorithms[4], hasTOPT0);
}
