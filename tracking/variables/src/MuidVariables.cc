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

    //! @returns a pointer to the related Muid dataobject
    Muid* getMuid(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return nullptr;
      return track->getRelatedTo<Muid>();
    }

    //! @returns the muon probability stored in the Muid dataobject
    double muidMuonProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getMuonPDFValue();
    }

    //! @returns the pion probability stored in the Muid dataobject
    double muidPionProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getPionPDFValue();
    }

    //! @returns the kaon probability stored in the Muid dataobject
    double muidKaonProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getKaonPDFValue();
    }

    //! @returns the electron probability stored in the Muid dataobject
    double muidElectronProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getElectronPDFValue();
    }

    //! @returns the proton probability stored in the Muid dataobject
    double muidProtonProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getProtonPDFValue();
    }

    //! @returns the deuteron probability stored in the Muid dataobject
    double muidDeuteronProbability(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getDeuteronPDFValue();
    }

    //! @returns the muon log-likelihood stored in the Muid dataobject
    double muidMuonLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_mu();
    }

    //! @returns the pion log-likelihood stored in the Muid dataobject
    double muidPionLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_pi();
    }

    //! @returns the kaon log-likelihood stored in the Muid dataobject
    double muidKaonLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_K();
    }

    //! @returns the electron log-likelihood stored in the Muid dataobject
    double muidElectronLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_e();
    }

    //! @returns the proton log-likelihood stored in the Muid dataobject
    double muidProtonLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_p();
    }

    //! @returns the deuteron log-likelihood stored in the Muid dataobject
    double muidDeuteronLogLikelihood(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_d();
    }

    //! @return outcome of the extrapolation: 0=missed KLM, 1=barrel stop, 2=endcap stop, 3=barrel exit, 4=endcap exit
    double muidOutcomeExtTrack(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getOutcome();
    }

    //! @returns chi-squared for this extrapolation
    double muidChiSquared(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getChiSquared();
    }

    //! @returns number of degrees of freedom in chi-squared calculation
    double muidDegreesOfFreedom(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getDegreesOfFreedom();
    }

    //! @return outermost EKLM layer crossed by track during extrapolation
    double muidEndcapExtLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getEndcapExtLayer();
    }

    //! @return outermost BKLM layer crossed by track during extrapolation
    double muidBarrelExtLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getBarrelExtLayer();
    }

    //! @return outermost BKLM or EKLM layer crossed by track during extrapolation
    double muidExtLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getExtLayer();
    }

    //! @return outermost EKLM layer with a matching hit
    double muidEndcapHitLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getEndcapHitLayer();
    }

    //! @return outermost BKLM layer with a matching hit
    double muidBarrelHitLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getBarrelHitLayer();
    }

    //! @return outermost BKLM or EKLM layer with a matching hit
    double muidHitLayer(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getHitLayer();
    }

    //! @return layer-crossing bit pattern during extrapolation
    double muidExtLayerPattern(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getExtLayerPattern();
    }

    //! @return matching-hit bit pattern
    double muidHitLayerPattern(const Particle* particle)
    {
      Muid* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getHitLayerPattern();
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
    REGISTER_VARIABLE("muidMuonLogLikelihood", muidMuonLogLikelihood,
                      "[calibration] Returns the muon log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidPionLogLikelihood", muidPionLogLikelihood,
                      "[calibration] Returns the pion log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidKaonLogLikelihood", muidKaonLogLikelihood,
                      "[calibration] Returns the kaon log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidElectronLogLikelihood", muidElectronLogLikelihood,
                      "[calibration] Returns the electron log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidProtonLogLikelihood", muidProtonLogLikelihood,
                      "[calibration] Returns the proton log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidDeuteronLogLikelihood", muidDeuteronLogLikelihood,
                      "[calibration] Returns the deuteron log-likelihood stored in the Muid dataobject");
    REGISTER_VARIABLE("muidOutcomeExtTrack", muidOutcomeExtTrack,
                      "[calibration] Returns outcome of muon extrapolation: 0=missed KLM, 1=BKLM stop, 2=EKLM stop, 3=BKLM exit, 4=EKLM exit");
    REGISTER_VARIABLE("muidChiSquared", muidChiSquared,
                      "[calibration] Returns chi-squared for this extrapolation");
    REGISTER_VARIABLE("muidDegreesOfFreedom", muidDegreesOfFreedom,
                      "[calibration] Returns number of degrees of freedom in chi-squared calculation");
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
