/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/EventT0Validation/EventT0Validation.h>

#include <TEfficiency.h>

using namespace Belle2;

REG_MODULE(EventT0Validation);

//---------------------------------
EventT0ValidationModule::EventT0ValidationModule(): Module()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for EventT0 for bhabha, mu mu, and hadron samples for different trigger (time) sources.");
  addParam("RootFileName", m_RootFileName, "Name of the ROOT output file.", m_RootFileName);
}

//---------------------------------
EventT0ValidationModule::~EventT0ValidationModule() { }


//---------------------------------
void EventT0ValidationModule::initialize()
{
  m_outputFile = new TFile(m_RootFileName.c_str(), "RECREATE");
  if (m_outputFile != nullptr) {
    m_outputFile->cd();

    int nBins = 400 ;
    double minT0 = -100 ;
    double maxT0 =  100 ;

    m_histECLEventT0 = new TH1F("histECLEventT0", "ECL EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
    setPlotMetaData(m_histECLEventT0, "Distribution of ECL EventT0s.", "Should be centered around 0.", m_contact);
    m_histSVDEventT0 = new TH1F("histSVDEventT0", "SVD EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
    setPlotMetaData(m_histSVDEventT0, "Distribution of SVD EventT0s.", "Should be centered around 0.", m_contact);
    m_histTOPEventT0 = new TH1F("histTOPEventT0", "TOP EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
    setPlotMetaData(m_histTOPEventT0, "Distribution of TOP EventT0s.", "Should be centered around 0.", m_contact);
    m_histCDCEventT0 = new TH1F("histCDCEventT0", "CDC EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0, maxT0);
    setPlotMetaData(m_histCDCEventT0, "Distribution of CDC EventT0s.", "Should be centered around 0.", m_contact);
    m_histCDCHitBasedEventT0 = new TH1F("histCDCHitBasedEventT0", "CDC hit based EventT0;EventT0 [ns];events / 0.5 ns",
                                        nBins, minT0, maxT0);
    setPlotMetaData(m_histCDCHitBasedEventT0, "Distribution of CDC hit based EventT0s.", "Should be centered around 0.", m_contact);
    m_histCDCChi2EventT0 = new TH1F("histCDCChi2EventT0", "CDC FullGrid #chi^{2} EventT0;EventT0 [ns];events / 0.5 ns",
                                    nBins, minT0, maxT0);
    setPlotMetaData(m_histCDCChi2EventT0, "Distribution of CDC FullGrid #chi^{2} EventT0s.", "Should be centered around 0.",
                    m_contact, "");
    m_histCDCGridEventT0 = new TH1F("histCDCGridEventT0", "CDC Grid search EventT0;EventT0 [ns];events / 0.5 ns", nBins, minT0,
                                    maxT0);
    setPlotMetaData(m_histCDCGridEventT0, "Distribution of CDC Grid search EventT0s.", "Should be centered around 0.", m_contact, "");

    m_histAlgorithmSourceCounts =
      new TH1D("histAlgorithmSourceCounts",
               "Number of events with EventT0 from each algorithm;Algorithm;Count",
               11, 0, 11);
    setPlotMetaData(m_histAlgorithmSourceCounts,
                    "Number of events in which an EventT0 was found by each algorithm. "\
                    "Some of the CDC algorithms are only executed if no SVD EventT0 is found.",
                    "Values should be around 1000 (1000 events in validation).", m_contact);
    m_histAlgorithmSourceCountsActive =
      new TH1D("histAlgorithmSourceCountsActive",
               "Number of events with EventT0 from each algorithm where it was active;Algorithm;Count",
               11, 0, 11);
    setPlotMetaData(m_histAlgorithmSourceCountsActive,
                    "Number of events in which an EventT0 was found by each algorithm if they are executed. "\
                    "Some of the CDC algorithms are only executed if no SVD EventT0 is found.",
                    "Values should be around 1000 (1000 events in validation).", m_contact);

    for (uint i = 0; i < 11; i++) {
      m_histAlgorithmSourceCounts->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
      m_histAlgorithmSourceCountsActive->GetXaxis()->SetBinLabel(i + 1, c_eventT0Algorithms[i]);
    }
    m_histAlgorithmSourceCounts->GetXaxis()->CenterTitle(kTRUE);
    m_histAlgorithmSourceCounts->GetXaxis()->SetTitleOffset(1.2);
    m_histAlgorithmSourceCountsActive->GetXaxis()->CenterTitle(kTRUE);
    m_histAlgorithmSourceCountsActive->GetXaxis()->SetTitleOffset(1.2);
  }

  m_eventT0.isRequired();
}



//---------------------------------
void EventT0ValidationModule::beginRun()
{
  if (!m_eventT0.isValid()) {
    B2WARNING("Missing EventT0, EventT0Validation is skipped.");
    return;
  }
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

  const auto getCDCEventT0sForAlgorithm = [cdcEventT0s = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC)](
  const std::string & algorithm) {
    std::vector<EventT0::EventT0Component> temporaries;
    temporaries.reserve(cdcEventT0s.size());
    for (const auto& evtt0 : cdcEventT0s) {
      if (evtt0.algorithm == algorithm) {
        temporaries.push_back(evtt0);
      }
    }
    return temporaries;
  };

  m_histECLEventT0->Fill(eventT0ECL);
  m_histSVDEventT0->Fill(eventT0SVD);
  m_histTOPEventT0->Fill(eventT0TOP);
  m_histCDCEventT0->Fill(eventT0CDC);

  const auto hitBasedCDCT0 = getCDCEventT0sForAlgorithm("hit based");
  const auto chi2CDCT0 = getCDCEventT0sForAlgorithm("chi2");
  const auto gridCDCT0 = getCDCEventT0sForAlgorithm("grid");
  m_histCDCHitBasedEventT0->Fill(hitBasedCDCT0.empty() ? -1000 : hitBasedCDCT0.back().eventT0);
  m_histCDCChi2EventT0->Fill(chi2CDCT0.empty() ? -1000 : chi2CDCT0.back().eventT0);
  m_histCDCGridEventT0->Fill(gridCDCT0.empty() ? -1000 : gridCDCT0.back().eventT0);

  B2DEBUG(20, "eventT0ECL = " << eventT0ECL << " ns") ;
  B2DEBUG(20, "eventT0CDC = " << eventT0CDC << " ns") ;
  B2DEBUG(20, "eventT0TOP = " << eventT0TOP << " ns") ;
  B2DEBUG(20, "eventT0SVD = " << eventT0SVD << " ns") ;

  const bool hasECLEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL);
  const bool hasSVDEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::SVD);
  const bool hasTOPEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::TOP);
  const bool hasCDCEventT0 = m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC);
  const bool hasCDCHitBasedEventT0 = not hitBasedCDCT0.empty();
  const bool hasCDCFullGridChi2EventT0 = not chi2CDCT0.empty();
  const bool hasCDCGridEventT0 = not gridCDCT0.empty();

  m_histAlgorithmSourceCounts->Fill(-1);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[0], m_eventT0->hasEventT0());
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[1], hasECLEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[2], hasSVDEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[3], hasTOPEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[4], hasCDCEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[5], hasCDCHitBasedEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[6], hasCDCHitBasedEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[7], hasCDCFullGridChi2EventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[8], hasCDCFullGridChi2EventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[9], hasCDCGridEventT0);
  m_histAlgorithmSourceCounts->Fill(c_eventT0Algorithms[10], hasCDCGridEventT0);

  m_histAlgorithmSourceCountsActive->Fill(-1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[0], 1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[1], 1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[2], 1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[3], 1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[4], 1);
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[5], 1); // We always execute hit based search
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[6], 1); // We always execute hit based search
  // We only execute the chi2 algorithm if no SVD value is found, but this is the "all" column
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[7], 1);
  // We only execute the chi2 algorithm if no SVD value is found
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[8], not hasSVDEventT0);
  // We only execute the chi2 algorithm if no SVD value is found, but this is the "all" column
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[9], 1);
  // We only execute the grid algorithm if no SVD value is found
  m_histAlgorithmSourceCountsActive->Fill(c_eventT0Algorithms[10], not hasSVDEventT0);
}


