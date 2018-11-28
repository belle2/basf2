/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jan Strube (jan.strube@desy.de)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/gearbox/Const.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPRecBunch.h>

#include <algorithm> // for sort
using namespace std;

namespace Belle2 {
  // contains a couple of helper functions that are related to TOP variables
  namespace topDigitVariables {
    // returns the TOP likelihood that is associated with a given particle
    const TOPLikelihood* getTOPLikelihood(const Particle* particle);

    // returns the ExtHit that is associated with a given particle
    const ExtHit* getExtHit(const Particle* particle);

    // returns the TOP slot ID of the particle
    double getSlotID(const Particle* particle);

    // returns the local coordinate of the particle's entry point to the TOP
    TVector3 getLocalPosition(const Particle* particle);

    // returns the local coordinates of the particles momentum in the TOP
    TVector3 getLocalMomentum(const Particle* particle);

    // counts the number of photons in the TOP in a given time frame
    // if tmin < 0, count from the time of the first photon
    int countHits(const Particle* particle, double tmin, double tmax, bool clean = true);

    // counts the number of photons regardless of hit quality
    int countRawHits(const Particle* particle, double tmin, double tmax);

    // returns the expected number of photons for a given hypothesis
    double getExpectedPhotonCount(const Particle* particle, int pdg = 0);

  }
  namespace Variable {
    //! @returns the number of digits in the same module as the particle
    double topDigitCount(const Particle* particle);

    //! @returns the number of digits in all other module as the particle
    double topBackgroundDigitCount(const Particle* particle);

    //! @returns the number of digits in all other module as the particle
    double topBackgroundDigitCountRaw(const Particle* particle);

    //! @returns the number of all digits regardless of hit quality in the same module as the particle
    double topRawDigitCount(const Particle* particle);

    //! @returns the largest time between to subsequent digits in the same module as the particle
    double topDigitGapSize(const Particle* particle);

    // The number of reflected digits is defined as the number of digits after the gap
    //! @returns the number of reflected digits in the same module as the particle
    double topReflectedDigitCount(const Particle* particle);

    //! @returns the X coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalX(const Particle* particle);

    //! @returns the Y coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalY(const Particle* particle);

    //! @returns the Z coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalZ(const Particle* particle);

    //! @returns the local phi component of the particle's momentum in the TOP
    double getTOPLocalPhi(const Particle* particle);

    //! @returns the local theta component of the particle's momentum in the TOP
    double getTOPLocalTheta(const Particle* particle);

    //! @returns the number of photons associated with the particle likelihood
    double getTOPPhotonCount(const Particle* particle);

    //! @returns the expected number of photons for the assigned hypothesis
    double getExpectedTOPPhotonCount(const Particle* particle, const vector<double>& vars);

    //! @returns the slot ID of the TOP for the particle
    double getTOPSlotID(const Particle* particle);

    //! @returns the number of TOP photons in the given time interval
    double countTOPHitsInInterval(const Particle* particle, const vector<double>& vars);

    //! @returns the number of TOP photons in the first 20 ns
    double countTOPHitsInFirst20ns(const Particle* particle);

    //! @returns the number of raw TOP photons in the given time interval
    double countRawTOPHitsInInterval(const Particle* particle, const vector<double>& vars);

    //---------------- TOPRecBunch related --------------------
    //! @returns whether the rec bunch is reconstructed
    double isTOPRecBunchReconstructed(const Particle* particle);

    //! returns the bunch number. Use -9999 to indicate error
    double TOPRecBunchNumber(const Particle* particle);

    //! returns the current offset
    double TOPRecBunchCurrentOffset(const Particle* particle);

    //! returns the number of tracks in the TOP acceptance
    double TOPRecBunchTrackCount(const Particle* particle);

    //! returns the number of tracks used in the bunch reconstruction
    double TOPRecBunchUsedTrackCount(const Particle* particle);

    //-------------- Event based -----------------------------------
    //! returns the number of photons in a given slot without cleaning
    double TOPRawPhotonsInSlot(const Particle* particle, const vector<double>& vars);

    //! returns the number of good photons in a given slot
    double TOPGoodPhotonsInSlot(const Particle* particle, const vector<double>& vars);
  }
// Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableTOPDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
}
