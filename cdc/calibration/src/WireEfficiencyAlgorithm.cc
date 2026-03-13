/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/WireEfficiencyAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dbobjects/CDCBadWires.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <TH2F.h>
#include <TFitResult.h>
#include <TH1F.h>
#include <TF1.h>
#include <TGraphAsymmErrors.h>
#include <TMath.h>
#include <math.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;
WireEfficiencyAlgorithm::WireEfficiencyAlgorithm(): CalibrationAlgorithm("CDCBadWireCollector")
{
  setDescription(
    " -------------------------- Wire Efficiency Estimation Algorithm -------------------------\n"
  );
}

bool WireEfficiencyAlgorithm::buildEfficiencies()
{
  unsigned short wireID;
  unsigned short layerID;
  float z;
  bool isFound;

  auto efftree = getObjectPtr<TTree>("efftree");
  efftree->SetBranchAddress("wireID", &wireID);
  efftree->SetBranchAddress("layerID", &layerID);
  efftree->SetBranchAddress("z", &z);
  efftree->SetBranchAddress("isFound", &isFound);

  static const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  static const CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    unsigned short layerNo = wireLayer.getICLayer();

    unsigned short nzbins = 30 - layerNo / 7;
    unsigned short nwidbins = cdcgeo.nWiresInLayer(layerNo);
    double widbins[2] = { -0.5, (double)cdcgeo.nWiresInLayer(layerNo) - 0.5};
    double zbins[2] = {wireLayer.getBackwardZ(), wireLayer.getForwardZ()};

    TEfficiency* effInLayer = new TEfficiency(Form("effLayer_%d", layerNo),
                                              Form("Hit efficiency pf L%d ; z (cm) ; IWire ", layerNo),
                                              nzbins, zbins[0], zbins[1], nwidbins, widbins[0], widbins[1]);
    m_efficiencyList->Add(effInLayer);
  }

  const Long64_t nEntries = efftree->GetEntries();
  for (Long64_t i = 0; i < nEntries; i++) {
    efftree->GetEntry(i);
    if (layerID < m_efficiencyList->GetEntries()) {
      TEfficiency* efficiencyInLayer = (TEfficiency*)m_efficiencyList->At(layerID);
      efficiencyInLayer->Fill(isFound, z, wireID);
    }
  }

  double totalCountsForActiveWires = 0;
  long long activeWireCount = 0;

  for (int i = 0; i < m_efficiencyList->GetEntries(); ++i) {
    TEfficiency* eff = (TEfficiency*)m_efficiencyList->At(i);
    // GetTotalHistogram contains both 'passed' and 'failed' attempts (total trials)
    TH2* hTotal = (TH2*)eff->GetTotalHistogram();

    // Loop through Y-bins (Wire IDs)
    for (int yBin = 1; yBin <= hTotal->GetNbinsY(); ++yBin) {
      // Sum all Z-bins (X-axis) for this specific wire (Y-bin)
      // Integral(firstX, lastX, firstY, lastY). 0 and -1 include under/overflow.
      double wireTotalEntries = hTotal->Integral(0, -1, yBin, yBin);

      if (wireTotalEntries > 0) {
        totalCountsForActiveWires += wireTotalEntries;
        activeWireCount++;
      }
    }
  }

  double averageOccupancy = (activeWireCount > 0) ? (totalCountsForActiveWires / activeWireCount) : 0;
  B2INFO("Average number of hits per wire in this run: " << averageOccupancy);

  TFile* outputCollection = new TFile("LayerEfficiencies.root", "RECREATE");
  m_efficiencyList->Write();
  outputCollection->Close();
  delete outputCollection;

  // Return true if average hits per non-empty wire > 20000
  return (averageOccupancy > 20000);
}

