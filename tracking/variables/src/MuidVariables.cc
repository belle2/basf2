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

using namespace std;

namespace Belle2 {
  namespace Variable {

    //! @returns the muon probability stored in the Muid dataobject
    double muidMuonProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getMuonPDFValue();
        }
      }
    }

    //! @returns the pion probability stored in the Muid dataobject
    double muidPionProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getPionPDFValue();
        }
      }
    }

    //! @returns the kaon probability stored in the Muid dataobject
    double muidKaonProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getKaonPDFValue();
        }
      }
    }

    //! @returns the electron probability stored in the Muid dataobject
    double muidElectronProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getElectronPDFValue();
        }
      }
    }

    //! @returns the proton probability stored in the Muid dataobject
    double muidProtonProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getProtonPDFValue();
        }
      }
    }

    //! @returns the deuteron probability stored in the Muid dataobject
    double muidDeuteronProbability(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getDeuteronPDFValue();
        }
      }
    }

    //! @return outcome of the extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit
    double muidOutcomeExtTrack(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getOutcome();
        }
      }
    }

    //! @return outermost EKLM layer crossed by track during extrapolation
    double muidEndcapExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getEndcapExtLayer();
        }
      }
    }

    //! @return outermost BKLM layer crossed by track during extrapolation
    double muidBarrelExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
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
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getExtLayer();
        }
      }
    }

    //! @return outermost EKLM layer with a matching hit
    double muidEndcapHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getEndcapHitLayer();
        }
      }
    }

    //! @return outermost BKLM layer with a matching hit
    double muidBarrelHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
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
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
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
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
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
        return std::numeric_limits<double>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<double>::quiet_NaN();
        else {
          return muid->getHitLayerPattern();
        }
      }
    }

    VARIABLE_GROUP("Muid Calibration");
    REGISTER_VARIABLE("muidMuonProbability", muidMuonProbability,
                      "[calibration] Returns the muon probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidPionProbability", muidPionProbability,
                      "[calibration] Returns the pion probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidKaonProbability", muidKaonProbability,
                      "[calibration] Returns the kaon probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidElectronProbability", muidElectronProbability,
                      "[calibration] Returns the electron probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidProtonProbability", muidProtonProbability,
                      "[calibration] Returns the proton probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidDeuteronProbability", muidDeuteronProbability,
                      "[calibration] Returns the deuteron probability stored in the Muid dataobject");
    REGISTER_VARIABLE("muidOutcomeExtTrack", muidOutcomeExtTrack,
                      "[calibration] Returns outcome of muon extrapolation: 0=missed KLM, 1=BKLM stop, 2=EKLM stop, 3=BKLM exit, 4=EKLM exit");
    REGISTER_VARIABLE("muidEndcapExtLayer", muidEndcapExtLayer,
                      "[calibration] Returns outermost EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidBarrelExtLayer", muidBarrelExtLayer,
                      "[calibration] Returns outermost BKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidExtLayer", muidExtLayer,
                      "[calibration] Returns outermost BKLM or EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidEndcapHitLayer", muidEndcapHitLayer, "[calibration] Returns outermost EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidBarrelHitLayer", muidBarrelHitLayer, "[calibration] Returns outermost BKLM layer with a matching hit");
    REGISTER_VARIABLE("muidHitLayer", muidHitLayer, "[calibration] Return outermost BKLM or EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidExtLayerPattern", muidExtLayerPattern,
                      "[calibration] Returns layer-crossing bit pattern during extrapolation");
    REGISTER_VARIABLE("muidHitLayerPattern", muidHitLayerPattern, "[calibration] Returns matching-hit bit pattern");
  }
}
