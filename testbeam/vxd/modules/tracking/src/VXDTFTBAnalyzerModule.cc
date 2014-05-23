/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "testbeam/vxd/modules/tracking/VXDTFTBAnalyzerModule.h"
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <genfit/TrackCand.h>
#include <tracking/vxdCaTracking/LittleHelper.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDTFTBAnalyzer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDTFTBAnalyzerModule::VXDTFTBAnalyzerModule() : Module()
{
  //Set module properties
  setDescription("compares two different containers of genfit::TrackCand");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("stepSize", m_stepSize, "e.g. 100 will highlight every 100th event", int(100));
  addParam("nameContainerTCevaluate", m_PARAMnameContainerTCevaluate, "set here the first name of the containers to be compared (this one is the test-container)", string("caTracksJKL"));
  addParam("nameContainerTCreference", m_PARAMnameContainerTCreference, "set here the second name of the containers to be compared (this one is used as reference for evaluating the first one)", string("bestCandidates"));

}


VXDTFTBAnalyzerModule::~VXDTFTBAnalyzerModule()
{

}


void VXDTFTBAnalyzerModule::initialize()
{
  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::optional();
  StoreArray<genfit::TrackCand>::required(m_PARAMnameContainerTCreference);
  StoreArray<genfit::TrackCand>::required(m_PARAMnameContainerTCevaluate);

}


void VXDTFTBAnalyzerModule::beginRun()
{
  resetCounters();
  B2INFO("################## VXDTFTBAnalyzerModule enabled, highlighting every " << m_stepSize << " event ######################");
}


