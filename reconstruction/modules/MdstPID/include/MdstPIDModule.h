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

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>

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

    PIDLikelihood* m_pid; /**< pointer to the object to be filled */

  };

} // Belle2 namespace




