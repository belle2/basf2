/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecGamma/ECLGammaReconstructorModule.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/dataobjects/ECLGamma.h>

#include <mdst/dataobjects/Track.h>

#include <tracking/dataobjects/ExtHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

#include <TVector3.h>
#include <TMath.h>

#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLGammaReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLGammaReconstructorModule::ECLGammaReconstructorModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLGamma from ECLShower.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("gammaEnergyCut", m_ecut, "gamma enegy threshold ", 0.02);
  addParam("gammaE9o25Cut", m_e925cut, "gamma E9o25 threshold ", 0.75);
  addParam("gammaWidthCut", m_widcut, "gamma Width threshold ", 6.0);
  addParam("gammaNhitsCut", m_nhcut, "gamma Nhits threshold ", 0.);
  addParam("gammaThetaMinCut", m_thetaMin, "gamma theta min threshold ", 17.);
  addParam("gammaThetaMaxCut", m_thetaMax, "gamma theta max threshold ", 150.);


//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));
}


ECLGammaReconstructorModule::~ECLGammaReconstructorModule()
{
}

void ECLGammaReconstructorModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;
  //KM StoreArray<ECLGamma>::registerPersistent();
  //KM RelationArray::registerPersistent<ECLGamma, ECLShower>("", "");

  //Input Array ... moved from event func.
  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  StoreArray<ECLGamma> gammaArray;
  gammaArray.registerInDataStore();
  gammaArray.registerRelationTo(eclRecShowerArray);
}

void ECLGammaReconstructorModule::beginRun()
{
}

void ECLGammaReconstructorModule::event()
{
  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  StoreArray<ECLGamma> gammaArray;
  RelationArray eclGammaToShower(gammaArray, eclRecShowerArray);

  if (!eclRecShowerArray) {
    B2DEBUG(100, "ECLShowers in empty in event " << m_nEvent);
  }
  if (!eclHitAssignmentArray) {
    B2DEBUG(100, "ECLHitAssignment in empty in event " << m_nEvent);
  }

  readExtrapolate();//m_TrackCellId[i] =1 => Extrapolated cell

  for (const auto& eclShower : eclRecShowerArray) {
    const int showerId = eclShower.getShowerId();
    const double energy = eclShower.getEnergy();
    const double theta = eclShower.getTheta();
    const double Theta  = theta * TMath::RadToDeg();
    const double e9oe25 = eclShower.getE9oE25();
    const double width = eclShower.getWidth();
    const double nhit = eclShower.getNHits();
    const int quality = eclShower.getStatus();
    if (quality != 0) continue;
    if (!goodGamma(Theta, energy, nhit, e9oe25, width)) continue;

    bool extMatch = false;

    for (const auto& eclHitAssignment : eclHitAssignmentArray) {
      const auto haShowerId = eclHitAssignment.getShowerId();
      const auto haCellId = eclHitAssignment.getCellId() - 1;

      if (haShowerId != showerId) continue;
      if (haShowerId > showerId) break;
      if (haShowerId == showerId) {
        if (m_TrackCellId[haCellId]) {
          extMatch = true;
          break;
        }
      }//if HAShowerId == ShowerId
    }//for HA hANum

    if (!extMatch) { //no match to track => assign as gamma
      const auto eclGamma = gammaArray.appendNew(showerId);
      eclGamma->addRelationTo(&eclShower);
    }//if !extMatch


  }//for shower hitNum

  m_nEvent++;
}

void ECLGammaReconstructorModule::endRun()
{
  m_nRun++;
}

void ECLGammaReconstructorModule::terminate()
{
}

bool ECLGammaReconstructorModule::goodGamma(double ftheta, double energy, double nhit, double fe9oe25, double fwidth)
{
  if (ftheta < m_thetaMin) return false;
  if (ftheta > m_thetaMax) return false;
  if (energy < m_ecut) return false;

  if (energy < 0.5) {
    if (nhit <= m_nhcut) return false;
    if (fe9oe25 < m_e925cut) return false;
    if (fwidth > m_widcut) return false;
  }
  return true;
}

void ECLGammaReconstructorModule::readExtrapolate()
{
  std::fill_n(m_TrackCellId, 8736, false);

  StoreArray<Track> tracks;
  StoreArray<ExtHit> extHits;

  if (extHits) {
    Const::EDetector myDetID = Const::EDetector::ECL; // ECL in this example
    const int pdgCodePiPlus = Const::pion.getPDGCode();
    const int pdgCodePiMinus = -1 * Const::pion.getPDGCode();

    for (const auto& track : tracks) {
      for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
        if (extHit.getPdgCode() != pdgCodePiPlus and extHit.getPdgCode() != pdgCodePiMinus) continue;
        if ((extHit.getDetectorID() != myDetID) or (extHit.getCopyID() == 0)) continue;
        m_TrackCellId[extHit.getCopyID()] = true;
      } // extHits
    } // tracks
  }
}