void WireEfficiencyAlgorithm::detectBadWires()
{
  static const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    unsigned short layerNo = wireLayer.getICLayer();

    // need to use casting here because GetPassedHistogram assumes it returns TH1
    auto efficiencyInLayer = (TEfficiency*)m_efficiencyList->At(layerNo);
    auto passed = (TH2F*)efficiencyInLayer->GetPassedHistogram();
    auto total = (TH2F*)efficiencyInLayer->GetTotalHistogram();

    // Ignoring layers that have no hits at all
    if (!total->GetEntries()) continue;

    // prepare fitting functions
    double minFitRange = wireLayer.getBackwardZ() + 30;
    double maxFitRange = wireLayer.getForwardZ() - 30;
    TF1* constantFunction = new TF1("constantFunction", "pol0", minFitRange, maxFitRange);

    // Estimate average efficiency of the layer as function of z
    auto passedProjectedX = passed->ProjectionX("projectionx1");
    auto totalProjectedX = total->ProjectionX("projectionx2");
    TEfficiency* efficiencyProjectedX = new TEfficiency(*passedProjectedX, *totalProjectedX);

    unsigned short minFitBin = passedProjectedX->FindBin(minFitRange) + 1;
    unsigned short maxFitBin = passedProjectedX->FindBin(maxFitRange) - 1;

    // Estimate average efficiency of each wire in the layer
    auto passedProjectedY = passed->ProjectionY("projectiony1", minFitRange, maxFitRange);
    auto totalProjectedY = total->ProjectionY("projectiony2", minFitRange, maxFitRange);
    TEfficiency* efficiencyProjectedY = new TEfficiency(*passedProjectedY, *totalProjectedY);

    // Estimate average efficiency of the whole layer by summing up averages of individual wires
    float totalAverage = 0;
    int nonZeroWires = 0;
    for (int i = 0; i <= passedProjectedY->GetNbinsX(); ++i) {
      float efficiencyAtBin = efficiencyProjectedY->GetEfficiency(i);
      if (efficiencyAtBin > 0.2) {
        totalAverage += efficiencyAtBin;
        nonZeroWires++;
      }
    }
    // safeguard in case all wires are 0
    nonZeroWires > 0 ? totalAverage /= nonZeroWires : totalAverage == 0;

    TGraphAsymmErrors* graphEfficiencyProjected = efficiencyProjectedX->CreateGraph();

    // Due to the way histograms are binned, every bin center should correspond a wire ID.
    for (int i = 0; i <= passed->GetNbinsY(); ++i) {

      // project out 1 wire
      auto singleWirePassed = passed->ProjectionX("single wire projection passed", i, i);
      auto singleWireTotal = total->ProjectionX("single wire projection total", i, i);

      // if not a single hit within the central area of wire then mark it as bad
      if (!singleWirePassed->Integral(minFitBin, maxFitBin)) {
        double wireID = passed->GetYaxis()->GetBinCenter(i);
        unsigned short maxWires = CDCGeometryPar::Instance().nWiresInLayer(layerNo);
        if (wireID < 0 || wireID >= maxWires) {
          B2ERROR("Invalid wireID: " << wireID << " for LayerID: " << layerNo
                  << ". Max wires: " << maxWires);
          continue; // remove invalid wireID
        }
        m_badWireList->setWire(layerNo, round(wireID), 0);
        continue;
      }
      // constructs efficiency of one wire
      TEfficiency* singleWireEfficiency = new TEfficiency(*singleWirePassed, *singleWireTotal);
      TGraphAsymmErrors* graphSingleWireEfficiency = singleWireEfficiency->CreateGraph();

      // fits the efficiency with a constant
      TFitResultPtr singleWireFitResults = graphSingleWireEfficiency->Fit(constantFunction, "SQR");
      double singleWireEfficiencyFromFit = (singleWireFitResults.Get())->Value(0);
      double singleWireUpErrorFromFit = (singleWireFitResults.Get())->UpperError(0);

      // applies a chi test on the wire
      float p_value = chiTest(graphSingleWireEfficiency, graphEfficiencyProjected, minFitRange, maxFitRange);
      // check three conditions and decide if wire should be marked as bad
      bool averageCondition = (0.95 * totalAverage) > (singleWireEfficiencyFromFit + 3 * singleWireUpErrorFromFit);
      bool pvalueCondition = p_value < 0.01;
      bool generalCondition = singleWireEfficiencyFromFit < 0.4;
      if (generalCondition || (averageCondition && pvalueCondition)) {
        double wireID = passed->GetYaxis()->GetBinCenter(i);
        m_badWireList->setWire(layerNo, round(wireID), singleWireEfficiencyFromFit);
      }
    }
  }
  m_badWireList->outputToFile("wireFile.txt");
  saveCalibration(m_badWireList, "CDCBadWires");
}

double WireEfficiencyAlgorithm::chiTest(TGraphAsymmErrors* graph1, TGraphAsymmErrors* graph2, double minValue, double maxValue)
{

  // Vars to perform the chi test
  double chi = 0;
  unsigned short ndof = 0;


  int numOfEntries1 = graph1->GetN();
  int numOfEntries2 = graph2->GetN();

  // loop over entries in both graphs, index by index.

  for (int index1 = 0; index1 < numOfEntries1; ++index1) {
    // TGraph values are usually not listed in increasing order. Need to check that they are within min/max range for comparison.
    if (graph1->GetX()[index1] < minValue) continue;
    if (graph1->GetX()[index1] > maxValue) continue;
    for (int index2 = 0; index2 < numOfEntries2; ++index2) {
      if (graph1->GetX()[index1] == graph2->GetX()[index2]) {
        // this is broken up just for readability
        double chiNumerator = pow(graph1->GetY()[index1] - graph2->GetY()[index2], 2);
        double chiDenominator = pow(graph1->GetErrorYhigh(index1), 2) + pow(graph1->GetErrorYlow(index1), 2);
        chi += chiNumerator / chiDenominator;
        ndof++;
        continue;
      }
    }
  }
  return TMath::Prob(chi, ndof);
}

CalibrationAlgorithm::EResult WireEfficiencyAlgorithm::calibrate()
{
  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);

  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  bool enoughEventsInInput;
  enoughEventsInInput = buildEfficiencies();

  detectBadWires();

  if (enoughEventsInInput) return c_OK;
  else return c_NotEnoughData;
}
