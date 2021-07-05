/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <tuple>
#include <utility>

namespace Belle2 {

  /**
   * Class to store pixel-by-pixel likelihoods for a track
   * relation from Tracks
   * filled in top/modules/TOPPDFDebugger/src/TOPPDFDebuggerModule.cc
   */

  class TOPPixelLikelihood : public RelationsObject {

  public:

    /**
     * Array of length 512 to hold per-pixel information
     */
    typedef std::array<float, 512> PixelArray_t;

    /**
     * Default constructor
     */
    TOPPixelLikelihood() { }

    /**
     * Adds the likelihood array for the given hypothesis (PDG code)
     * @param plkhs array of pixel likelihoods
     * @param hypothesis PDG code of hypothesis
     */
    bool addHypothesisLikelihoods(const PixelArray_t& plkhs, const int hypothesis)
    {
      auto result = m_logls.insert(std::make_pair(hypothesis, plkhs));
      if (not result.second) {
        B2WARNING("Likelihoods already exist for this track");
      }
      return result.second;
    }

    /**
     * Adds the signal photon array for the given hypothesis (PDG code)
     * @param sfots array of per-pixel expected signal photon counts
     * @param hypothesis PDG code of hypothesis
     */
    bool addHypothesisSignalPhotons(const PixelArray_t& sfots, const int hypothesis)
    {
      auto result = m_sfots.insert(std::make_pair(hypothesis, sfots));
      if (not result.second) {
        B2WARNING("Signal photons already exist for this track");
      }
      return result.second;
    }

    /**
     * Returns the pixel likelihoods for the given hypothesis (PDG code)
     * @param hypothesis PDG code of desired hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getHypothesisLikelihoods(const int hypothesis) const
    {
      return m_logls.at(hypothesis);
    }

    /**
     * Returns the pixel signal photons for the given hypothesis (PDG code)
     * @param hypothesis PDG code of desire hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getHypothesisSignalPhotons(const int hypothesis) const
    {
      return m_sfots.at(hypothesis);
    }

    /**
     * Returns the pixel likelihoods for the electron hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getPixelLogL_e() const { return m_logls.at(11); }

    /**
     * Returns the pixel likelihoods for the muon hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getPixelLogL_mu() const { return m_logls.at(13); }

    /**
     * Returns the pixel likelihoods for the pion hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getPixelLogL_pi() const { return m_logls.at(211); }

    /**
     * Returns the pixel likelihoods for the kaon hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getPixelLogL_K() const { return m_logls.at(321); }

    /**
     * Returns the pixel likelihoods for the proton hypothesis
     * @return array of pixel log likelihoods
     */
    const PixelArray_t& getPixelLogL_p() const { return m_logls.at(2212); }

    /**
     * Returns the pixel signal photons for the electron hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getPixelSigPhot_e() const { return m_sfots.at(11); }

    /**
     * Returns the pixel signal photons for the muon hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getPixelSigPhot_mu() const { return m_sfots.at(13); }

    /**
     * Returns the pixel signal photons for the pion hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getPixelSigPhot_pi() const { return m_sfots.at(211); }

    /**
     * Returns the pixel signal photons for the kaon hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getPixelSigPhot_K() const { return m_sfots.at(321); }

    /**
     * Returns the pixel signal photons for the proton hypothesis
     * @return array of per-pixel expected signal photon counts
     */
    const PixelArray_t& getPixelSigPhot_p() const { return m_sfots.at(2212); }

    /**
     * Sets module ID of the associated exthit
     * @param moduleID the module ID of associated exthit
     */
    void setModuleID(int moduleID)
    {
      m_moduleID = moduleID;
    }

    /**
     * Returns module ID of the associated exthit
     */
    int getModuleID() const { return m_moduleID; }

  private:
    std::map<int, PixelArray_t> m_logls; /**< pixel likelihoods for different hypotheses */
    std::map<int, PixelArray_t> m_sfots; /**< pixel sigphot counts for different hypotheses */
    int m_moduleID = 0; /**< slot ID of the exthit */
    ClassDef(TOPPixelLikelihood, 3); /**< ClassDef */
  };
} // end namespace Belle2

