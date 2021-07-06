/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <mdst/dataobjects/Track.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //! @returns a pointer to the related KLMMuidLikelihood dataobject
    KLMMuidLikelihood* getMuid(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return nullptr;
      return track->getRelatedTo<KLMMuidLikelihood>();
    }

    //! @returns the muon probability stored in the KLMMuidLikelihood dataobject
    double muidMuonProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getMuonPDFValue();
    }

    //! @returns the pion probability stored in the KLMMuidLikelihood dataobject
    double muidPionProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getPionPDFValue();
    }

    //! @returns the kaon probability stored in the KLMMuidLikelihood dataobject
    double muidKaonProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getKaonPDFValue();
    }

    //! @returns the electron probability stored in the KLMMuidLikelihood dataobject
    double muidElectronProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getElectronPDFValue();
    }

    //! @returns the proton probability stored in the KLMMuidLikelihood dataobject
    double muidProtonProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getProtonPDFValue();
    }

    //! @returns the deuteron probability stored in the KLMMuidLikelihood dataobject
    double muidDeuteronProbability(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getDeuteronPDFValue();
    }

    //! @returns the muon log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidMuonLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_mu();
    }

    //! @returns the pion log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidPionLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_pi();
    }

    //! @returns the kaon log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidKaonLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_K();
    }

    //! @returns the electron log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidElectronLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_e();
    }

    //! @returns the proton log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidProtonLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_p();
    }

    //! @returns the deuteron log-likelihood stored in the KLMMuidLikelihood dataobject
    double muidDeuteronLogLikelihood(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getLogL_d();
    }

    //! @return outcome of muon extrapolation (see MuidElementNumbers for the complete list of the outcome values)
    double muidOutcomeExtTrack(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getOutcome();
    }

    //! @returns chi-squared for this extrapolation
    double muidChiSquared(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getChiSquared();
    }

    //! @returns number of degrees of freedom in chi-squared calculation
    double muidDegreesOfFreedom(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getDegreesOfFreedom();
    }

    //! @return outermost EKLM layer crossed by track during extrapolation
    double muidEndcapExtLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getEndcapExtLayer();
    }

    //! @return outermost BKLM layer crossed by track during extrapolation
    double muidBarrelExtLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getBarrelExtLayer();
    }

    //! @return outermost BKLM or EKLM layer crossed by track during extrapolation
    double muidExtLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getExtLayer();
    }

    //! @return outermost EKLM layer with a matching hit
    double muidEndcapHitLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getEndcapHitLayer();
    }

    //! @return outermost BKLM layer with a matching hit
    double muidBarrelHitLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getBarrelHitLayer();
    }

    //! @return outermost BKLM or EKLM layer with a matching hit
    double muidHitLayer(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getHitLayer();
    }

    //! @return layer-crossing bit pattern during extrapolation
    double muidExtLayerPattern(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getExtLayerPattern();
    }

    //! @return matching-hit bit pattern
    double muidHitLayerPattern(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getHitLayerPattern();
    }

    //! @return total number of matching BKLM hits
    double muidTotalBarrelHits(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getTotalBarrelHits();
    }

    //! @return total number of matching EKLM hits
    double muidTotalEndcapHits(const Particle* particle)
    {
      KLMMuidLikelihood* muid = getMuid(particle);
      if (!muid)
        return std::numeric_limits<double>::quiet_NaN();
      return muid->getTotalEndcapHits();
    }

    //! @return total number of matching KLM hits
    double muidTotalHits(const Particle* particle)
    {
      return muidTotalBarrelHits(particle) + muidTotalEndcapHits(particle);
    }

    VARIABLE_GROUP("KLMMuidLikelihood calibration");
    REGISTER_VARIABLE("muidMuonProbability", muidMuonProbability,
                      "[Calibration] Returns the muon probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidPionProbability", muidPionProbability,
                      "[Calibration] Returns the pion probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidKaonProbability", muidKaonProbability,
                      "[Calibration] Returns the kaon probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidElectronProbability", muidElectronProbability,
                      "[Calibration] Returns the electron probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidProtonProbability", muidProtonProbability,
                      "[Calibration] Returns the proton probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidDeuteronProbability", muidDeuteronProbability,
                      "[Calibration] Returns the deuteron probability stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidMuonLogLikelihood", muidMuonLogLikelihood,
                      "[Calibration] Returns the muon log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidPionLogLikelihood", muidPionLogLikelihood,
                      "[Calibration] Returns the pion log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidKaonLogLikelihood", muidKaonLogLikelihood,
                      "[Calibration] Returns the kaon log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidElectronLogLikelihood", muidElectronLogLikelihood,
                      "[Calibration] Returns the electron log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidProtonLogLikelihood", muidProtonLogLikelihood,
                      "[Calibration] Returns the proton log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidDeuteronLogLikelihood", muidDeuteronLogLikelihood,
                      "[Calibration] Returns the deuteron log-likelihood stored in the KLMMuidLikelihood dataobject");
    REGISTER_VARIABLE("muidOutcomeExtTrack", muidOutcomeExtTrack,
                      "[Calibration] Returns outcome of muon extrapolation (see MuidElementNumbers for the complete list of the outcome values)");
    REGISTER_VARIABLE("muidChiSquared", muidChiSquared,
                      "[Calibration] Returns chi-squared for this extrapolation");
    REGISTER_VARIABLE("muidDegreesOfFreedom", muidDegreesOfFreedom,
                      "[Calibration] Returns number of degrees of freedom in chi-squared calculation");
    REGISTER_VARIABLE("muidEndcapExtLayer", muidEndcapExtLayer,
                      "[Calibration] Returns outermost EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidBarrelExtLayer", muidBarrelExtLayer,
                      "[Calibration] Returns outermost BKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidExtLayer", muidExtLayer,
                      "[Calibration] Returns outermost BKLM or EKLM layer crossed by track during extrapolation");
    REGISTER_VARIABLE("muidEndcapHitLayer", muidEndcapHitLayer, "[Calibration] Returns outermost EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidBarrelHitLayer", muidBarrelHitLayer, "[Calibration] Returns outermost BKLM layer with a matching hit");
    REGISTER_VARIABLE("muidHitLayer", muidHitLayer, "[Calibration] Return outermost BKLM or EKLM layer with a matching hit");
    REGISTER_VARIABLE("muidExtLayerPattern", muidExtLayerPattern,
                      "[Calibration] Returns layer-crossing bit pattern during extrapolation");
    REGISTER_VARIABLE("muidHitLayerPattern", muidHitLayerPattern, "[Calibration] Returns matching-hit bit pattern");
    REGISTER_VARIABLE("muidTotalBarrelHits", muidTotalBarrelHits, "[Calibration] Returns total number of matching BKLM hits");
    REGISTER_VARIABLE("muidTotalEndcapHits", muidTotalEndcapHits, "[Calibration] Returns total number of matching EKLM hits");
    REGISTER_VARIABLE("muidTotalHits", muidTotalHits, "[Calibration] Returns total number of matching KLM hits");
  }
}
