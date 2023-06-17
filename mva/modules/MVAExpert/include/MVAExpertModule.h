/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/dataobjects/EventExtraInfo.h>

#include <vector>
#include <string>

namespace Belle2 {

  class Particle;

  /**
   * This module adds an ExtraInfo to the Particle objects in a given ParticleList.
   * The ExtraInfo is calculated by an MVA method loaded from the database.
   */
  class MVAExpertModule : public Module {
  public:

    /**
     * Constructor
     */
    MVAExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called at the beginning of a new run
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Called at the end of the event processing.
     */
    virtual void terminate() override;

  private:
    /**
     * Calculates expert output for given Particle pointer
     */
    float analyse(const Particle*);

    /**
     * Calculates expert output for given Particle pointer
     */
    std::vector<float> analyseMulticlass(const Particle*);

    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);

    /**
     * Evaluate the variables and fill the Dataset to be used by the expert.
     */
    void fillDataset(const Particle*);

    /**
     * Set the extra info field.
     */
    void setExtraInfoField(Particle*, std::string, float);

    /**
     * Set the event extra info field.
     */
    void setEventExtraInfoField(StoreObjPtr<EventExtraInfo>, std::string, float);


  private:
    /**
     * StoreArray of Particles
     */
    StoreArray<Particle> m_particles;
    /**
     * Decay descriptor of decays to look for.
     */
    std::unordered_map<std::string, DecayDescriptor>  m_decaydescriptors;
    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_targetListNames; /**< input particle list names after decay descriptor*/
    std::string m_identifier; /**< weight-file */
    std::string m_extraInfoName; /**< Name under which the SignalProbability is stored in the extraInfo of the Particle object. */
    double m_signal_fraction_override; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */

    std::vector<const Variable::Manager::Var*> m_feature_variables; /**< Pointers to the feature variables */

    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation; /**< Database pointer to the Database representation of the weightfile */
    std::unique_ptr<MVA::Expert> m_expert; /**< Pointer to the current MVA Expert */
    std::unique_ptr<MVA::SingleDataset> m_dataset; /**< Pointer to the current dataset */

    int m_overwriteExistingExtraInfo; /**< -1/0/1/2: overwrite if lower/ don't overwrite / overwrite if higher/ always overwrite, in case the given extraInfo is already defined. */
    bool m_existGivenExtraInfo; /**< check if the given extraInfo is already defined. */

    unsigned int
    m_nClasses; /**< number of classes (~outputs) of the current MVA Expert. If m_nClasses==2 then only 1 output is expected. */
  };

} // Belle2 namespace

