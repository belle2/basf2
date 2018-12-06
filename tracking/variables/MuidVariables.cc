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

    //! @returns the muon pdf stored in the Muid dataobject (normalized with all others)
    double muonPdf(const Particle* particle)
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

    //! @returns the pion pdf stored in the Muid dataobject (normalized with all others)
    double pionPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getPionPDFValue();
        }
      }
    }

    //! @returns the kaon pdf stored in the Muid dataobject (normalized with all others)
    double kaonPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getKaonPDFValue();
        }
      }
    }

    //! @returns the electron pdf stored in the Muid dataobject (normalized with all others)
    double electronPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getElectronPDFValue();
        }
      }
    }

    //! @returns the proton pdf stored in the Muid dataobject (normalized with all others)
    double protonPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
        else {
          return muid->getProtonPDFValue();
        }
      }
    }

    //! @returns the deuteron pdf stored in the Muid dataobject (normalized with all others)
    double deuteronPdf(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
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

    //! @return outermost EKLM layer crossed by track during extrapolation
    double muidEndcapExtLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
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

    //! @return outermost EKLM layer with a matching hit
    double muidEndcapHitLayer(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<float>::quiet_NaN();
      else {
        Muid* muid = track->getRelatedTo<Muid>();
        if (!muid)
          return std::numeric_limits<float>::quiet_NaN();
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

    VARIABLE_GROUP("KLM Calibration | PID");
    REGISTER_VARIABLE("muonPdf", muonPdf ,
                      "[calibration] Returns the muon pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("pionPdf", pionPdf ,
                      "[calibration] Returns the pion pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("kaonPdf", kaonPdf ,
                      "[calibration] Returns the kaon pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("electronPdf", electronPdf ,
                      "[calibration] Returns the electron pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("protonPdf", protonPdf ,
                      "[calibration] Returns the proton pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("deuteronPdf", deuteronPdf ,
                      "[calibration] Returns the deuteron pdf stored in the Muid dataobject (normalized with all others)");
    REGISTER_VARIABLE("muidOutcomeExtTrack", muidOutcomeExtTrack ,
                      "[calibration] Return outcome of muon extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit");
    REGISTER_VARIABLE("muidEndcapExtLayer", muidEndcapExtLayer ,
                      "[calibration] Return outermost EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidBarrelExtLayer", muidBarrelExtLayer ,
                      "[calibration] Return outermost BKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidExtLayer", muidExtLayer ,
                      "[calibration] Return outermost BKLM or EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidEndcapHitLayer", muidEndcapHitLayer , "[calibration] Return outermost EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidBarrelHitLayer", muidBarrelHitLayer , "[calibration] Return outermost BKLM layer with a matching hit");
    REGISTER_VARIABLE("muidHitLayer", muidHitLayer , "[calibration] Return outermost BKLM or EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidExtLayerPattern", muidExtLayerPattern ,
                      "[calibration] Return layer-crossing bit pattern during extrapolation");
    REGISTER_VARIABLE("muidHitLayerPattern", muidHitLayerPattern , "[calibration] Return matching-hit bit pattern");
  }
}
