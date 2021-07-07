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

REG_MODULE(EventT0DQM)

//---------------------------------
EventT0DQMModule::EventT0DQMModule(): HistoModule(), m_L1TimingSrc(0)
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for event t0 for bhabha, mu mu, and hadron samples seeded by different trigger times.");
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
                                                "ECL event t0 - bhabhas - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_CDC_bhabha_L1_ECLTRG",
                                                "CDC event t0 - bhabhas - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_bhabha_L1_ECLTRG = new TH1F("m_histEventT0_TOP_bhabha_L1_ECLTRG",
                                                "TOP event t0 - bhabhas - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_ECL_hadron_L1_ECLTRG = new TH1F("m_histEventT0_ECL_hadron_L1_ECLTRG",
                                                "ECL event t0 - hadrons - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_hadron_L1_ECLTRG = new TH1F("m_histEventT0_CDC_hadron_L1_ECLTRG",
                                                "CDC event t0 - hadrons - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_hadron_L1_ECLTRG = new TH1F("m_histEventT0_TOP_hadron_L1_ECLTRG",
                                                "TOP event t0 - hadrons - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_ECL_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_ECL_mumu_L1_ECLTRG",
                                                "ECL event t0 - mu mu - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_CDC_mumu_L1_ECLTRG",
                                                "CDC event t0 - mu mu - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_mumu_L1_ECLTRG   = new TH1F("m_histEventT0_TOP_mumu_L1_ECLTRG",
                                                "TOP event t0 - mu mu - ECLTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);





  m_histEventT0_ECL_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_ECL_bhabha_L1_CDCTRG",
                                                "ECL event t0 - bhabhas - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_CDC_bhabha_L1_CDCTRG",
                                                "CDC event t0 - bhabhas - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_bhabha_L1_CDCTRG = new TH1F("m_histEventT0_TOP_bhabha_L1_CDCTRG",
                                                "TOP event t0 - bhabhas - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_ECL_hadron_L1_CDCTRG = new TH1F("m_histEventT0_ECL_hadron_L1_CDCTRG",
                                                "ECL event t0 - hadrons - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_hadron_L1_CDCTRG = new TH1F("m_histEventT0_CDC_hadron_L1_CDCTRG",
                                                "CDC event t0 - hadrons - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_hadron_L1_CDCTRG = new TH1F("m_histEventT0_TOP_hadron_L1_CDCTRG",
                                                "TOP event t0 - hadrons - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_ECL_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_ECL_mumu_L1_CDCTRG",
                                                "ECL event t0 - mu mu - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_CDC_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_CDC_mumu_L1_CDCTRG",
                                                "CDC event t0 - mu mu - CDCTRG time;event t0 [ns];events / 0.5 ns",
                                                nBins, minT0, maxT0);
  m_histEventT0_TOP_mumu_L1_CDCTRG   = new TH1F("m_histEventT0_TOP_mumu_L1_CDCTRG",
                                                "TOP event t0 - mu mu - CDCTRG time;event t0 [ns];events / 0.5 ns",
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
    B2WARNING("Missing event t0, EventT0DQM is skipped.");
    return;
  }

  if (!m_TrgResult.isValid()) {
    B2WARNING("Missing SoftwareTriggerResult, EventT0DQM is skipped.");
    return;
  }

  m_histEventT0_ECL_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_CDC_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_TOP_bhabha_L1_ECLTRG->Reset();
  m_histEventT0_ECL_hadron_L1_ECLTRG->Reset();
  m_histEventT0_CDC_hadron_L1_ECLTRG->Reset();
  m_histEventT0_TOP_hadron_L1_ECLTRG->Reset();
  m_histEventT0_ECL_mumu_L1_ECLTRG->Reset();
  m_histEventT0_CDC_mumu_L1_ECLTRG->Reset();
  m_histEventT0_TOP_mumu_L1_ECLTRG->Reset();

  m_histEventT0_ECL_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_CDC_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_TOP_bhabha_L1_CDCTRG->Reset();
  m_histEventT0_ECL_hadron_L1_CDCTRG->Reset();
  m_histEventT0_CDC_hadron_L1_CDCTRG->Reset();
  m_histEventT0_TOP_hadron_L1_CDCTRG->Reset();
  m_histEventT0_ECL_mumu_L1_CDCTRG->Reset();
  m_histEventT0_CDC_mumu_L1_CDCTRG->Reset();
  m_histEventT0_TOP_mumu_L1_CDCTRG->Reset();

}