void EventT0ValidationModule::endRun()
{
  if (m_outputFile != nullptr) {
    m_outputFile->cd();

    TEfficiency* algorithmEffi = new TEfficiency(*m_histAlgorithmSourceCounts, *m_histAlgorithmSourceCountsActive);
    algorithmEffi->SetTitle("Efficiency of finding an EventT0 per Algorithm");
    algorithmEffi->GetListOfFunctions()->Add(new TNamed("Contact", m_contact.c_str()));
    algorithmEffi->GetListOfFunctions()->Add(new TNamed("Description", "Efficiencies of the various EventT0 algorithms."));
    algorithmEffi->GetListOfFunctions()->Add(new TNamed("Check", "Efficiency should be close to 1 for all active algorithms."));
    algorithmEffi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
    algorithmEffi->Write("EventT0AlgorithmEfficiency");

    m_outputFile->Write();
    m_outputFile->Close();
  }
}


void EventT0ValidationModule::setPlotMetaData(TH1* hist, const std::string& description, const std::string& check,
                                              const std::string& contact, const std::string& shifter)
{
  hist->GetListOfFunctions()->Add(new TNamed("Contact", contact.c_str()));
  hist->GetListOfFunctions()->Add(new TNamed("Description", description.c_str()));
  hist->GetListOfFunctions()->Add(new TNamed("Check", check.c_str()));
  hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", shifter.c_str()));
}
