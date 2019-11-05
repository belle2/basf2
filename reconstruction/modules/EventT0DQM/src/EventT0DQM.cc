/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill (ehill@mail.ubc.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/EventT0DQM/EventT0DQM.h>
#include <framework/core/HistoModule.h>

using namespace Belle2;

REG_MODULE(EventT0DQM)

//---------------------------------
EventT0DQMModule::EventT0DQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for event t0 for bhabha and hadron samples.");
}

//---------------------------------
EventT0DQMModule::~EventT0DQMModule() { }


//---------------------------------
void EventT0DQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("EventT0DQMdir")->cd();

  int nBins = 160 ;
  double minT0 = -40 ;
  double maxT0 =  40 ;
  h_eventT0_ECL_bhabha = new TH1F("h_eventT0_ECL_bhabha", "ECL event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  h_eventT0_CDC_bhabha = new TH1F("h_eventT0_CDC_bhabha", "CDC event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  h_eventT0_TOP_bhabha = new TH1F("h_eventT0_TOP_bhabha", "TOP event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  h_eventT0_ECL_hadron = new TH1F("h_eventT0_ECL_hadron", "ECL event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  h_eventT0_CDC_hadron = new TH1F("h_eventT0_CDC_hadron", "CDC event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  h_eventT0_TOP_hadron = new TH1F("h_eventT0_TOP_hadron", "TOP event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);

  oldDir->cd();

}


//---------------------------------
void EventT0DQMModule::initialize()
{

  if (!m_eventT0.isOptional()) {
    B2WARNING("Missing event t0 module, EventT0DQM is skipped.");
    return;
  }

  m_TrgResult.isOptional();
  m_eventT0.isRequired();
  REG_HISTOGRAM

}



//---------------------------------
void EventT0DQMModule::beginRun()
{
  if (!m_eventT0.isOptional()) {
    B2WARNING("Missing event t0, EventT0DQM is skipped.");
    return;
  }

  h_eventT0_ECL_bhabha->Reset();
  h_eventT0_CDC_bhabha->Reset();
  h_eventT0_TOP_bhabha->Reset();
  h_eventT0_ECL_hadron->Reset();
  h_eventT0_CDC_hadron->Reset();
  h_eventT0_TOP_hadron->Reset();

}


//---------------------------------
void EventT0DQMModule::event()
{

  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha/hadron events for this module");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if ((fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())) {
    B2WARNING("EventT0DQMModule: Can't find required bhabha or hadron trigger identifier");
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

  // determine if the event was part of the hadron skim or bhabha skim
  const bool IsEvtAcceptedBhabha = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsEvtAcceptedHadron = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                                    SoftwareTriggerCutResult::c_accept);

  // default values of the event t0 given that there may not be a value for every event depending on the detector measuring it.
  double event_t0_ECL = -1000 ;
  double event_t0_CDC = -1000 ;
  double event_t0_TOP = -1000 ;

  // Set the CDC event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    vector<EventT0::EventT0Component> evt_t0_list_CDC = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC) ;

    // set the CDC event t0 value for filling into the histogram
    //    The most accurate CDC event t0 value is the last one in the list.
    event_t0_CDC = evt_t0_list_CDC.back().eventT0 ;
  }

  // Set the ECL event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL)) {
    // Get the list of ECL event t0 values.  There are several event t0 values, not just one.
    vector<EventT0::EventT0Component> evt_t0_list_ECL = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL) ;

    // Select the event t0 value from the ECL as the one with the smallest chi squared value (defined as ".quality")
    double smallest_ECL_t0_minChi2 = evt_t0_list_ECL[0].quality ;
    int smallest_ECL_t0_minChi2_idx = 0 ;
    for (int ECLi = 0; ECLi < evt_t0_list_ECL.size(); ECLi++) {
      if (evt_t0_list_ECL[ECLi].quality < smallest_ECL_t0_minChi2) {
        smallest_ECL_t0_minChi2 = evt_t0_list_ECL[ECLi].quality ;
        smallest_ECL_t0_minChi2_idx = ECLi ;
      }
    }
    // set the ECL event t0 value for filling into the histogram
    //    It is the value found to have the small chi square
    event_t0_ECL = evt_t0_list_ECL[smallest_ECL_t0_minChi2_idx].eventT0 ;
  }

  // Set the TOP event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    vector<EventT0::EventT0Component> evt_t0_list_TOP = m_eventT0->getTemporaryEventT0s(Const::EDetector::TOP) ;

    // set the TOP event t0 value for filling into the histogram
    //    There should only be at most one value in the list per event
    event_t0_TOP = evt_t0_list_TOP.back().eventT0 ;
  }



  // Fill the histograms with the event t0 values
  if (IsEvtAcceptedBhabha) {   // fill the bha bha skim event t0s
    h_eventT0_ECL_bhabha->Fill(event_t0_ECL);
    h_eventT0_CDC_bhabha->Fill(event_t0_CDC);
    h_eventT0_TOP_bhabha->Fill(event_t0_TOP);
  }

  if (IsEvtAcceptedHadron) {    // fill the hadron skim event t0s
    h_eventT0_ECL_hadron->Fill(event_t0_ECL);
    h_eventT0_CDC_hadron->Fill(event_t0_CDC);
    h_eventT0_TOP_hadron->Fill(event_t0_TOP);
  }


}


//---------------------------------
void EventT0DQMModule::endRun()
{

}


//---------------------------------
void EventT0DQMModule::terminate()
{

}