//---------------------------------
void EventT0DQMModule::event()
{
  if (!m_objTrgSummary.isValid()) {
    B2WARNING("TRGSummary object not available but required to indicate which detector provided the L1 trigger time");
    return;
  } else {
    m_L1TimingSrc = m_objTrgSummary->getTimType();
  }

  bool Is_ECL_L1TriggerSource = false ;
  bool Is_CDC_L1TriggerSource = false ;
  if (m_L1TimingSrc == 0) {            // for L1 timing source is "ecl trigger"
    Is_ECL_L1TriggerSource = true ;
  } else if (m_L1TimingSrc == 3) {     // for L1 timing source is "cdc trigger"
    Is_CDC_L1TriggerSource = true ;
  }
  // else if(m_L1TimingSrc==5){  // for L1 timing source is "delayed Bhabha" }
  B2DEBUG(20, "Is_ECL_L1TriggerSource = " << Is_ECL_L1TriggerSource) ;
  B2DEBUG(20, "Is_CDC_L1TriggerSource= " << Is_CDC_L1TriggerSource) ;


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



  // Skip this event if there is no event t0, to avoid crashing other DQM
  if (!m_eventT0.isOptional()) {
    B2WARNING("Missing event t0, EventT0DQM is skipped.");
    return;
  }

  // Determine if there is a valid event t0 to use and then extract the information about it
  if (!m_eventT0.isValid()) {
    return ;
  }

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

    // Select the event t0 value from the ECL as the one with the smallest chi squared value (defined as ".quality")
    double smallest_ECL_t0_minChi2 = evtT0List_ECL[0].quality ;
    int smallest_ECL_t0_minChi2_idx = 0 ;
    for (long unsigned int ECLi = 0; ECLi < evtT0List_ECL.size(); ECLi++) {
      if (evtT0List_ECL[ECLi].quality < smallest_ECL_t0_minChi2) {
        smallest_ECL_t0_minChi2 = evtT0List_ECL[ECLi].quality ;
        smallest_ECL_t0_minChi2_idx = ECLi ;
      }
    }
    // set the ECL event t0 value for filling into the histogram
    //    It is the value found to have the small chi square
    eventT0_ECL = evtT0List_ECL[smallest_ECL_t0_minChi2_idx].eventT0 ;
  }

  // Set the TOP event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP)) {
    auto evtT0List_TOP = m_eventT0->getTemporaryEventT0s(Const::EDetector::TOP) ;

    // set the TOP event t0 value for filling into the histogram
    //    There should only be at most one value in the list per event
    eventT0_TOP = evtT0List_TOP.back().eventT0 ;
  }




  // Fill the plots that used the ECL trigger as the L1 timing source
  if (Is_ECL_L1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_bhabha_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_bhabha_L1_ECLTRG->Fill(eventT0_TOP);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_hadron_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_hadron_L1_ECLTRG->Fill(eventT0_TOP);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_ECLTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_mumu_L1_ECLTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_mumu_L1_ECLTRG->Fill(eventT0_TOP);
    }
  }
  // Fill the plots that used the CDC trigger as the L1 timing source
  else if (Is_CDC_L1TriggerSource) {
    // Fill the histograms with the event t0 values
    if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
      m_histEventT0_ECL_bhabha_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_bhabha_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_bhabha_L1_CDCTRG->Fill(eventT0_TOP);
    }

    if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
      m_histEventT0_ECL_hadron_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_hadron_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_hadron_L1_CDCTRG->Fill(eventT0_TOP);
    }

    if (IsEvtAcceptedMumu) {    // fill the mumu skim event t0s
      m_histEventT0_ECL_mumu_L1_CDCTRG->Fill(eventT0_ECL);
      m_histEventT0_CDC_mumu_L1_CDCTRG->Fill(eventT0_CDC);
      m_histEventT0_TOP_mumu_L1_CDCTRG->Fill(eventT0_TOP);
    }
  }

  B2DEBUG(20, "eventT0_ECL = " << eventT0_ECL << " ns") ;
  B2DEBUG(20, "eventT0_CDC = " << eventT0_CDC << " ns") ;
  B2DEBUG(20, "eventT0_TOP = " << eventT0_TOP << " ns") ;
}


//---------------------------------
void EventT0DQMModule::endRun()
{

}


//---------------------------------
void EventT0DQMModule::terminate()
{

}
