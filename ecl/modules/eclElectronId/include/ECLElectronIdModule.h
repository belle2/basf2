/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLELECTRONIDMODULE_H
#define ECLELECTRONIDMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {

  /** The module implements a first version of charged particle identification
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
    //    double likelihood( const Const::ChargedStable& hyp, double eop) const;
    Belle2::ECL::ECLAbsPdf* m_pdf[ Const::ChargedStable::c_SetSize ];
    // max value of Log Likelihood for a particle hypothesis.
    // used when the pdf value is not positive or subnormal.
    static constexpr double m_minLogLike = -700;

    /** Use PDF hypotheses for anti-particles */
    bool m_useAntiParticleHypo;

  };

} //Belle2
#endif
