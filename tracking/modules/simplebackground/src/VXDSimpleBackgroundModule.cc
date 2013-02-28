/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/simplebackground/VXDSimpleBackgroundModule.h>
#include <vxd/dataobjects/VXDSimpleDigiHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <generators/dataobjects/MCParticle.h>

#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

//C++ std lib
#include <vector>
using std::vector;
#include <fstream>
using std::ofstream;
#include <cmath>
using std::sin;
using std::cos;
using std::sqrt;

#include <iostream>
using std::cerr;
using std::endl;


//root stuff
#include <TRandom.h>
//genfit stuff
#include <GFTrackCand.h>

using namespace Belle2;


REG_MODULE(VXDSimpleBackground);

VXDSimpleBackgroundModule::VXDSimpleBackgroundModule() : Module()
{
  setDescription("The VXDSimpleBackgroundModule generate a dummy backround and measurment outliers for Si tracking. Works only with single track events");

  addParam("outlierRatio", m_outlierRatio, "ratio of measurement outliers", 0.0);
  addParam("outlierShift", m_outlierShift, "if positive value is set outliers will be created by a constant shift away from the true hit with given value. If negative outliers will come from a Gaussion distibution", -1.0);
  addParam("sigmaFactorU", m_sigmaFactorU, "factor to create outlier sigma by multiplication with normal u sigma ", 1.0);
  addParam("sigmaFactorV", m_sigmaFactorV, "factor to create outlier sigma by multiplication with normal v sigma ", 1.0);
  addParam("energyThreshold", m_energyThreshold, "particles with energy deposit lower then this will not create a digiHit (GeV)", 5E-6);
  addParam("onlyPrimaries", m_onlyPrimaries, "if true use only primary particles from the generator no particles created by Geant4", false);
  addParam("outlierLayers", m_outlierLayers, "choose the layers that should have outliers. By default all layers will have measurement outliers", vector<bool>(6, true));
  addParam("backroundLayers", m_backroundLayers, "choose the layers that should have additional background hits. By default all layers will have background outliers", vector<bool>(6, true));
  addParam("backgroundRatio2", m_backgroundRatio2, "ratio of trueHits that already have one background hit will be accaponied by one additional dummy background hit (if backroundLayers is 0 this option has no effect)", 0.0);
  addParam("backgroundRatio", m_backgroundRatio, "ratio of dummy background hits ", 0.0);

  addParam("semiAxisFactorU", m_semiAxisFactorU, "factor multiplied to the u sigma to get a maximal semi axis for the creation of background hits", 1.0);
  addParam("semiAxisFactorV", m_semiAxisFactorV, "factor multiplied to the v sigma to get a maximal semi axis for the creation of background hits", 1.0);

  addParam("writeTruthToFile", m_writeTruthToFile, "write the u and v coordinate of the trueHits of one track to a text file", false);
  addParam("randomRadius", m_randomRadius, "true if a background hit is created somewhere inside the radius around a trueHit. false if background hit is created exactly on this circle", false);
  addParam("setMeasSigma", m_setMeasSigma, "if positive value (in cm) is given it will be used as the sigma to smear the VXDSimpleDigiHits otherwise pitch/sqrt(12) will be used", -1.0);
  addParam("backgroundAngle", m_backgroundAngle, "if positive value (in rad) is given it will be used as the direction with respect to a true hit where a background hit will be created. If negative a random angle will be chosen", -1.0);

  addParam("only6", m_only6, "throw away tracks with do not have exactly 1 true hit in every Si layer", false);

  addParam("outliersPerTrack", m_outliersPerTrack, "state how many true hits of one track will become outlier hits. This option overwrites the outlierRatio and outlierLayers option", -1);

}

VXDSimpleBackgroundModule::~VXDSimpleBackgroundModule()
{
}

void VXDSimpleBackgroundModule::initialize()
{
  //output containers
  StoreArray<VXDSimpleDigiHit>::registerPersistent("pxdSimpleDigiHits");
  StoreArray<VXDSimpleDigiHit>::registerPersistent("svdSimpleDigiHits");
  StoreArray<GFTrackCand>::registerPersistent();
  m_notPerfectCounter = 0;
}

void VXDSimpleBackgroundModule::beginRun()
{

}

