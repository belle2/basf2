/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann (marcel.hohmann@unimelb.edu.au)           *
 *                                                                        *
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
#include <framework/datastore/RelationEntry.h>

#include <mva/interface/Expert.h>
#include <mva/interface/Dataset.h>

//MDST
#include <mdst/dataobjects/Track.h>

//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dbobjects/ECLChargedPIDMVAWeights.h>


namespace Belle2 {


  /**
  * This module implements charged particle identification using ECL-related observables via a multiclass BDT.
  * For each track matched with a suitable ECLShower, the relevant ECL variables (shower shape, PSD etc.) are fed to the BDT which is stored in a conditions database payload.
  * The BDT output variables are then used to construct a liklihood from pdfs also stored in the payload.
  * The liklihood is then stored in the ECLPidLikelihood object.
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

    void initializeMVA();

    float logTransformation(const float value) const;

    float gaussTransformation(const float value, const TH1F* cdf) const;

    std::vector<float> decorrTransformation(const std::vector<float> scores, const std::vector<float>* decorrelationMatrix) const;


  private:

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
     * Array of ECLPidLiklihood objects.
     */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods;

    /**
     * Interface to DB payloads containing MVA weightfiles and pdfs.
     */
    std::unique_ptr<DBObjPtr<ECLChargedPIDMVAWeights>> m_mvaWeights;

    /**
     * Vector of MVA experts. One per phasespace region.
     */
    std::vector<std::unique_ptr<MVA::Expert>> m_experts;

    /**
     * Small offset to use in the log transformation. Stored in the payload.
     */

    double m_log_transform_offset;

    /**
     * Dummy value of Log Likelihood for a particle hypothesis.
     * This leads to a null contribution in the PID likelihood ratio.
     * Used when the pdf value is (0 | subnormal | inf | NaN).
    */
    static constexpr float c_dummyLogL = -std::numeric_limits<float>::max();

    /**
     * vector containing the ECL variables to be fed to the MVA.
     * use the same variables in each region.
     * TODO: can be altered to allow each phasespace region to have a unique variable set.
     */
    std::vector<float> m_variables;

    /**
     * vector containing the spectator variables to be fed to the MVA.
     * use the same variables in each region.
     * TODO: can be altered to allow each phasespace region to have a unique variable set.
     */
    std::vector<float> m_spectators;

    /**
      * MVA dataset to be passed to the expert.
      */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

    /**
     * Defintion of sqrt(2)
     */
    static constexpr double c_sqrt2 = 1.41421356237309504880;

  }; // class
} // Belle2 namespace

