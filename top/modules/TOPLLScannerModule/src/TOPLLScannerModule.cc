/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPLLScannerModule/TOPLLScannerModule.h>

#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <iostream>
#include <TTree.h>
#include <TH2F.h>
#include <TFile.h>
#include <TRandom.h>
#include <TGraph.h>
#include <TF1.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TOP;

REG_MODULE(TOPLLScanner)

TOPLLScannerModule::TOPLLScannerModule() : Module()
{
  setDescription(R"DOC(A module to perform the TOP PID likelihood scan and find the actual minimum as function of the mass)DOC");
}

TOPLLScannerModule::~TOPLLScannerModule()
{
}

void TOPLLScannerModule::scanLikelihood(std::vector<float>masses, std::vector<float>logLs, float deltaLL, float& maxLL,
                                        float& massMax, float& minMassRange, float& maxMassRange)
{
  // Find the index and the value of the LL maximum
  auto llMaxIndex = std::distance(logLs.begin(), std::max_element(logLs.begin(), logLs.end()));
  maxLL =  logLs[llMaxIndex];
  massMax = masses[llMaxIndex];

  // if no deltaLL value is given, return a +/- 30 MeV range around the maximum
  if (isnan(deltaLL)) {
    minMassRange = massMax - 0.03;
    if (minMassRange < masses[0])
      minMassRange = masses[0];

    maxMassRange = massMax + 0.03;
    if (maxMassRange < masses[0])
      maxMassRange = masses.back();
  } else {
    // search forward
    auto llValue = maxLL;
    int llIndex = llMaxIndex;
    while (llValue > maxLL - deltaLL && llIndex < int(masses.size())) {
      llValue = logLs[llIndex];
      maxMassRange = masses[llIndex];
      llIndex++;
    }
    // search backward
    llValue = maxLL;
    llIndex = llMaxIndex;
    while (llValue > maxLL - deltaLL && llIndex >= 0) {
      llValue = logLs[llIndex];
      minMassRange = masses[llIndex];
      llIndex--;
    }
  }
}

void TOPLLScannerModule::initialize()
{
  m_digits.isRequired();
  m_tracks.isRequired();
  m_extHits.isRequired();
  m_barHits.isOptional();
  m_recBunch.isOptional();


  m_likelihoodScanResults.registerInDataStore();
  m_likelihoodScanResults.registerRelationTo(m_extHits);
  m_likelihoodScanResults.registerRelationTo(m_barHits);
  m_tracks.registerRelationTo(m_likelihoodScanResults);


  // Fill the array of masses for the coarse scan in un-even steps
  float step;
  float lastVal = 0.0002;
  for (auto i = 1; i < 240; i++) {
    if (i < 10)
      step = 0.001;
    else if (i < 50)
      step = 0.005;
    else step = 0.01;
    m_massPoints.push_back(lastVal);
    lastVal = lastVal + step;
  }
}

void TOPLLScannerModule::event()
{

  for (const auto& track : m_tracks) {
    auto* topLLScanRes = m_likelihoodScanResults.appendNew();
    track.addRelationTo(topLLScanRes);

    const TOPTrack trk(track);
    if (not trk.isValid())
      continue; // track missed the bars

    topLLScanRes->addRelationTo(trk.getExtHit());
    topLLScanRes->addRelationTo(trk.getBarHit());

    // -------------
    // First make a coarse scan to store it and find the LL maximum
    // -------------
    std::vector<float> logLcoarseScan;
    std::vector<float> nSignalPhotonsCoarseScan;

    for (const auto& mass : m_massPoints) {
      const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Optimal, PDFConstructor::c_Reduced, mass);
      pdfConstructor.switchOffDeltaRayPDF();
      auto LL = pdfConstructor.getLogL();
      logLcoarseScan.push_back(LL.logL);
      nSignalPhotonsCoarseScan.push_back(pdfConstructor.getExpectedSignalPhotons());
    }

    float massMax = -1;
    float minMassRange = 0;
    float maxMassRange = 0;
    float maxLL = 0;
    scanLikelihood(m_massPoints, logLcoarseScan, 0.8, maxLL, massMax, minMassRange, maxMassRange);


    // -------------
    // Then make a fine scan to store it and measure the TOP mass and uncertainty
    // -------------
    std::vector<float> logLfineScan;
    std::vector<float> massPointsFineScan;
    std::vector<float> nSignalPhotonsFineScan;
    auto mass = minMassRange;
    auto step = (maxMassRange - minMassRange) / m_nFineScanPoints;
    if (step > 0.001)
      step = 0.001;
    while (mass < maxMassRange) {
      const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Optimal, PDFConstructor::c_Reduced, mass);
      pdfConstructor.switchOffDeltaRayPDF();
      auto LL = pdfConstructor.getLogL();
      logLfineScan.push_back(LL.logL);
      nSignalPhotonsFineScan.push_back(pdfConstructor.getExpectedSignalPhotons());
      massPointsFineScan.push_back(mass);
      mass += step;
    }

    // find the maximum and the confidence interval usin the fine-grained scan
    scanLikelihood(massPointsFineScan, logLfineScan, 0.5, maxLL, massMax, minMassRange, maxMassRange);


    // finally find the location of the cherenkov threshold looking for a plateau of the
    // LL value
    auto lastLL = logLcoarseScan.back();
    auto currentLL = logLcoarseScan.back();
    auto index = m_massPoints.size() - 1;
    while (TMath::Abs(lastLL - currentLL) < 0.01 && index > 0) {
      lastLL = currentLL;
      currentLL = logLcoarseScan[index];
      index--;
    }
    float threshold = m_massPoints[index + 1];

    // grab the number of expected photons at the LL maximum
    const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Optimal, PDFConstructor::c_Reduced, massMax);
    pdfConstructor.switchOffDeltaRayPDF();
    float nSignalPhotons = pdfConstructor.getExpectedSignalPhotons();
    float nBackgroundPhotons = pdfConstructor.getExpectedBkgPhotons();
    float nDeltaPhotons = pdfConstructor.getExpectedDeltaPhotons();

    topLLScanRes->set(massMax,
                      minMassRange,
                      maxMassRange,
                      threshold,
                      nSignalPhotons,
                      nBackgroundPhotons,
                      nDeltaPhotons,
                      m_massPoints, massPointsFineScan,
                      logLcoarseScan, logLfineScan,
                      nSignalPhotonsCoarseScan, nSignalPhotonsFineScan);
  }
}

void TOPLLScannerModule::terminate()
{

}