void VXDSimpleBackgroundModule::event()
{
  ofstream dataOut;
  if (m_writeTruthToFile == true) dataOut.open("data.txt");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "*******  VXDSimpleBackgroundModule processing event number: " << eventCounter << " *******");
  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles("");
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(100, "VXDSimpleBackgroundModule: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) {B2DEBUG(100, "MCTrackFinder: MCParticlesCollection is empty!");}
  //PXD
  StoreArray<PXDTrueHit> pxdTrueHits("");
  int nPxdTrueHits = pxdTrueHits.getEntries();
  B2DEBUG(100, "VXDSimpleBackgroundModule: Number of PXDHits: " << nPxdTrueHits);
  if (nPxdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: PXDHitsCollection is empty!");}

  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;
  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;

  //SVD
  StoreArray<SVDTrueHit> svdTrueHits("");
  int nSvdTrueHits = svdTrueHits.getEntries();
  B2DEBUG(100, "VXDSimpleBackgroundModule: Number of SVDDHits: " << nSvdTrueHits);
  if (nSvdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: SVDHitsCollection is empty!");}



  //output containers
  StoreArray<VXDSimpleDigiHit> pxdSimpleDigiHits("pxdSimpleDigiHits");
  StoreArray<VXDSimpleDigiHit> svdSimpleDigiHits("svdSimpleDigiHits");
  StoreArray<GFTrackCand> trackCandidates;

  MCParticle* aMcParticle = mcParticles[0];

  // if option is set ignore every track that does not have exactly 1 hit in every Si layer
  bool filterEvent = false;
  if (m_only6 == true) {

    vector<int> layerIds;
    RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getElementsFrom(aMcParticle);
    while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
      int layerId = iterPairMcPxd.first->to->getSensorID().getLayerNumber();
      layerIds.push_back(layerId);
      ++iterPairMcPxd.first;
    }
    RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getElementsFrom(aMcParticle);
    while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
      int layerId = iterPairMcSvd.first->to->getSensorID().getLayerNumber();
      layerIds.push_back(layerId);
      ++iterPairMcSvd.first;
    }
    if (layerIds.size() not_eq 6) {
      filterEvent = true;
      ++m_notPerfectCounter;
    } else {
      sort(layerIds.begin(), layerIds.end());
      for (int l = 0; l not_eq 6; ++l) {
        if (l + 1 not_eq layerIds[l]) {
          filterEvent = true;
          ++m_notPerfectCounter;
        }
      }
    }
  }


  if (filterEvent == false) {

    //determin witch layers will have an outlier hit ( if outliersPerTrack is set)
    if (m_outliersPerTrack > 0) {
      m_outlierLayers.clear();
      m_outlierLayers.resize(6, false);
      m_outlierRatio = 1.1;

      for (int i = 0; i not_eq m_outliersPerTrack; ++i) {
        int lNew = gRandom->Integer(6);
        if (m_outlierLayers[lNew] == true) {
          --i;
        }
        m_outlierLayers[lNew] = true;
      }
    }


    GFTrackCand* trackCand = trackCandidates.appendNew();


    trackCand->setMcTrackId(0);//Save the MCParticleID in the TrackCandidate
    TVector3 position = aMcParticle->getProductionVertex();
    TVector3 momentum = aMcParticle->getMomentum();

    TVectorD stateSeed(6);
    TMatrixDSym covSeed(6);
    covSeed.Zero(); // just to be save
    stateSeed(0) = position[0]; stateSeed(1) = position[1]; stateSeed(2) = position[2];
    stateSeed(3) = momentum[0]; stateSeed(4) = momentum[1]; stateSeed(5) = momentum[2];
    covSeed(0, 0) = 1; covSeed(1, 1) = 1; covSeed(2, 2) = 2 * 2;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.2 * 0.2;
    //Finally set the complete track seed
    trackCand->set6DSeedAndPdgCode(stateSeed, aMcParticle->getPDG(), covSeed);


    double sigmaU = m_setMeasSigma;
    double sigmaV = m_setMeasSigma;

    //bool skipHit = false;
    //smear the pxd true Hits and store them in pxdSimpleDigiHits
    int iDigiHit = 0;
    for (int i = 0; i not_eq nPxdTrueHits; ++i) {
      const PXDTrueHit* aPxdTrueHit = pxdTrueHits[i];
      float energy = aPxdTrueHit->getEnergyDep();
      if (energy < m_energyThreshold) { //ignore hit if energy deposit is too small
        continue;
      }

      if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getElementsFrom(aMcParticle);
        while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
          if (iterPairMcPxd.first->to == aPxdTrueHit) {
            break;
          }
          ++iterPairMcPxd.first;
        }
        if (iterPairMcPxd.first == iterPairMcPxd.second) { // if the beak statement never was invoced the current pxdHit does not come from a primary particle and will not be added to the track cand
          continue;
        }
      }


      VxdID aVXDId = aPxdTrueHit->getSensorID();
      float uTrue = aPxdTrueHit->getU();
      float vTrue = aPxdTrueHit->getV();
      float u = -20;
      float v = -20;
      if (m_setMeasSigma < 0.0) {
        const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
        sigmaU = geometry.getUPitch(uTrue) / sqrt(12);
        sigmaV = geometry.getVPitch(vTrue) / sqrt(12);

      }
      B2DEBUG(100, "sigU sigV: " << sigmaU << " " << sigmaV);
      bool noOutlier = true;

      if (m_outlierLayers[aVXDId.getLayerNumber() - 1] == true and gRandom->Uniform() < m_outlierRatio) { //Create a measurement outlier

        if (m_outlierShift > 0.0) {
          u = uTrue + m_outlierShift;
          v = vTrue + m_outlierShift;
        } else {
          u = gRandom->Gaus(uTrue, sigmaU * m_sigmaFactorU);
          v = gRandom->Gaus(vTrue, sigmaV * m_sigmaFactorV);
        }

        noOutlier = false;
        B2DEBUG(100, "pxd loop gRandom->Uniform() < m_outlierRatio");

      } else { //make a normal measurment
        u = gRandom->Gaus(uTrue, sigmaU);
        v = gRandom->Gaus(vTrue, sigmaV);
        noOutlier = true;
        B2DEBUG(100, "pxd loop not gRandom->Uniform() < m_outlierRatio");
      }
      if (m_writeTruthToFile == true) dataOut << uTrue << "\t" << vTrue << "\n";

      new(pxdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, static_cast<const VXDTrueHit*>(aPxdTrueHit), noOutlier);

      float time = pxdTrueHits[i]->getGlobalTime();
      int uniqueSensorId = aVXDId.getID();
      trackCand->addHit(Const::PXD, iDigiHit, uniqueSensorId, double(time));
      ++iDigiHit;
      if (m_backroundLayers[aVXDId.getLayerNumber() - 1] == true and gRandom->Uniform() <= m_backgroundRatio) {
        double randomAngle = -2.0;
        if (m_backgroundAngle > 0.0) {
          randomAngle = m_backgroundAngle; //set a constant angle
        } else { //set a random angle
          randomAngle = gRandom->Uniform(2.0 * TMath::Pi());
        }
        double randomFactor = 1.0;
        double uSemiAxis = m_semiAxisFactorU * sigmaU;
        double vSemiAxis = m_semiAxisFactorV * sigmaV;

        if (m_randomRadius == true) {
          randomFactor = gRandom->Uniform();
        }
        u = uTrue + randomFactor * uSemiAxis * sin(randomAngle);
        v = vTrue + randomFactor * vSemiAxis * cos(randomAngle);
        new(pxdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, NULL, false);
        trackCand->addHit(Const::PXD, iDigiHit, uniqueSensorId, double(time));
        ++iDigiHit;
        if (gRandom->Uniform() <= m_backgroundRatio2) {   // add a second BG hit
          randomAngle = -2.0;
          if (m_backgroundAngle > 0.0) {
            randomAngle = m_backgroundAngle; //set a constant angle
          } else { //set a random angle
            randomAngle = gRandom->Uniform(2.0 * TMath::Pi());
          }
          randomFactor = 1.0;
          uSemiAxis = m_semiAxisFactorU * sigmaU;
          vSemiAxis = m_semiAxisFactorV * sigmaV;

          if (m_randomRadius == true) {
            randomFactor = gRandom->Uniform();
          }
          u = uTrue + randomFactor * uSemiAxis * sin(randomAngle);
          v = vTrue + randomFactor * vSemiAxis * cos(randomAngle);
          new(pxdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, NULL, false);
          trackCand->addHit(Const::PXD, iDigiHit, uniqueSensorId, double(time));
          ++iDigiHit;
        }
      }

    }
