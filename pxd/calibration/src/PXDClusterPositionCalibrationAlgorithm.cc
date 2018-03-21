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
#include "TH2I.h"

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

  for (auto clusterKind : clusterKinds) {

    B2INFO("Start calibration of clusterkind=" << clusterKind << " ...");

    string gridname = str(format("GridKind_%1%") % clusterKind);
    auto grid = getObjectPtr<TH2I>(gridname);

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

        PXDClusterShapeClassifierPar* ShapeClassifier = new PXDClusterShapeClassifierPar();
        PXDClusterShapeIndexPar* ShapeIndexer = new PXDClusterShapeIndexPar();
        createShapeClassifier(treename, ShapeClassifier, ShapeIndexer);

      }
    }
  }

  /*
  // Create position estimator
  estimator_payload = Belle2.PXDClusterPositionEstimatorPar();
  for (auto clusterKind : clusterKinds) {
    grid = ROOT.TH2F("grid_{:d}".format(pixelkind), "grid_{:d}".format(pixelkind), 18, -90.0, +90.0, 18, -90.0, +90.0)
    estimator_payload.addGrid(pixelkind, grid)

    for uBin in range(1, grid.GetXaxis().GetNbins() + 1):
      for vBin in range(1, grid.GetYaxis().GetNbins() + 1):
        classifier_payload = Belle2.PXDClusterShapeClassifierPar()

        for shape in classifier.getDigitalLabels():
          # Likelyhood to create shape
          classifier_payload.addShapeLikelyhood(shape_index, classifier.getDigitalProb(shape))

          for eta_index, hit in classifier.getHitMap(shape).items():
             offset_payload = Belle2.PXDClusterOffsetPar(0.1 * pos[0], 0.1 * pos[1], 0.01 * cov[0, 0], 0.01 * cov[1, 1], 0.01 * cov[1, 0])
             classifier_payload.addEtaOffset(shape_index, offset_payload)

             # Fill percentile and eta likelyhood
             classifier_payload.addEtaPercentile(shape_index, percentiles[eta_index - 1])
             classifier_payload.addEtaLikelyhood(shape_index, prob)

        # and fill into position estimator payload
        estimator_payload.setShapeClassifier(classifier_payload, uBin, vBin, pixelkind)
  }
  */

  // Save the cluster positions and index table to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  PXDClusterPositionEstimatorPar* PositionEstimator = new PXDClusterPositionEstimatorPar();
  saveCalibration(PositionEstimator, "PXDClusterPositionEstimatorPar");

  PXDClusterShapeIndexPar* ClusterShapeIndexer = new PXDClusterShapeIndexPar();
  saveCalibration(ClusterShapeIndexer, "PXDClusterShapeIndexPar");

  B2INFO("PXDClusterPosition Calibration Successful");
  return c_OK;
}


void PXDClusterPositionCalibrationAlgorithm::createShapeClassifier(string treename,
    PXDClusterShapeClassifierPar* classifier, PXDClusterShapeIndexPar* index)
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

  // The first loop over the tree is to enumerate all shapes and
  // and to find their frequency.
  vector< pair<string, float> > shapeList;

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    string shapeName = m_shapeName;

    auto it = std::find_if(shapeList.begin(), shapeList.end(),
    [&](const pair<string, float>& element) { return element.first == shapeName;});

    //Item exists in map
    if (it != shapeList.end()) {
      //increment key in map
      it->second++;
    }
    //Item does not exist
    else {
      //Not found, insert in map
      shapeList.push_back(pair<string, int>(shapeName, 1));
    }
  }

  // Postprocessing of first loop: Enumerate shapes and compute their likelyhood
  unsigned int shapeIndex = 0;
  double coverage = 0.0;
  vector< pair<string, TH1D> > etaHistos;

  for (auto iter : shapeList) {
    auto name = iter.first;
    auto counter = iter.second;

    double likelyhood = counter / nEntries;

    if (counter >=  minClusterForShapeLikelyhood) {
      index->addShape(name, shapeIndex);
      shapeIndex++;

      B2INFO("Add shape " << name << " wiht shape likelyhood " << likelyhood << " and count " << counter);

      classifier->addShape(shapeIndex);
      classifier->addShapeLikelyhood(shapeIndex, likelyhood);
    }

    if (counter >=  minClusterForPositionOffset) {
      coverage += likelyhood;
      string etaname = str(format("eta_%1%") % name);

      // Single pixel case
      if (name == "SD0.0") {
        etaHistos.push_back(pair<string, TH1D>(name, TH1D(etaname.c_str(), etaname.c_str(), 255, 0, 255)));
      }
      // Multipixel case
      else {
        etaHistos.push_back(pair<string, TH1D>(name, TH1D(etaname.c_str(), etaname.c_str(), 301, 0, 1)));
      }
    }
  }

  B2INFO("Offset coverage is "  << 100 * coverage << "%");

  // The second loop over the tree is to fill eta histograms for
  // frequent shapes.

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

  vector< pair<string, TH2D> > offsetHistos;
  vector< pair<string, double> > offsetQuantiles;

  for (auto iter : etaHistos) {
    auto name = iter.first;
    auto& histo = iter.second;
    int nClusters = histo.GetEntries();

    // Try to split clusters into n bins with minClusterForPositionOffset clusters
    int nEtaBins  = std::max(int(nClusters / minClusterForPositionOffset), 1);

    // Limit the number of bins if needed
    nEtaBins =  std::min(nEtaBins, maxEtaBins);

    B2INFO("SHAPE NAME:" << name << " WITH BINS " << nEtaBins);

    for (int i = 0; i < nEtaBins; i++) {
      // Position where to compute the quantiles in [0,1]
      double xq = float(i + 1) / nEtaBins;
      // Double to contain the quantile
      double yq = 0;
      histo.GetQuantiles(1, &yq, &xq);
      B2INFO("   Quantile at xq =" << xq << " is yq=" << yq);

      string offsetname = str(format("offset_%1%_%2%") % name % i);
      TH2D offsetHisto(offsetname.c_str(), offsetname.c_str(), 1, 0, 1, 1, 0, 1);
      offsetHisto.StatOverflows();
      offsetHistos.push_back(pair<string, TH2D>(offsetname, offsetHisto));

      offsetQuantiles.push_back(pair<string, float>(offsetname, yq));
    }
  }

  // The third loop over the tree is to fill histograms for compurting the first and
  // second moment of truehit intersects

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    int etaBin = 0;
    string offsetname = str(format("offset_%1%_%2%") % m_shapeName % etaBin);

    auto it = std::find_if(offsetHistos.begin(), offsetHistos.end(),
    [&](const pair<string, TH2D>& element) { return element.first == offsetname;});
    //Item exists in map
    if (it != offsetHistos.end()) {
      it->second.Fill(m_positionOffsetU, m_positionOffsetV);
    }
  }

  // Fill the offsets and finalize the shape classifier object

  for (auto iter : offsetHistos) {
    auto name = iter.first;
    auto& offsetHisto = iter.second;

    // Compute offset moments
    double offsetU = offsetHisto.GetMean(1);
    double offsetV = offsetHisto.GetMean(2);
    double covUV = offsetHisto.GetCovariance();
    double covU = pow(offsetHisto.GetRMS(1), 2);
    double covV = pow(offsetHisto.GetRMS(2), 2);

    B2INFO("OFFSETNAME " << name << ", posU=" << offsetU << ", posV=" << offsetV << ", covU=" << covU << ", covV=" << covV << ", covUV="
           << covUV);
  }

  B2INFO("Added shape classifier with coverage " << 100 * coverage << "% on training data sample.");

  return;
}


