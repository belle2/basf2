/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTPIDMODULE
#define MDSTPIDMODULE

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <tracking/dataobjects/Muid.h>

#include <string>

namespace Belle2 {

  class PIDLikelihood;
  class TOPLikelihood;
  class ARICHLikelihood;
  class CDCDedxLikelihood;
  class VXDDedxLikelihood;
  class ECLPidLikelihood;
  class Muid;

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
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     * loop over Tracks, collect likelihoods and fill PIDLikelihoods
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

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
    StoreArray<Muid> m_muid; /**< Optional array for Muid */

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
     * @param muid Muid pointer
     */
    void setLikelihoods(const Muid* muid);

    PIDLikelihood* m_pid; /**< pointer to the object to be filled */

  };

} // Belle2 namespace

#endif




