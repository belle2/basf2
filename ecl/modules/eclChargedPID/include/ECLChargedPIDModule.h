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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Unit.h>
#include <framework/database/DBObjPtr.h>

//MDST
#include <mdst/dataobjects/Track.h>

//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dbobjects/ECLChargedPidPDFs.h>


namespace Belle2 {

  /**
   * The module implements charged particle identification using ECL-related observables.
   * For each Track matched with a suitable ECLShower, likelihoods for each particle
   * hypothesis are obtained from pdfs stored in a conditions database payload, and then get stored in an ECLPidLikelihood object.
   *
   * The dimensionality of the likelihood depends on how many variables are stored in the payload.
   * The baseline method could be a simple univariate likelihood based on E/p PDFs, but it could be extended to include more ECL quantitites (e.g. shower shape variables).
   *
   * Please note however that the multi-dimensional likelihood approach is based on the the assumption that PDFs of the observables can be factorised into a simple product:
   *
   \f{equation}{
   \mathcal{L}_{\mathrm{ECL}} = \prod_{i}^{N_{\mathrm{obs}}} \mathcal{L}(\mathrm{obs}_{i}),
   \f}
   *
   * namely, the observables are \b statistically \b independent.
   * This is of course quite a strong assumption, which is rarely fulfilled by default.
   * A first attempt can be made by removing \a linear correlations between the input observables when preparing the payload object.
   * Please refer to the documentation of ECLChargedPidPDFs for further details on input variable transformations.
   *
   */
  class ECLChargedPIDModule : public Module {

  public:

    /**
     * Constructor, for setting module description and parameters.
     */
    ECLChargedPIDModule();

    /**
     * Destructor, use to clean up anything you created in the constructor.
     */
    virtual ~ECLChargedPIDModule();

    /**
     * Check the PDFs payload for consistency everytime they change in the database.
     */
    void checkPdfsDB();

    /**
     * Use this to initialize resources or memory your module needs.
     *
     * Also register any outputs of your module (StoreArrays, RelationArrays,
     * StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize() override;

    /**
     * Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /**
     * Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /**
     * Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /**
     * Clean up anything you created in initialize().
     */
    virtual void terminate() override;

  private:

    /**
     * Array of Track objects.
     */
    StoreArray<Track> m_tracks;

    /**
     * Array of ECLPidLikelihood objects.
     */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods;

    /**
     * Interface to get the DB payload for ECL charged PID PDFs.
     */
    DBObjPtr<ECLChargedPidPDFs> m_pdfs;

    /**
     * Dummy value of Log Likelihood for a particle hypothesis.
     * This leads to a null contribution in the PID likelihood ratio.
     * Used when the pdf value is (0 | subnormal | inf | NaN).
    */
    static constexpr double c_dummyLogL = 0.0;

    /**
     * Apply cluster timing selection.
     */
    bool m_applyClusterTimingSel;

    /**
     * Map to contain ECL shower observables.
     * Updated for each shower candidate processed.
     * The keys are enum identifiers defined in the `ECLChargedPidPDFs` DB representation class.
     *
     * When performing transformations of the input variables based on the info stored in the payload,
     * it is crucial to respect the order of the variable names as they appear in the payload itself.
     * For this purpose, the variable list retrieved from the DB payload object via:
     *
     \code
     m_pdfs->getVars(...)
     \endcode
     *
     *can be used to lookup items in the map in the desired order.
     */
    std::unordered_map<ECLChargedPidPDFs::InputVar, double> m_variables {
      {ECLChargedPidPDFs::InputVar::c_E1E9, -1.0},
      {ECLChargedPidPDFs::InputVar::c_E9E21, -1.0},
      {ECLChargedPidPDFs::InputVar::c_S2, -999.0},
      {ECLChargedPidPDFs::InputVar::c_E, -1.0},
      {ECLChargedPidPDFs::InputVar::c_EoP, -1.0},
      {ECLChargedPidPDFs::InputVar::c_Z40, -999.0},
      {ECLChargedPidPDFs::InputVar::c_Z51, -999.0},
      {ECLChargedPidPDFs::InputVar::c_ZMVA, -999.0},
      {ECLChargedPidPDFs::InputVar::c_PSDMVA, -999.0},
      {ECLChargedPidPDFs::InputVar::c_DeltaL, -999.0},
      {ECLChargedPidPDFs::InputVar::c_LAT, -999.0},
    };

    /**
     * The event information.
     */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /**
     * Get the index corresponding to element i,j in a linearised n*m array.
     * Used to perform matrix algebra for variable transformations.
     *
     * n: nr. of rows
     * m: nr. of cols
     * i: row index
     * j: column index
    */
    inline int linIndex(int i, int j, int m)
    {
      return j + m * i;
    }

    /**
     * Transform input variables according to:
     *
     * -) gaussianisation via integration of cumulative distribution function and erf inversion.
     * -) decorrelation via inverse square-root covariance matrix.
     *
     * Implemented as in TMVA (see TMVA version 4.3.0 for ROOT >= 6.12/00, August 8, 2018 user guide, sec. 4.1.2, 4.1.4).
    */
    void transfoGaussDecorr(const unsigned int pdg, const int charge, const double& p, const double& theta,
                            std::vector<double>& variables);

    /**
     * Extract the PDF value for a given variable from the TF1 object.
     * If the variable is out of bounds, the pdf is set to be equal to the pdf value at the minimum/maximum of the TF1 range.
     */
    double getPdfVal(const double& x, const TF1* pdf);

    /**
     * Defintion of sqrt(2)
     */
    static constexpr double c_sqrt2 = 1.41421356237309504880;

  };

} //Belle2
