/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alberto Martini                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

// Belle II dataobjects include
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/Muid.h>

// ROOT include
#include <TLorentzVector.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

//! @returns the muon pdf stored in the Muid dataobject
    double muidPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getMuonPDFValue();
        }
      }
    }


//! @return outcome of the extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit

    double muidOutcomeExtTrack(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getOutcome();
        }
      }
    }


//! @return outermost BKLM layer crossed by track during extrapolation

    double muidBarrelExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getBarrelExtLayer();
        }
      }
    }

    //! @return outermost BKLM or EKLM layer crossed by track during extrapolation
    double muidExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getExtLayer();
        }
      }
    }

//! @return outermost BKLM layer with a matching hit
    double muidBarrelHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getBarrelHitLayer();
        }
      }
    }

//! @return outermost BKLM or EKLM layer with a matching hit
    double muidHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getHitLayer();
        }
      }
    }

//! @return layer-crossing bit pattern during extrapolation
    double muidExtLayerPattern(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getExtLayerPattern();
        }
      }
    }

//! @return matching-hit bit pattern
    double muidHitLayerPattern(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getHitLayerPattern();
        }
      }
    }


    // REGISTER_VARIABLE("basf2Name", variableNameAsAbove, "Description");
    VARIABLE_GROUP("KLM Calibration | PID");
    REGISTER_VARIABLE("muidPdf", muidPdf , "[calibration] Returns the muon pdf stored in the Muid dataobject");
    REGISTER_VARIABLE("muidOutcomeExtTrack", muidOutcomeExtTrack ,
                      "[calibration] Return outcome of muon extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit");
    REGISTER_VARIABLE("muidBarrelExtLayer", muidBarrelExtLayer ,
                      "[calibration] Return outermost BKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidExtLayer", muidExtLayer ,
                      "[calibration] Return outermost BKLM or EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidBarrelHitLayer", muidBarrelHitLayer , "[calibration] Return outermost BKLM layer with a matching hit");
    REGISTER_VARIABLE("muidHitLayer", muidHitLayer , "[calibration] Return outermost BKLM or EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidExtLayerPattern", muidExtLayerPattern ,
                      "[calibration] Return layer-crossing bit pattern during extrapolation");
    REGISTER_VARIABLE("muidHitLayerPattern", muidHitLayerPattern , "[calibration] Return matching-hit bit pattern");

  }

  // Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableMuidVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableMuidVariables);
}
