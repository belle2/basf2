/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019, 2021 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <top/utilities/TrackSelector.h>
#include <top/reconstruction_cpp/ModuleAlignment.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>

#include <vector>
#include <string>


namespace Belle2 {

  /**
   * Collector for geometrical alignment of a TOP module with dimuons or Bhabhas.
   * Iterative alignment procedure is performed here, algorithm just collects the results.
   *
   * For the procedure see M. Staric, NIMA 876 (2017) 260-264.
   */
  class TOPAlignmentCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPAlignmentCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    /** Enumerator for sizes */
    enum {c_numSets = 2,  /**< number of statistically independent subsamples */
         };

    // steering parameters
    int m_targetMid; /**< target module to align. Must be 1 <= Mid <= 16 */
    int m_maxFails; /**< maximum allowed number of failed iterations */
    std::string m_sample; /**< sample type */
    double m_deltaEcms; /**< c.m.s energy window if sample is "dimuon" or "bhabha" */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    double m_stepPosition; /**< step size for translations */
    double m_stepAngle; /**< step size for rotations */
    double m_stepTime; /**< step size for t0 */
    std::vector<double> m_parInit; /**< initial parameter values */
    std::vector<std::string> m_parFixed; /**< names of parameters to be fixed */

    // alignment procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    std::vector<TOP::ModuleAlignment> m_align;  /**< alignment objects */
    std::vector<int> m_countFails;      /**< counters for failed iterations */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // tree variables
    std::vector<std::string> m_treeNames; /**< tree names */
    int m_iter = 0;  /**< iteration counter */
    int m_ntrk = 0;  /**< number of tracks used */
    int m_errorCode = 0;  /**< error code of the alignment procedure */
    std::vector<float> m_vAlignPars;     /**< alignment parameters */
    std::vector<float> m_vAlignParsErr;  /**< error on alignment parameters */
    bool m_valid = false;  /**< true if alignment parameters are valid */
    int m_numPhot = 0; /**< number of photons used for log likelihood in this iteration */
    float m_x = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_y = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_z = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_p = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_theta = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_phi = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_pocaR = 0; /**< r of POCA */
    float m_pocaZ = 0; /**< z of POCA */
    float m_pocaX = 0; /**< x of POCA */
    float m_pocaY = 0; /**< y of POCA */
    float m_cmsE = 0; /**< c.m.s. energy of dimuon or bhabha */
    int m_charge = 0; /**< track charge */
    int m_PDG = 0; /**< track MC truth (simulated data only) */

  };

} // end namespace Belle2
