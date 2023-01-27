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

#include <framework/database/DBObjPtr.h>

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
  class MVAMultipleExpertsModule : public Module {
  public:

    /**
     * Constructor
     */
    MVAMultipleExpertsModule();

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
    std::vector<std::vector<float>> analyse(Particle*);

    /**
     * Initialize mva expert, dataset and features
     * Called every time the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile, unsigned int i);

    /**
     * Evaluate the variables and fill the Datasets to be used by the experts.
     */
    void fillDatasets(Particle*);

    /**
     * Set the extra info field.
     */
    void setExtraInfoField(Particle*, std::string, float, unsigned int);

    /**
     * Set the event extra info field.
     */
    void setEventExtraInfoField(StoreObjPtr<EventExtraInfo>, std::string, float, unsigned int);

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_identifiers; /**< weight-files */
    std::vector<std::string>
    m_extraInfoNames; /**< Names under which the SignalProbability is stored in the extraInfo of the Particle object. */
    double m_signal_fraction_override; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */

    std::vector<std::vector<const Variable::Manager::Var*>>
                                                         m_individual_feature_variables; /**< Vector of pointers to the feature variables for each expert*/

//     std::vector<const Variable::Manager::Var*> m_feature_variables;

    std::map<const Variable::Manager::Var*, float>
    m_feature_variables; /**< Map containing the values of all needed feature variables */

    std::vector<std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>>
    m_weightfile_representations; /**< Vector of database pointers to the Database representation of the weightfile */

    std::vector<std::unique_ptr<MVA::Expert>> m_experts; /**< Vector of pointers to the current MVA Experts */

    std::vector<std::unique_ptr<MVA::SingleDataset>> m_datasets; /**< Vector of pointers to the current input datasets */

    std::vector<int>
    m_overwriteExistingExtraInfo; /**< vector of -1/0/1/2: overwrite if lower/ don't overwrite / overwrite if higher/ always overwrite, in case the given extraInfo for the corresponding method is already defined. */
    std::vector<bool> m_existGivenExtraInfo; /**< check if the given extraInfo is already defined. */

    std::vector<unsigned int>
    m_nClasses; /**< number of classes (~outputs) of the MVA Experts. If m_nClasses==2 then only 1 output is expected. */
  };

} // Belle2 namespace

