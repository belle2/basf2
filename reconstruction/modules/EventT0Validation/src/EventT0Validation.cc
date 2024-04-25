/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/EventT0Validation/EventT0Validation.h>

using namespace Belle2;

REG_MODULE(EventT0Validation);

//---------------------------------
EventT0ValidationModule::EventT0ValidationModule(): Module()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for EventT0 for bhabha, mu mu, and hadron samples for different trigger (time) sources.");
}

//---------------------------------
EventT0ValidationModule::~EventT0ValidationModule() { }


//---------------------------------
void EventT0ValidationModule::initialize()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("EventT0")->cd();

  int nBins = 400 ;
  double minT0 = -100 ;
  double maxT0 =  100 ;

  m_histECLEventT0 = new TH1F("m_histECLEventT0", "ECL EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
  m_histSVDEventT0 = new TH1F("m_histSVDEventT0", "SVD EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
  m_histTOPEventT0 = new TH1F("m_histTOPEventT0", "TOP EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
  m_histCDCEventT0 = new TH1F("m_histCDCEventT0", "CDC EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
  m_histCDCHitBasedEventT0 = new TH1F("m_histCDCHitBasedEventT0", "CDC hit based EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0,
                                      maxT0);
  m_histCDCChi2EventT0 = new TH1F("m_histCDCChi2EventT0", "CDC FullGrid #Chi^2 EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0,
                                  maxT0);
  m_histCDCGridEventT0 = new TH1F("m_histCDCGridEventT0", "CDC Grid EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);

  m_histAlgorithmSourceFractions =
    new TH1D("m_histAlgorithmSourceFractions",
             "Fraction of events with EventT0 from each algorithm for hadronic events triggerd by ECL;Algorithm;Fraction",
             10, 0, 10);

  for (uint i = 0; i < 10; i++) {
    m_histAlgorithmSourceFractions->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
  }

  oldDir->cd();

  m_eventT0.isRequired();
}



//---------------------------------
void EventT0ValidationModule::beginRun()
{
  if (!m_eventT0.isValid()) {
    B2WARNING("Missing EventT0, EventT0Validation is skipped.");
    return;
  }

  m_histECLEventT0->Reset();
  m_histSVDEventT0->Reset();
  m_histTOPEventT0->Reset();
  m_histCDCEventT0->Reset();
  m_histCDCHitBasedEventT0->Reset();
  m_histCDCChi2EventT0->Reset();
  m_histCDCGridEventT0->Reset();

  m_histAlgorithmSourceFractions->Reset();

}


//---------------------------------
void EventT0ValidationModule::event()
{
  // Determine if there is a valid event t0 to use and then extract the information about it
  if (!m_eventT0.isValid()) {
    B2WARNING("Missing EventT0, EventT0Validation is skipped.");
    return ;
  }


  // Set the different EventT0 values, default is -1000 in case there are no information based on a given detector
  const double eventT0ECL =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL) ? m_eventT0->getBestECLTemporaryEventT0()->eventT0 : -1000;
  const double eventT0CDC =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC) ? m_eventT0->getBestCDCTemporaryEventT0()->eventT0 : -1000;
  const double eventT0TOP =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP) ? m_eventT0->getBestTOPTemporaryEventT0()->eventT0 : -1000;
  const double eventT0SVD =
    m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD) ? m_eventT0->getBestSVDTemporaryEventT0()->eventT0 : -1000;

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
  const bool hasCDCFullGridChi2EventT0 = checkForCDCAlgorithm("chi2");
  const bool hasCDCGridEventT0 = checkForCDCAlgorithm("grid");
  const bool hasECLEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL);
  const bool hasSVDEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD);
  const bool hasTOPEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP);

  m_histECLEventT0->Fill(eventT0ECL);
  m_histSVDEventT0->Fill(eventT0SVD);
  m_histTOPEventT0->Fill(eventT0TOP);
  m_histCDCEventT0->Fill(eventT0CDC);

  B2DEBUG(20, "eventT0ECL = " << eventT0ECL << " ns") ;
  B2DEBUG(20, "eventT0CDC = " << eventT0CDC << " ns") ;
  B2DEBUG(20, "eventT0TOP = " << eventT0TOP << " ns") ;
  B2DEBUG(20, "eventT0SVD = " << eventT0SVD << " ns") ;
}
