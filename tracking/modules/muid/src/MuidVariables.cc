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
    double getMuonPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getMuonPDFValue();
        }
      }
    }


//! @return outcome of this extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit

    double Outcome(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getOutcome();
        }
      }
    }


//! @return outermost BKLM layer crossed by track during extrapolation

    double BarrelExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getBarrelExtLayer();
        }
      }
    }

    //! @return outermost BKLM or EKLM layer crossed by track during extrapolation
    double ExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getExtLayer();
        }
      }
    }

//! @return outermost BKLM layer with a matching hit
    double BarrelHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getBarrelHitLayer();
        }
      }
    }

//! @return outermost BKLM or EKLM layer with a matching hit
    double HitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getHitLayer();
        }
      }
    }

//! @return layer-crossing bit pattern during extrapolation
    double ExtLayerPattern(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getExtLayerPattern();
        }
      }
    }

//! @return matching-hit bit pattern
    double HitLayerPattern(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return NAN;
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return NAN;
        else {
          return muid->getHitLayerPattern();
        }
      }
    }


    // REGISTER_VARIABLE("basf2Name", variableNameAsAbove, "Description");
    VARIABLE_GROUP("KLM Calibration | PID");
    REGISTER_VARIABLE("muonPdf", getMuonPdf , "[calibration] Returns the muon pdf stored in the Muid dataobject");
    REGISTER_VARIABLE("muonOutcome", Outcome ,
                      "[calibration] Return outcome of muon extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit");
    REGISTER_VARIABLE("muonBarrelExtLayer", BarrelExtLayer ,
                      "[calibration] Return outermost BKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muonExtLayer", ExtLayer ,
                      "[calibration] Return outermost BKLM or EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muonBarrelHitLayer", BarrelHitLayer , "[calibration] Return outermost BKLM layer with a matching hit");
    REGISTER_VARIABLE("muonHitLayer", HitLayer , "[calibration] Return outermost BKLM or EKLM layer with a matching hit");
    REGISTER_VARIABLE("muonExtLayerPattern", ExtLayerPattern , "[calibration] Return layer-crossing bit pattern during extrapolation");
    REGISTER_VARIABLE("muonHitLayerPattern", HitLayerPattern , "[calibration] Return matching-hit bit pattern");

  }

  // Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableMuidVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableMuidVariables);
}
