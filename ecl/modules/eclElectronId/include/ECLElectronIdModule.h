/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  class Track;
  class ECLPidLikelihood;

  namespace ECL {
    class ECLAbsPdf;
  }

  /** The modules implements a first version of Electron identification
      using the E/p as discriminating variable.
      For each Track matched with ECLShowers Likelihoods for each particle
      hypothesis are calculated and stored in an ECLPidLikelihood object.

   */

  class ECLElectronIdModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLElectronIdModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLElectronIdModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();

  private:
    /** StoreArray Track */
    StoreArray<Track> m_tracks;

    /** StoreArray ECLPidLikelihood */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods;

    Belle2::ECL::ECLAbsPdf* m_pdf[ Const::ChargedStable::c_SetSize ];

    /**  max value of Log Likelihood for a particle hypothesis.
         used when the pdf value is not positive or subnormal.*/
    static constexpr double m_minLogLike = -700;

  };

} //Belle2
