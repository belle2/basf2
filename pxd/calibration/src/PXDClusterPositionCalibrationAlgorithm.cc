/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterPositionCalibrationAlgorithm.h>

#include <string>
#include <tuple>
#include "TH2F.h"

#include <boost/format.hpp>
#include <cmath>

using namespace std;
using boost::format;
using namespace Belle2;

PXDClusterPositionCalibrationAlgorithm::PXDClusterPositionCalibrationAlgorithm():
  CalibrationAlgorithm("PXDClusterPositionCollector"),
  minClusterForShapeLikelyhood(500), minClusterForPositionOffset(2000), maxEtaBins(10)
{
  setDescription(
    " -------------------------- PXDClusterPositionCalibrationAlgorithm ----------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating cluster position offsets and shape likelyhoods.               \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult PXDClusterPositionCalibrationAlgorithm::calibrate()
{

  // Read back the V pitch of all cluster kinds in source data
  // This avoids relying on VXD::GeoCache.
  auto pitchtree = getObjectPtr<TTree>("pitchtree");
  pitchtree->SetBranchAddress("PitchV", &m_pitchV);
  pitchtree->SetBranchAddress("ClusterKind", &m_clusterKind);

  for (int i = 0; i < pitchtree->GetEntries(); ++i) {
    pitchtree->GetEntry(i);
    m_pitchMap[m_clusterKind] = m_pitchV;
  }

  // Buffer temporary payloads for shape calibration for all
  // clusterkinds and angle bins
  typedef tuple<int, int, int> bufferkey_t;
  typedef pair<PXDClusterShapeIndexPar, PXDClusterShapeClassifierPar> buffervalue_t;
  map<bufferkey_t, buffervalue_t> localCalibrationMap;

  for (auto clusterKind : clusterKinds) {

    B2INFO("Start calibration of clusterkind=" << clusterKind << " ...");

    string gridname = str(format("GridKind_%1%") % clusterKind);
    auto grid = getObjectPtr<TH2F>(gridname);

    for (auto uBin = 1; uBin <= grid->GetXaxis()->GetNbins(); uBin++) {
      for (auto vBin = 1; vBin <= grid->GetYaxis()->GetNbins(); vBin++) {

        // Bin is centered around angles
        auto thetaU = grid->GetXaxis()->GetBinCenter(uBin);
        auto thetaV = grid->GetYaxis()->GetBinCenter(vBin);

        if (thetaV < 0) {
          B2INFO("Skip training estimator on thetaU=" << thetaU << ", thetaV=" << thetaV);
          continue;
        } else {
          B2INFO("Start training estimator on thetaU=" << thetaU << ", thetaV=" << thetaV);
        }

        string treename = str(format("tree_%1%_%2%_%3%") % clusterKind % uBin % vBin);

        auto localShapeIndexer = PXDClusterShapeIndexPar();
        auto localShapeClassifier = PXDClusterShapeClassifierPar();
        createShapeClassifier(treename, &localShapeClassifier, &localShapeIndexer);

        bufferkey_t key = std::make_tuple(clusterKind, uBin, vBin);
        localCalibrationMap[key] = buffervalue_t(localShapeIndexer ,  localShapeClassifier);
      }
    }
  }

  // Create a ShapeIndexer payload
  PXDClusterShapeIndexPar* shapeIndexer = new PXDClusterShapeIndexPar();

  int globalShapeIndex = 0;
  for (auto it = m_shapeSet.begin(); it != m_shapeSet.end(); ++it) {
    shapeIndexer->addShape(*it, globalShapeIndex);
    globalShapeIndex++;
  }

  B2INFO("Number of cluster shapes is " << globalShapeIndex);

  // Save the cluster shape index table to database.
  saveCalibration(shapeIndexer, "PXDClusterShapeIndexPar");

  // Create position estimator
  PXDClusterPositionEstimatorPar* positionEstimator = new PXDClusterPositionEstimatorPar();

  for (auto clusterKind : clusterKinds) {

    string gridname = str(format("GridKind_%1%") % clusterKind);
    auto grid = getObjectPtr<TH2F>(gridname);

    positionEstimator->addGrid(clusterKind, *grid);

    for (auto uBin = 1; uBin <= grid->GetXaxis()->GetNbins(); uBin++) {
      for (auto vBin = 1; vBin <= grid->GetYaxis()->GetNbins(); vBin++) {

        // Bin is centered around angles
        auto thetaU = grid->GetXaxis()->GetBinCenter(uBin);
        auto thetaV = grid->GetYaxis()->GetBinCenter(vBin);

        if (thetaV < 0) {
          // We skipped this part in training before
          continue;
        }

        // Find the local calibration results
        auto iter = localCalibrationMap.find(std::make_tuple(clusterKind, uBin, vBin));
        auto localShapeIndexer = iter->second.first;
        auto localShapeClassifer = iter->second.second;

        // Require that all shape classifiers use a common shape indexer
        auto shapeClassifier = localToGlobal(&localShapeClassifer, &localShapeIndexer, shapeIndexer);

        // Mirror the shape classifier along v
        auto mirror_vBin = grid->GetYaxis()->FindBin(-thetaV);
        auto mirroredClassifier = mirrorShapeClassifier(&shapeClassifier, shapeIndexer, clusterKind);

        // and fill into position estimator payload
        B2INFO("Add shape classifier for angles thetaU=" << thetaU << ", thetaV=" << thetaV << ", clusterkind=" << clusterKind);
        positionEstimator->setShapeClassifier(shapeClassifier, uBin, vBin, clusterKind);
        B2INFO("Add mirrored shape classifier for angles thetaU=" << thetaU << ", thetaV=" << -thetaV << ", clusterkind=" << clusterKind);
        positionEstimator->setShapeClassifier(mirroredClassifier, uBin, mirror_vBin, clusterKind);
      }
    }
  }

  // Save the cluster positions to database.
  saveCalibration(positionEstimator, "PXDClusterPositionEstimatorPar");

  B2INFO("PXDClusterPosition Calibration Successful");
  return c_OK;
}


PXDClusterShapeClassifierPar PXDClusterPositionCalibrationAlgorithm::mirrorShapeClassifier(PXDClusterShapeClassifierPar*
    shapeClassifier, PXDClusterShapeIndexPar* shapeIndexer, int clusterKind)
{
  // Create a mirrored shape classifier
  auto mirroredShapeClassifier = PXDClusterShapeClassifierPar();

  // Mirror the shape likelyhood map
  auto shapeLikelyhoodMap = shapeClassifier->getShapeLikelyhoodMap();
  for (auto indexAndValue : shapeLikelyhoodMap) {
    // Compute the mirrored shape index
    auto shapeIndex = indexAndValue.first;
    auto shapeName = shapeIndexer->getShapeName(shapeIndex);
    auto mirroredName = m_mirrorMap[shapeName];
    auto mirroredIndex = shapeIndexer->getShapeIndex(mirroredName);
    // Store the result
    mirroredShapeClassifier.addShapeLikelyhood(mirroredIndex, indexAndValue.second);
  }

  // Mirror the offset related maps
  auto offsetMap = shapeClassifier->getOffsetMap();
  auto percentileMap = shapeClassifier->getPercentilesMap();
  auto likelyhoodMap = shapeClassifier->getLikelyhoodMap();
  for (auto indexAndValue : offsetMap) {
    // Compute the mirrored shape index
    auto shapeIndex = indexAndValue.first;
    auto shapeName = shapeIndexer->getShapeName(shapeIndex);
    auto mirroredName = m_mirrorMap[shapeName];
    auto mirroredIndex = shapeIndexer->getShapeIndex(mirroredName);

    mirroredShapeClassifier.addShape(mirroredIndex);

    int etaBin = 0;
    for (auto offset : indexAndValue.second) {
      // Copy over percentile
      auto percentile = percentileMap[shapeIndex][etaBin];
      mirroredShapeClassifier.addEtaPercentile(mirroredIndex, percentile);
      // Copy over likelyhood
      auto likelyhood = likelyhoodMap[shapeIndex][etaBin];
      mirroredShapeClassifier.addEtaLikelyhood(mirroredIndex, likelyhood);
      // Mirror the offset: v offset shifts and covariance swaps sign
      double shift = (m_sizeMap[shapeName] - 1) * m_pitchMap[clusterKind];
      auto mirroredOffset = PXDClusterOffsetPar(offset.getU(), shift - offset.getV(), offset.getUSigma2(), offset.getVSigma2(),
                                                -offset.getUVCovariance());
      mirroredShapeClassifier.addEtaOffset(mirroredIndex, mirroredOffset);
      etaBin++;
    }
  }

  return mirroredShapeClassifier;
}

PXDClusterShapeClassifierPar PXDClusterPositionCalibrationAlgorithm::localToGlobal(PXDClusterShapeClassifierPar* shapeClassifier,
    PXDClusterShapeIndexPar* shapeIndexer, PXDClusterShapeIndexPar* globalShapeIndexer)
{
  // Create a shape classifier using global shape indices
  auto globalShapeClassifier = PXDClusterShapeClassifierPar();

  // Re-index the the shape likelyhood map
  auto shapeLikelyhoodMap = shapeClassifier->getShapeLikelyhoodMap();
  for (auto indexAndValue : shapeLikelyhoodMap) {
    // Compute the global shape index
    auto shapeIndex = indexAndValue.first;
    auto shapeName = shapeIndexer->getShapeName(shapeIndex);
    auto globalIndex = globalShapeIndexer->getShapeIndex(shapeName);
    // Store the result
    globalShapeClassifier.addShapeLikelyhood(globalIndex, indexAndValue.second);
  }

  // Re-index the offset related maps
  auto offsetMap = shapeClassifier->getOffsetMap();
  auto percentileMap = shapeClassifier->getPercentilesMap();
  auto likelyhoodMap = shapeClassifier->getLikelyhoodMap();
  for (auto indexAndValue : offsetMap) {
    // Compute the global shape index
    auto shapeIndex = indexAndValue.first;
    auto shapeName = shapeIndexer->getShapeName(shapeIndex);
    auto globalIndex = globalShapeIndexer->getShapeIndex(shapeName);

    globalShapeClassifier.addShape(globalIndex);

    int etaBin = 0;
    for (auto offset : indexAndValue.second) {
      // Copy over percentile
      auto percentile = percentileMap[shapeIndex][etaBin];
      globalShapeClassifier.addEtaPercentile(globalIndex, percentile);
      // Copy over likelyhood
      auto likelyhood = likelyhoodMap[shapeIndex][etaBin];
      globalShapeClassifier.addEtaLikelyhood(globalIndex, likelyhood);
      // Copy over offset
      globalShapeClassifier.addEtaOffset(globalIndex, offset);
      etaBin++;
    }
  }
  return globalShapeClassifier;
}

void PXDClusterPositionCalibrationAlgorithm::createShapeClassifier(string treename,
    PXDClusterShapeClassifierPar* shapeClassifier, PXDClusterShapeIndexPar* shapeIndexer)
{

  auto tree = getObjectPtr<TTree>(treename);

  const auto nEntries = tree->GetEntries();
  B2INFO("Number of clusters is " << nEntries);

  string* shapeNamePtr = &m_shapeName;
  string* mirroredShapeNamePtr = &m_mirroredShapeName;

  tree->SetBranchAddress("ShapeName", &shapeNamePtr);
  tree->SetBranchAddress("MirroredShapeName", &mirroredShapeNamePtr);
  tree->SetBranchAddress("ClusterEta", &m_clusterEta);
  tree->SetBranchAddress("OffsetU", &m_positionOffsetU);
  tree->SetBranchAddress("OffsetV", &m_positionOffsetV);
  tree->SetBranchAddress("SizeV", &m_sizeV);

  // Vector to enumerate all shapes by unique name and count their
  // occurence in training data.
  vector< pair<string, float> > shapeList;

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    string shapeName = m_shapeName;

    auto it = std::find_if(shapeList.begin(), shapeList.end(),
    [&](const pair<string, float>& element) { return element.first == shapeName;});

    //Shape name exists in vector
    if (it != shapeList.end()) {
      //increment key in map
      it->second++;
    }
    //Shape name does not exist
    else {
      //Not found, insert in vector
      shapeList.push_back(pair<string, int>(shapeName, 1));
      // Remember the relation between name of a shape and name of mirrored shape
      m_mirrorMap[shapeName] = m_mirroredShapeName;
      // Remember the relation between name of a shape and its size
      m_sizeMap[shapeName] = m_sizeV;
    }
  }

  // Loop over shapeList to select shapes for
  // next calibration step

  // Vector with eta histograms for selected shapes
  vector< pair<string, TH1D> > etaHistos;

  // Index for enumerating selected shapes
  int tmpIndex = 0;

  // Coverage of position offsets on training data
  double coverage = 0.0;

  for (auto iter : shapeList) {
    auto name = iter.first;
    auto counter = iter.second;

    double likelyhood = counter / nEntries;

    if (counter >=  minClusterForShapeLikelyhood) {
      //B2INFO("Adding shape " << name << " with index " << tmpIndex << " and shape likelyhood " << 100*likelyhood << "% and count " << counter);
      shapeIndexer->addShape(name, tmpIndex);
      shapeClassifier->addShapeLikelyhood(tmpIndex, likelyhood);

      // Add name of shape to global (all clusterkinds + all angle bins) shape set
      m_shapeSet.insert(name);
      // Add name of mirrored shape as well
      m_shapeSet.insert(m_mirrorMap[name]);
      // Increment the index
      tmpIndex++;
    }

    if (counter >=  minClusterForPositionOffset) {
      coverage += likelyhood;
      string etaname = str(format("eta_%1%") % name);

      // Single pixel case: Eta value is cluster charge
      if (name == "SD0.0") {
        etaHistos.push_back(pair<string, TH1D>(name, TH1D(etaname.c_str(), etaname.c_str(), 255, 0, 255)));
      }
      // Multipixel case: Eta value is ratio head/(tail+head) of charges (to be less gain sensitive)
      else {
        etaHistos.push_back(pair<string, TH1D>(name, TH1D(etaname.c_str(), etaname.c_str(), 301, 0, 1)));
      }
    }
  }

  B2INFO("Offset coverage is "  << 100 * coverage << "%");

  // Loop over the tree is to fill the eta histograms for
  // selected shapes.

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    string shapeName = m_shapeName;
    auto it = std::find_if(etaHistos.begin(), etaHistos.end(),
    [&](const pair<string, TH1D>& element) { return element.first == shapeName;});
    //Item exists in map
    if (it != etaHistos.end()) {
      // increment key in map
      it->second.Fill(m_clusterEta);
    }
  }

  // Vector for offset histograms stored by offset shape name and eta bin
  vector< pair< string, vector<TH2D> > > offsetHistosVec;

  for (auto iter : etaHistos) {
    auto name = iter.first;
    auto& histo = iter.second;
    int nClusters = histo.GetEntries();

    // Add shape for offset correction
    int shapeIndex = shapeIndexer->getShapeIndex(name);
    shapeClassifier->addShape(shapeIndex);

    // Try to split clusters into n bins with minClusterForPositionOffset clusters
    int nEtaBins  = std::max(int(nClusters / minClusterForPositionOffset), 1);
    nEtaBins =  std::min(nEtaBins, maxEtaBins);

    //B2INFO("SHAPE NAME:" << name << " WITH BINS " << nEtaBins);

    vector< TH2D > offsetHistos;

    for (int i = 0; i < nEtaBins; i++) {
      // Position where to compute the quantiles in [0,1]
      double xq = double(i) / nEtaBins;
      // Double to contain the quantile
      double yq = 0;
      histo.GetQuantiles(1, &yq, &xq);
      //B2INFO("   Quantile at xq =" << xq << " is yq=" << yq);
      shapeClassifier->addEtaPercentile(shapeIndex, yq);

      string offsetname = str(format("offset_%1%_%2%") % name % i);
      TH2D offsetHisto(offsetname.c_str(), offsetname.c_str(), 1, 0, 1, 1, 0, 1);
      offsetHisto.StatOverflows();
      offsetHistos.push_back(offsetHisto);

    }
    offsetHistosVec.push_back(pair< string, vector<TH2D> >(name, offsetHistos));
  }

  // Loop over the tree is to fill offset histograms

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    string shapeName = m_shapeName;
    auto it = std::find_if(offsetHistosVec.begin(), offsetHistosVec.end(),
    [&](const pair<string, vector<TH2D>>& element) { return element.first == shapeName;});
    //Item exists in map
    if (it != offsetHistosVec.end()) {
      int shapeIndex = shapeIndexer->getShapeIndex(shapeName);
      int etaBin = shapeClassifier->getEtaIndex(shapeIndex, m_clusterEta);
      it->second.at(etaBin).Fill(m_positionOffsetU, m_positionOffsetV);
    }
  }

  // Compute the moments of the offset histograms and finalize the shape classifier object

  // Loop over shape names
  for (auto iter : offsetHistosVec) {
    auto name = iter.first;
    auto& histovec = iter.second;

    int shapeIndex = shapeIndexer->getShapeIndex(name);

    // Loop over eta bins
    for (auto& histo : histovec) {
      // Compute offset moments
      double etaLikelyhood = double(histo.GetEntries()) / nEntries;
      double offsetU = histo.GetMean(1);
      double offsetV = histo.GetMean(2);
      double covUV = histo.GetCovariance();
      double covU = pow(histo.GetRMS(1), 2);
      double covV = pow(histo.GetRMS(2), 2);

      B2INFO("Name " << name  << ", posU=" << offsetU << ", posV=" << offsetV << ", covU=" << covU << ", covV=" << covV << ", covUV=" <<
             covUV);

      auto offset = PXDClusterOffsetPar(offsetU, offsetV, covU, covV, covUV);
      shapeClassifier->addEtaLikelyhood(shapeIndex, etaLikelyhood);
      shapeClassifier->addEtaOffset(shapeIndex, offset);
    }
  }

  B2INFO("Added shape classifier with coverage " << 100 * coverage << "% on training data sample.");

  return;
}


