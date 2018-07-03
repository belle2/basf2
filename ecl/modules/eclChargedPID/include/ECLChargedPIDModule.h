/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Unit.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/chargedPID/ECLAbsPdf.h>
#include <ecl/chargedPID/ECLElectronPdf.h>
#include <ecl/chargedPID/ECLMuonPdf.h>
#include <ecl/chargedPID/ECLPionPdf.h>
#include <ecl/chargedPID/ECLKaonPdf.h>
#include <ecl/chargedPID/ECLProtonPdf.h>

namespace Belle2 {

  namespace ECL {
    class ECLAbsPdf;
  }

  /** The module implements a first version of charged particle identification
      using E/p as discriminating variable.
      For each Track matched with ECLShowers, likelihoods for each particle
      hypothesis are calculated and stored in an ECLPidLikelihood object.
   */
  class ECLChargedPIDModule : public Module {
  public:

    /** Constructor, for setting module description and parameters.
     */
    ECLChargedPIDModule();

    /** Use to clean up anything you created in the constructor.
     */
    virtual ~ECLChargedPIDModule();

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

    /** Clean up anything you created in initialize().
     */
    virtual void terminate();

  private:

    /** StoreArray Track
     */
    StoreArray<Track> m_tracks;

    /** StoreArray ECLPidLikelihood
     */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods;

    /** Array of ECLAbsPdfs
    PDFs are stored for both +/- charge hypotheses, hence the double array structure.
     */
    Belle2::ECL::ECLAbsPdf* m_pdf[2][Const::ChargedStable::c_SetSize];

    /** Max value of Log Likelihood for a particle hypothesis.
    Used when the pdf value is not positive or subnormal.
    */
    static constexpr double m_minLogLike = -700;

    /** Use PDF hypotheses for particles regardless of the sign of charge.
     */
    bool m_useUnsignedParticleHypo;

    /** Apply cluster timing selection.
     */
    bool m_applyClusterTimingSel;

  };

} //Belle2
