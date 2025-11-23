/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <cdc/dataobjects/CDCDedxLikelihood.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>

#include <string>
#include <cmath>
#include <map>

namespace Belle2 {

  /**
   * a module to fill PIDLikelihoods
   */

  class MdstPIDModule : public Module {
  public:


    /**
     * Constructor
     */
    MdstPIDModule();

    /**
     * Initialize the module.
     * data store registration of PIDLikelihoods and relations to Tracks
     */
    virtual void initialize() override;

    /**
     * Called for each event.
     * loop over Tracks, collect likelihoods and fill PIDLikelihoods
     */
    virtual void event() override;

  private:

    /**
     * Set TOP log likelihoods and corresponding reconstruction flag
     * @param logl TOPLikelihood pointer
     */
    void setLikelihoods(const TOPLikelihood* logl);

    /**
     * Set ARICH log likelihoods and corresponding reconstruction flag
     * @param logl ARICHLikelihood pointer
     */
    void setLikelihoods(const ARICHLikelihood* logl);

    /**
     * Set CDC dE/dx log likelihoods and corresponding reconstruction flag
     * @param logl CDCDedxLikelihood pointer
     */
    void setLikelihoods(const CDCDedxLikelihood* logl);

    /**
     * Set VXD dE/dx log likelihoods and corresponding reconstruction flag
     * @param logl VXDDedxLikelihood pointer
     */
    void setLikelihoods(const VXDDedxLikelihood* logl);

    /**
     * Set ECL log likelihoods and corresponding reconstruction flag
     * @param logl ECLPidLikelihood pointer
     */
    void setLikelihoods(const ECLPidLikelihood* logl);

    /**
     * Set KLM log likelihoods and corresponding reconstruction flag
     * @param muid KLMMuidLikelihood pointer
     */
    void setLikelihoods(const KLMMuidLikelihood* muid);

    /**
     * Get log likelihood for a given particle
     * @param logl detector log likelihoods
     * @param chargedStable particle
     * @return log likelihood
     */
    template<class T>
    float getLogL(const T* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogL(chargedStable);
    }

    /**
     * Get log likelihood for a given particle (ECL specialization)
     * @param logl detector log likelihoods
     * @param chargedStable particle
     * @return log likelihood
     */
    float getLogL(const ECLPidLikelihood* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogLikelihood(chargedStable);
    }

    /**
     * Get log likelihood for a given particle (KLM specialization)
     * @param logl detector log likelihoods
     * @param chargedStable particle
     * @return log likelihood
     */
    float getLogL(const KLMMuidLikelihood* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogL(chargedStable.getPDGCode());
    }

    /**
     * Check for validity of log likelihood values (NaN and +Inf are not allowed).
     * @param logl detector log likelihoods
     * @return true if all likelihoods are valid and not being the same
     */
    template<class T>
    bool areLikelihoodsValid(const T* logl)
    {
      std::vector<float> values;
      for (const auto& chargedStable : Const::chargedStableSet) {
        auto value = getLogL(logl, chargedStable);
        values.push_back(value);
        if (std::isnan(value) or value == INFINITY) {
          B2ERROR("MdstPID::setLikelihoods: invalid " << logl->ClassName() << " for " << m_chargedNames[chargedStable]
                  << ", is " << value
                  << ". Likelihoods ignored.");
          return false;
        }
      }
      // check if log likelihoods differ
      for (auto value : values) {
        if (value != values.back()) return true; // values differ
      }
      return false; // values are all the same - ignore Likelihoods
    }

    // module parameters
    bool m_subtractMaximum; /**< if true subtract the maximum of log likelihoods */

    // input collections
    StoreArray<Track> m_tracks; /**< Required collection of Tracks */
    StoreArray<TOPLikelihood> m_topLikelihoods; /**< Optional collection of TOPLikelihoods */
    StoreArray<ARICHLikelihood> m_arichLikelihoods; /**< Optional collection of ARICHLikelihoods */
    StoreArray<CDCDedxLikelihood> m_cdcDedxLikelihoods; /**< Optional collection of CDCDedxLikelihoods */
    StoreArray<VXDDedxLikelihood> m_vxdDedxLikelihoods; /**< Optional collection of VXDDedxLikelihoods */
    StoreArray<ECLPidLikelihood> m_eclLikelihoods; /**< Optional collection of ECLPidLikelihoods */
    StoreArray<KLMMuidLikelihood> m_muid; /**< Optional collection of KLMMuidLikelihood */

    // output collection
    StoreArray<PIDLikelihood> m_pidLikelihoods; /**< collection of PIDLikelihoods */

    // other
    PIDLikelihood* m_pid; /**< pointer to the object to be filled */
    std::map<Const::ChargedStable, std::string> m_chargedNames; /**< names of charged particles (used in error messages) */

  };

} // Belle2 namespace




