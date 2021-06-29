/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Umberto Tamponi                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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

  minMassRange = masses[0];
  maxMassRange = masses.back();
  // search forward
  auto llValue = maxLL;
  auto llIndex = llMaxIndex;
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


  // go up to 4 GeV to capture all light nuclei
  float step = 0;
  float lastVal = 0;
  for (auto i = 0; i < 200; i++) {
    if (i < 10)
      step = 0.001;
    else if (i < 20)
      step = 0.005;
    else if (i < 30)
      step = 0.008;
    else if (i < 40)
      step = 0.01;
    else if (i < 50)
      step = 0.015;
    else
      step = 0.020;
    m_massPoints.push_back(lastVal);
    lastVal = lastVal + step;
  }
}

void TOPLLScannerModule::event()
{
  if (! m_tracks.isValid())
    return;

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

    short iGuard = 0;
    for (const auto& mass : m_massPoints) {
      const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Fine, PDFConstructor::c_Reduced, mass);
      pdfConstructor.switchOffDeltaRayPDF();
      auto LL = pdfConstructor.getLogL();
      logLcoarseScan.push_back(LL.logL);
      nSignalPhotonsCoarseScan.push_back(pdfConstructor.getExpectedSignalPhotons());
    }

    float massMax = -1;
    float minMassRange = 0;
    float maxMassRange = 0;
    float maxLL = 0;
    scanLikelihood(m_massPoints, logLcoarseScan, 1, maxLL, massMax, minMassRange, maxMassRange);


    // -------------
    // Then make a fine scan to store it and measure the TOP mass and uncertainty
    // -------------
    std::vector<float> logLfineScan;
    std::vector<float> massPointsFineScan;
    std::vector<float> nSignalPhotonsFineScan;
    auto mass = minMassRange;
    auto step = (maxMassRange - minMassRange) / m_nFineScanPoints;
    while (mass < maxMassRange) {
      const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Fine, PDFConstructor::c_Reduced, mass);
      pdfConstructor.switchOffDeltaRayPDF();
      auto LL = pdfConstructor.getLogL();
      logLfineScan.push_back(LL.logL);
      nSignalPhotonsFineScan.push_back(pdfConstructor.getExpectedSignalPhotons());
      massPointsFineScan.push_back(mass);
      mass += step;
    }

    scanLikelihood(massPointsFineScan, logLfineScan, 0.5, maxLL, massMax, minMassRange, maxMassRange);


    // finally grab the number of expected photons at the LL maximum
    const PDFConstructor pdfConstructor(trk, Const::pion, PDFConstructor::c_Fine, PDFConstructor::c_Reduced, massMax);
    pdfConstructor.switchOffDeltaRayPDF();
    float nSignalPhotons = pdfConstructor.getExpectedSignalPhotons();
    float nBackgroundPhotons = pdfConstructor.getExpectedBkgPhotons();
    float nDeltaPhotons = pdfConstructor.getExpectedDeltaPhotons();

    topLLScanRes->set(massMax,
                      minMassRange,
                      maxMassRange,
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


