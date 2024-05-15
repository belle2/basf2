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
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>
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
     * Destructor
     */
    virtual ~MdstPIDModule();

    /**
     * Initialize the module.
     * data store registration of PIDLikelihoods and relations to Tracks
     */
    virtual void initialize() override;

    /**
     * Called when a new run is started.
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     * loop over Tracks, collect likelihoods and fill PIDLikelihoods
     */
    virtual void event() override;

    /**
     * Called when run ended.
     */
    virtual void endRun() override;

    /**
     * Terminates the module.
     */
    virtual void terminate() override;

  private:

    // required input
    StoreArray<Track> m_tracks; /**< Required array for Tracks */
    StoreArray<PIDLikelihood> m_pidLikelihoods; /**< Required array for PIDLikelihoods */

    // optional input
    StoreArray<TOPLikelihood> m_topLikelihoods; /**< Optional array for TOPLikelihoods */
    StoreArray<ARICHLikelihood> m_arichLikelihoods; /**< Optional array for ARICHLikelihoods */
    StoreArray<CDCDedxLikelihood> m_cdcDedxLikelihoods; /**< Optional array for CDCDedxLikelihoods */
    StoreArray<VXDDedxLikelihood> m_vxdDedxLikelihoods; /**< Optional array for VXDDedxLikelihoods */
    StoreArray<ECLPidLikelihood> m_eclLikelihoods; /**< Optional array for ECLPidLikelihoods */
    StoreArray<KLMMuidLikelihood> m_muid; /**< Optional array for KLMMuidLikelihood */

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
    double getLogL(const T* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogL(chargedStable);
    }

    /**
     * Get log likelihood for a given particle (ECL specialization)
     * @param logl detector log likelihoods
     * @param chargedStable particle
     * @return log likelihood
     */
    double getLogL(const ECLPidLikelihood* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogLikelihood(chargedStable);
    }

    /**
     * Get log likelihood for a given particle (KLM specialization)
     * @param logl detector log likelihoods
     * @param chargedStable particle
     * @return log likelihood
     */
    double getLogL(const KLMMuidLikelihood* logl, const Const::ChargedStable& chargedStable) const
    {
      return logl->getLogL(chargedStable.getPDGCode());
    }

    /**
     * Check for validity of log likelihood values
     * @param logl detector log likelihoods
     * @return true if all likelihoods valid
     */
    template<class T>
    bool areLikelihoodsValid(const T* logl)
    {
      for (const auto& chargedStable : Const::chargedStableSet) {
        auto value = getLogL(logl, chargedStable);
        if (isnan(value) or value == INFINITY) {
          B2ERROR("MdstPID::setLikelihoods: invalid " << logl->ClassName() << " for " << m_chargedNames[chargedStable]
                  << ", is " << value
                  << ". Likelihoods ignored.");
          return false;
        }
      }
      return true;
    }

    PIDLikelihood* m_pid; /**< pointer to the object to be filled */
    std::map<Const::ChargedStable, std::string> m_chargedNames; /**< names of charged particles */

  };

} // Belle2 namespace




