/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

#include <TMVA/Reader.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {

  class Particle;

  /**
   * This module adds an ExtraInfo to the Particle objects in a given ParticleList.
   * The ExtraInfo is calculated by: a TMVA method, FastBDT, Tensorflow, ...
   * given the corresponding weight-files
   * and represents the SignalProbability of the particle with respect to the training.
   */

  class ExpertModule : public Module {
  public:

    enum Type {
      UnkownType = 0,
      TMVAExpert = 1,
      TensorflowExpert = 2,
      FastBDTExpert = 3,
      NeuroBayesExpert = 4,
    };

    /**
     * Constructor
     */
    ExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called at the beginning of a new run
     */
    virtual void beginRun() override;

    /**
     * Called at the end of a run
     */
    virtual void endRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Terminates the module.
     */
    virtual void terminate() override;

  private:
    /**
     * TMVA specific initialization
     */
    void initializeTMVA();

    /**
     * Tensorflow specific initialization
     */
    void initializeTensorflow();

    /**
     * FastBDT specific initialization
     */
    void initializeFastBDT();

    /**
     * NeuroBayes specific initialization
     */
    void initializeNeuroBayes();

    /**
     * TMVA specific initialization
     */
    void beginRunTMVA(const boost::property_tree::ptree&);

    /**
     * Tensorflow specific initialization
     */
    void beginRunTensorflow(const boost::property_tree::ptree&);

    /**
     * FastBDT specific initialization
     */
    void beginRunFastBDT(const boost::property_tree::ptree&);

    /**
     * NeuroBayes specific initialization
     */
    void beginRunNeuroBayes(const boost::property_tree::ptree&);

    /**
     * Calculates expert output for given Particle pointer
     */
    float analyse(Particle*);

    /**
     * Calculates TMVA output
     */
    float analyseTMVA();

    /**
     * Calculates Tensorflow output
     */
    float analyseTensorflow();

    /**
     * Calculates FastBDT output
     */
    float analyseFastBDT();

    /**
     * Calculates NeuroBayes output
     */
    float analyseNeuroBayes();

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_weightfile; /**< weight-file */
    std::string m_extraInfoName; /** Name under which the SignalProbability is stored in the extraInfo of the Particle object. */
    bool m_transform2probability; /**< Transform result to probability before returning it */
    double m_signal_fraction_override; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */
    double m_signal_fraction; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */

    std::vector<std::string> m_feature_names; /**< Store place for the feature names*/
    std::vector<std::string> m_spectator_names; /**< Store place for the spectator names */
    std::vector<float> m_feature_input; /**< Store place for the input variables */
    std::vector<float> m_spectator_input; /**< Store place for the input variables */
    std::vector<const Variable::Manager::Var*> m_feature_variables; /**< Pointers to the feature variables */
    std::vector<const Variable::Manager::Var*> m_spectator_variables; /**< Pointers to the spectator variables */

    Type m_expert_type; /**< Type of expert */

    std::unique_ptr<TMVA::Reader> m_tmva_reader; /**<TMVA::Reader, which steers the booked TMVA method */
    std::string m_tmva_method; /**< Method name of TMVA method which should be used */
    std::string m_tmva_type; /**< Type of TMVA method which should be used */
    std::string m_tmva_weightfile; /**< Method name of TMVA method which should be used */
    std::string m_tmva_working_directory; /**< Working directory used by tmva */

  };

} // Belle2 namespace

