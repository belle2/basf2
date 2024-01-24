/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// MVA
#include <mva/interface/Expert.h>
#include <mva/interface/Dataset.h>

//MDST
#include <mdst/dataobjects/Track.h>

//ECL
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dbobjects/ECLChargedPIDMVAWeights.h>


namespace Belle2 {


  /**
  * This module implements charged particle identification using ECL-related observables via a multiclass MVA.
  * For each track matched with a suitable ECLShower, the relevant ECL variables (shower shape, PSD etc.) are fed to the MVA which is stored in a conditions database payload.
  * The MVA output variables are then used to construct a likelihood from pdfs also stored in the payload.
  * The likelihood is then stored in the ECLPidLikelihood object.
  */

  class ECLChargedPIDMVAModule : public Module {

  public:

    /**
     * Constructor
     */
    ECLChargedPIDMVAModule();

    /**
     * Destructor
     */
    virtual ~ECLChargedPIDMVAModule();

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
     * Check payloads for consistency.
     */
    void checkDBPayloads();

    /**
     * Initialise the multiclass MVAs.
     */
    void initializeMVA();

    /**
     * log transformation.
     * @param value to be log transformed.
     * @param offset offset applied during the log transformation to prevent transforming 0.
     * @param max maximum possible return value of the MVA method.
     */
    float logTransformation(const float value, const float offset, const float max) const;

    /**
     * gaussian transformation.
     * @param value to be gauss transformed.
     * @param cdf histogram containing a cdf of the distribution from which value is taken.
     */
    float gaussTransformation(const float value, const TH1F* cdf) const;

    /**
     * decorrelation transformation.
     * Follows the TMVA algorithm for input variable decorrelation.
     @ param scores list of scores to be transformed.
     @ param decorrelationMatrix 1D flattened list of a square matrix.
     */
    std::vector<float> decorrTransformation(const std::vector<float> scores, const std::vector<float>* decorrelationMatrix) const;


  private:

    /**
     * evaluate the variable for the particle and convert the return value float.
     */
    float evaluateVariable(const Variable::Manager::Var* varobj, const Particle* particle);

    /**
     * The name of the database payload object with the MVA weights.
     */
    std::string m_payload_name;

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /**
     * array of track objects.
     */
    StoreArray<Track> m_tracks;

    /**
     * Array of ECLPidLikelihood objects.
     */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods;

    /**
     * Interface to DB payloads containing MVA weightfiles and pdfs.
     */
    std::unique_ptr<DBObjPtr<ECLChargedPIDMVAWeights>> m_mvaWeights;

    /**
     * unordered map of MVA experts.
     * One per region if the region is covered by the payload.
     */
    std::unordered_map<unsigned int, std::unique_ptr<MVA::Expert>> m_experts;

    /**
     * Dummy value of log Likelihood for a particle hypothesis.
     * Used when the pdf value is (0 | subnormal | inf | NaN).
     * Equivalent to std::log(std::numeric_limits<float>::min());
     * ie. the log of the closest possible value to 0.
     * (log is not constexpr safe for clang, but gcc does not like const)
     * Relevant in two cases:
     * 1. A likelihood is exactly 0, then std::numeric_limits<float>::min()
          is the correct closest value for which the log can be calculated.
     * 2. Unable to calculate likelihoods for the track (outside of acceptance etc.),
     *    then this leads to equally probable ECL PID outcomes for the track regardless
     *    of the tested mass hypothesis (e.g.,
     *    \f$ \Delta logL^{ECL}{i,j} = L^{ECL}{i} - L^{ECL}{j} = 0 \f$ or equivalently,
     *    \f$ \frac{L^{ECL}{i}}{L^{ECL}{i}+L^{ECL}{j}} = 0.5 \f$)
     */
    static constexpr float c_dummyLogL = -87.3365;

    /**
     * Vector of variables used to define the regions in which the MVAs are trained.
     */
    std::vector<const Variable::Manager::Var*> m_binningVariables;

    /**
     * Map of vectors containing the variables objects to be fed to the MVA.
     * Each region can have a unique set of variables.
     */
    std::unordered_map<unsigned int, std::vector<const Variable::Manager::Var*>> m_variables;

    /**
      * MVA dataset to be passed to the expert.
      * One per region if the region is covered by the payload.
      */
    std::unordered_map<unsigned int, std::unique_ptr<MVA::SingleDataset>> m_datasets;

    /**
     * Definition of sqrt(2)
     */
    static constexpr double c_sqrt2 = 1.41421356237309504880;

  }; // class
} // Belle2 namespace