////////////////////////////////////////////////  NOW THE SVD
    //smear the svd true Hits and store them in svdSimpleDigiHits
    iDigiHit = 0;
    for (int i = 0; i not_eq nSvdTrueHits; ++i) {
      const SVDTrueHit* aSvdTrueHit = svdTrueHits[i];
      float energy = aSvdTrueHit->getEnergyDep();
      if (energy < m_energyThreshold) { //ignore hit if energy deposity is too snall
        continue;
      }

      if (m_onlyPrimaries == true) {
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getElementsFrom(aMcParticle);
        while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
          if (iterPairMcSvd.first->to == aSvdTrueHit) {
            break;
          }
          ++iterPairMcSvd.first;
        }
        if (iterPairMcSvd.first == iterPairMcSvd.second) { // if the beak statement never was invoced the current svdHit does not come from a primary particle and will not be added to the track cand
          continue;
        }
      }

      VxdID aVXDId = aSvdTrueHit->getSensorID();
      float uTrue = aSvdTrueHit->getU();
      float vTrue = aSvdTrueHit->getV();
      float u = -20;
      float v = -20;
      if (m_setMeasSigma < 0.0) {
        const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
        sigmaU = geometry.getUPitch(uTrue) / sqrt(12);
        sigmaV = geometry.getVPitch(vTrue) / sqrt(12);
      }
      B2DEBUG(100, "sigU sigV: " << sigmaU << " " << sigmaV);
      bool noOutlier = true;

      if (m_outlierLayers[aVXDId.getLayerNumber() - 1] == true and gRandom->Uniform() < m_outlierRatio) { //Create a measurement outlier
        if (m_outlierShift > 0.0) {
          u = uTrue + m_outlierShift;
          v = vTrue + m_outlierShift;
        } else {
          u = gRandom->Gaus(uTrue, sigmaU * m_sigmaFactorU);
          v = gRandom->Gaus(vTrue, sigmaV * m_sigmaFactorV);
        }
        noOutlier = false;
        B2DEBUG(100, "svd loop gRandom->Uniform() < m_outlierRatio");


      }   else { //make a normal measurment
        u = gRandom->Gaus(uTrue, sigmaU);
        v = gRandom->Gaus(vTrue, sigmaV);
        noOutlier = true;
        B2DEBUG(100, "svd loop not gRandom->Uniform() < m_outlierRatio");
      }
      if (m_writeTruthToFile == true) dataOut << uTrue << "\t" << vTrue << "\n";
      new(svdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, static_cast<const VXDTrueHit*>(aSvdTrueHit), noOutlier);
      float time = svdTrueHits[i]->getGlobalTime();
      int uniqueSensorId = aVXDId.getID();
      trackCand->addHit(Const::SVD, iDigiHit, uniqueSensorId, double(time));
      ++iDigiHit;

      if (m_backroundLayers[aVXDId.getLayerNumber() - 1] == true and gRandom->Uniform() < m_backgroundRatio) {
        double randomAngle = -2.0;
        if (m_backgroundAngle > 0.0) {
          randomAngle = m_backgroundAngle; //set a constant angle
        } else { //set a random angle
          randomAngle = gRandom->Uniform(2.0 * TMath::Pi());
        }
        double randomFactor = 1.0;
        double uSemiAxis = m_semiAxisFactorU * sigmaU;
        double vSemiAxis = m_semiAxisFactorV * sigmaV;

        if (m_randomRadius == true) {
          randomFactor = gRandom->Uniform();
        }
        u = uTrue + randomFactor * uSemiAxis * sin(randomAngle);
        v = vTrue + randomFactor * vSemiAxis * cos(randomAngle);
        new(svdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, NULL, false);
        trackCand->addHit(Const::SVD, iDigiHit, uniqueSensorId, double(time));
        ++iDigiHit;
        if (gRandom->Uniform() <= m_backgroundRatio2) {   // add a second BG hit
          randomAngle = -2.0;
          if (m_backgroundAngle > 0.0) {
            randomAngle = m_backgroundAngle; //set a constant angle
          } else { //set a random angle
            randomAngle = gRandom->Uniform(2.0 * TMath::Pi());
          }
          randomFactor = 1.0;
          uSemiAxis = m_semiAxisFactorU * sigmaU;
          vSemiAxis = m_semiAxisFactorV * sigmaV;

          if (m_randomRadius == true) {
            randomFactor = gRandom->Uniform();
          }
          u = uTrue + randomFactor * uSemiAxis * sin(randomAngle);
          v = vTrue + randomFactor * vSemiAxis * cos(randomAngle);
          new(svdSimpleDigiHits.nextFreeAddress()) VXDSimpleDigiHit(aVXDId, u, v, sigmaU, sigmaV, NULL, false);
          trackCand->addHit(Const::SVD, iDigiHit, uniqueSensorId, double(time));
          ++iDigiHit;
        }
      }
    }


    trackCand->sortHits();


    if (m_writeTruthToFile == true) dataOut.close();
  }
  B2DEBUG(100, "pxdSimpleDigiHits.getEntries()" << pxdSimpleDigiHits.getEntries());
  B2DEBUG(100, "svdSimpleDigiHits.getEntries()" << svdSimpleDigiHits.getEntries());


}

void VXDSimpleBackgroundModule::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " tracks had not exactly one true hit in every layer and were not passed to the Fitter module");
  }
}

void VXDSimpleBackgroundModule::terminate()
{

}