void VXDTFTBAnalyzerModule::event()
{
  m_eventCounter++;

  StoreArray<PXDCluster> aPxdClusterArray;
  m_pxdClusterCounter += aPxdClusterArray.getEntries();

  StoreArray<SVDCluster> aSvdClusterArray;
  m_svdClusterCounter += aSvdClusterArray.getEntries();

  StoreArray<genfit::TrackCand> tcsReference(m_PARAMnameContainerTCreference);
  int nReferenceTCs = tcsReference.getEntries();
  bool foundReferenceTCs = false;
  if (nReferenceTCs != 0) { foundReferenceTCs = true; }

  StoreArray<genfit::TrackCand> tcsEvaluate(m_PARAMnameContainerTCevaluate);
  int nEvaluateTCs = tcsEvaluate.getEntries();
  bool foundEvaluatingTCs = false;
  if (nEvaluateTCs != 0) { foundEvaluatingTCs = true; }


  //collecting info about number of cluster-combinations (only guess since it will not be checked whether they are u or v clusters):
  vector<SensorAndCluster> clustersPerSensor, clusterCombinationsPerSensor;

  for (SVDCluster & aCluster : aSvdClusterArray) {
    VxdID::baseType thisID = aCluster.getSensorID().getID();
    bool foundSensor = false;
    for (SensorAndCluster & thisSensor : clustersPerSensor) {
      if (thisSensor.first == thisID) { thisSensor.second++; foundSensor = true; }
    }

    if (foundSensor == false) { clustersPerSensor.push_back({thisID, 1}); }
  }
  // now we now total number of clusters per sensor, calculating number of clusterCombinations:
  stringstream printCombinations;
  printCombinations << "Found the following SVD-combinations:\n";
  for (SensorAndCluster & thisSensor : clustersPerSensor) {
    uShort halfNumber = static_cast<uShort>((float(thisSensor.second) + 0.5) * 0.5);
    uShort nCombinations = halfNumber * halfNumber;
    clusterCombinationsPerSensor.push_back({thisSensor.first, nCombinations});
    printCombinations << VxdID(thisSensor.first) << ": " << nCombinations << "\n";
    m_overallCombinationsCount.push_back(nCombinations);

    bool foundSensor = false;
    for (SensorAndClusterCollection & anotherSensor : m_overallCombinationsPerSensor) {
      if (anotherSensor.first == thisSensor.first) { anotherSensor.second.push_back(nCombinations); foundSensor = true; }
    }
    if (foundSensor == false) {
      m_overallCombinationsPerSensor.push_back({thisSensor.first, vector<uShort>(1, nCombinations)});
    }
  }

  //  fetching data for congruence table:
  if (foundEvaluatingTCs == true and foundReferenceTCs == true) {
    m_countBothTrue++;
  } else if (foundEvaluatingTCs == true and foundReferenceTCs == false) {
    m_countEvalTrueRefFalse++;
  } else if (foundEvaluatingTCs == false and foundReferenceTCs == true) {
    m_countEvalFalseRefTrue++;
  } else if (foundEvaluatingTCs == false and foundReferenceTCs == false) {
    m_countBothFalse++;
  }


  // collecting info about typical length of reference TCs
  int totalRefHits = 0, totalEvalHits = 0;
  for (genfit::TrackCand & aTC : tcsReference) {
    int total = 0, svd = 0, tel = 0, pxd = 0;
    total = aTC.getHitIDs().size();
    svd = aTC.getHitIDs(Const::SVD).size();
    pxd = aTC.getHitIDs(Const::PXD).size();
    tel = aTC.getHitIDs(Const::TEST).size();
    m_referenceTrackLengthTotal.push_back(total);
    m_countReferenceTrackLengthTotal += total;
    totalRefHits += total;
    m_referenceTrackLengthSvd.push_back(svd);
    m_countReferenceTrackLengthSvd += svd;

    m_referenceTrackLengthPxd.push_back(pxd);
    m_countReferenceTrackLengthPxd += pxd;

    m_referenceTrackLengthTel.push_back(tel);
    m_countReferenceTrackLengthTel += tel;
  }
  float meanRefTCHits = float(totalRefHits) / float(nReferenceTCs);
  if (std::isnan(meanRefTCHits) == true) { meanRefTCHits = 0; }


  // collecting info about typical length of evaluate TCs
  bool found8ClusterTCs = false;
  for (genfit::TrackCand & aTC : tcsEvaluate) {
    int total = 0, svd = 0, tel = 0, pxd = 0;
    total = aTC.getHitIDs().size();
    svd = aTC.getHitIDs(Const::SVD).size();
    pxd = aTC.getHitIDs(Const::PXD).size();
    tel = aTC.getHitIDs(Const::TEST).size();
    m_evaluateTrackLengthTotal.push_back(total);
    m_countEvaluateTrackLengthTotal += total;
    totalEvalHits += total;
    m_evaluateTrackLengthSvd.push_back(svd);
    m_countEvaluateTrackLengthSvd += svd;

    m_evaluateTrackLengthPxd.push_back(pxd);
    m_countEvaluateTrackLengthPxd += pxd;

    m_evaluateTrackLengthTel.push_back(tel);
    m_countEvaluateTrackLengthTel += tel;

    if (aTC.getHitIDs(Const::SVD).size() == 8) { found8ClusterTCs = true; }
  }
  float meanEvalTCHits = float(totalEvalHits) / float(nEvaluateTCs);
  if (std::isnan(meanEvalTCHits) == true) { meanEvalTCHits = 0; }


  //  fetching data for more strict congruence table:
  if (found8ClusterTCs == true and foundReferenceTCs == true) {
    m_countBothTrueFull++;
  } else if (found8ClusterTCs == true and foundReferenceTCs == false) {
    m_countEvalTrueRefFalseFull++;
  } else if (found8ClusterTCs == false and foundReferenceTCs == true) {
    m_countEvalFalseRefTrueFull++;
  } else if (found8ClusterTCs == false and foundReferenceTCs == false) {
    m_countBothFalseFull++;
  }

  // eventWise output:
  if (m_eventCounter % m_stepSize == 0) {
    B2INFO("VXDTFTBAnalyzerModule - Event: " << m_eventCounter << " having " << aPxdClusterArray.getEntries() << "/" << aSvdClusterArray.getEntries() << " pxd/svdClusters and " << tcsReference.getEntries() << "(" << meanRefTCHits << ")/" << tcsEvaluate.getEntries() << "(" << meanEvalTCHits << ") reference/evaluateTCs(hitsPerTC). " << printCombinations.str());
  } else {
    B2DEBUG(5, "VXDTFTBAnalyzerModule - Event: " << m_eventCounter << " having " << aPxdClusterArray.getEntries() << "/" << aSvdClusterArray.getEntries() << " pxd/svdClusters and " << tcsReference.getEntries() << "(" << meanRefTCHits << ")/" << tcsEvaluate.getEntries() << "(" << meanEvalTCHits << ") reference/evaluateTCs(hitsPerTC). " << printCombinations.str());
  }
}


void VXDTFTBAnalyzerModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;
  B2INFO("VXDTFTBAnalyzerModule: after " << m_eventCounter << " events there were " << m_pxdClusterCounter << "/" << m_svdClusterCounter << " pxd/svdClusters total and " << double(m_pxdClusterCounter)*invEvents << "/" << double(m_svdClusterCounter)*invEvents << " pxd/svdClusters per event");

  float invTotalVal = 1. / float(m_countBothTrue + m_countBothFalse + m_countEvalTrueRefFalse + m_countEvalFalseRefTrue);
  float diagonalEfficiency = float(m_countBothTrue + m_countBothFalse) * invTotalVal;
  if (std::isnan(diagonalEfficiency) == true) { diagonalEfficiency = 0; }
  float tunedEfficiency = float(m_countBothTrue + m_countBothFalse + m_countEvalTrueRefFalse) * invTotalVal;
  if (std::isnan(tunedEfficiency) == true) { tunedEfficiency = 0; }

  B2INFO("VXDTFTBAnalyzerModule: after " << m_eventCounter << " events the congruence values are as follows:\n"
         << "nCases when the reference source found a TC and the evaluating source too: " << m_countBothTrue
         << "\nnCases when the reference source found a TC but the evaluating source didn't: " << m_countEvalFalseRefTrue
         << "\nnCases when the evaluating source found a TC but the reference source didn't: "  << m_countEvalTrueRefFalse
         << "\nnCases when both source couldn't find a TC: " << m_countBothFalse
         << "\nEfficiency (caseBothYesOrNo/nTotal): " << diagonalEfficiency
         << "\nEfficiency ((caseBothYesOrNo+nCasesRefTCsFound)/nTotal): " << tunedEfficiency);

  float invTotalValFull = 1. / float(m_countBothTrueFull + m_countBothFalseFull + m_countEvalTrueRefFalseFull + m_countEvalFalseRefTrueFull);
  float diagonalEfficiencyFull = float(m_countBothTrueFull + m_countBothFalseFull) * invTotalValFull;
  if (std::isnan(diagonalEfficiencyFull) == true) { diagonalEfficiencyFull = 0; }
  float tunedEfficiencyFull = float(m_countBothTrueFull + m_countBothFalseFull + m_countEvalTrueRefFalseFull) * invTotalValFull;
  if (std::isnan(tunedEfficiencyFull) == true) { tunedEfficiencyFull = 0; }

  B2INFO("VXDTFTBAnalyzerModule: after " << m_eventCounter << " events the congruence values for the 8-clustes-at-the-SVD are as follows:\n"
         << "nCases when the reference source found a TC with 8 svdClusters and the evaluating source too: " << m_countBothTrueFull
         << "\nnCases when the reference source found a TC with 8 svdClusters but the evaluating source didn't: " << m_countEvalFalseRefTrueFull
         << "\nnCases when the evaluating source found a TC with 8 svdClusters but the reference source didn't: "  << m_countEvalTrueRefFalseFull
         << "\nnCases when both source couldn't find a TC with 8 svdClusters: " << m_countBothFalseFull
         << "\nEfficiency8Cluster (caseBothYesOrNo/nTotal): " << diagonalEfficiencyFull
         << "\nEfficiency8Cluster ((caseBothYesOrNo+nCasesRefTCsFound)/nTotal): " << tunedEfficiencyFull);

  B2INFO("VXDTFTBAnalyzerModule: after " << m_eventCounter << " events typical TC-lengths are as follows:\n"
         << "nClusters-total per TC added to reference/evaluate source: "
         << float(m_countReferenceTrackLengthTotal) / float(m_referenceTrackLengthTotal.size())
         << "/" << float(m_countEvaluateTrackLengthTotal) / float(m_evaluateTrackLengthTotal.size())
         << "\nnClusters-SVD per TC added to reference/evaluate source: "
         << float(m_countReferenceTrackLengthSvd) / float(m_referenceTrackLengthSvd.size())
         << "/" << float(m_countEvaluateTrackLengthSvd) / float(m_evaluateTrackLengthSvd.size())
         << "\nnClusters-PXD per TC added to reference/evaluate source: "
         << float(m_countReferenceTrackLengthPxd) / float(m_referenceTrackLengthPxd.size())
         << "/" << float(m_countEvaluateTrackLengthPxd) / float(m_evaluateTrackLengthPxd.size())
         << "\nnClusters-TEL per TC added to reference/evaluate source: "
         << float(m_countReferenceTrackLengthTel) / float(m_referenceTrackLengthTel.size())
         << "/" << float(m_countEvaluateTrackLengthTel) / float(m_evaluateTrackLengthTel.size()));

  stringstream printCombinations;
  printCombinations << "Found the following SVD-combinations:\n";
  for (SensorAndClusterCollection & entry : m_overallCombinationsPerSensor) {
    int q1 = 0, q5 = 0, q25 = 0, q50 = 0, q75 = 0, q95 = 0, q99 = 0; // index numbers for quantiles 1,5,25,median,75,95, 99
    int nEntries = entry.second.size();
    std::sort(entry.second.begin(), entry.second.end());
    q1 = int(float(nEntries) * 0.01);
    LittleHelper::checkBoundaries(q1, 0, nEntries - 1);
    q5 = int(float(nEntries) * 0.05);
    LittleHelper::checkBoundaries(q5, 0, nEntries - 1);
    q25 = int(float(nEntries) * 0.25);
    LittleHelper::checkBoundaries(q25, 0, nEntries - 1);
    q50 = int(float(nEntries) * 0.5);
    LittleHelper::checkBoundaries(q50, 0, nEntries - 1);
    q75 = int(float(nEntries) * 0.75);
    LittleHelper::checkBoundaries(q75, 0, nEntries - 1);
    q95 = int(float(nEntries) * 0.95);
    LittleHelper::checkBoundaries(q95, 0, nEntries - 1);
    q99 = int(float(nEntries) * 0.99);
    LittleHelper::checkBoundaries(q99, 0, nEntries - 1);
    printCombinations << VxdID(entry.first) << ": "
                      << "min=" << entry.second.at(0)
                      << ", q1=" << entry.second.at(q1)
                      << ", q5=" << entry.second.at(q5)
                      << ", q25=" << entry.second.at(q25)
                      << ", median=" << entry.second.at(q50)
                      << ", q75=" << entry.second.at(q75)
                      << ", q95=" << entry.second.at(q95)
                      << ", q99=" << entry.second.at(q99)
                      << ", max=" << entry.second.at(nEntries - 1) << "\n";
  }
  int countTotalClusterCombis = 0, nMeasurements = m_overallCombinationsCount.size();
  for (auto entry : m_overallCombinationsCount) {
    countTotalClusterCombis += entry;
  }
  std::sort(m_overallCombinationsCount.begin(), m_overallCombinationsCount.end());
  B2INFO("Distribution of Svd-Cluster-combinations (total/mean/median:" << countTotalClusterCombis << "/" << countTotalClusterCombis / nMeasurements << "/" << m_overallCombinationsCount.at(nMeasurements / 2) << ") per layer:\n" << printCombinations.str());
}


void VXDTFTBAnalyzerModule::terminate()
{

}

