/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Lipp                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/TMVAInterface/Teacher.h>

#include <framework/datastore/StoreObjPtr.h>
#include <analysis/modules/TMVATeacher/RooDataSetMergeable.h>

#include <RooWorkspace.h>

#include <TFile.h>
#include <memory>

namespace Belle2 {

  class TMVATeacher;

  /**
   * This module trains a TMVA method with the given particle lists as training samples.  The target variable has to be a
   * discrete integer valued variable (although the Variable::Manager returns the value as a float) which defines the clusters
   * in the sample. All the clusters are trained against each other.
   * You can apply a trained TMVAMethod with the TMVAExpertModule and calculate the SignalProbability for the Particles in a ParticleList.
   */

  class TMVASPlotTeacherModule : public Module {
  public:


    /**
     * Constructor
     */
    TMVASPlotTeacherModule();

    /**
     * Destructor
     */
    virtual ~TMVASPlotTeacherModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Terminates the module.
     */
    virtual void terminate();

  private:

    std::shared_ptr<RooWorkspace>
    getWorkspace(); /**< Helper function creating a RooWorkspace and importing the model from the Root file specified by the user. */

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::vector<std::string> m_variables; /**< input variables for the TMVA method */
    std::vector<std::string> m_spectators; /**< input spectators for the TMVA method */
    std::vector<std::string>
    m_discriminatingVariables; /**< The variables that will be used as discriminating variables in the sPlot algorithm to determine the weights. */
    std::string
    m_modelFileName; /**< Path to the Root file containing the model which describes the distribution of signal and background events in the discriminating variable. This file will only be opened readonly. */
    std::string m_modelObjectName; /**< Name of the RooAbsPdf object which represents the model. */
    std::vector<std::string>
    m_modelPlotComponentNames; /**< Name of RooAbsPdf objects that are part of the model and should be plotted additionally in the control plot. */
    std::vector<std::string>
    m_modelYieldsObjectNames; /**< Name of the RooRealVar objects that represent the yields of the event classes in the model. */
    std::vector<double> m_modelYieldsInitialFractions; /**< Fractions used to set the initial values of the yields. */
    bool m_setYieldRanges; /**< Set the upper limit of the yield variables to the number of candidates. */
    std::vector<std::tuple<std::string, std::string, std::string>> m_methods; /**< tuple(name, type, config) for every method */
    std::string m_methodPrefix; /**< common prefix for the methods trained by TMVATeacher */
    std::string m_workingDirectory; /**< Working directory in which the config file and the weight file directory is created */
    std::string m_factoryOption; /**< Options which are passed to the TMVA Factory */
    std::string m_prepareOption; /**< Options which are passed to the TMVA Factory::PrepareTrainingAndTestTree */
    bool m_createMVAPDFs; /**< Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability */

    bool m_doNotTrain; /**< Do not train the method, just create the sample file. Useful for external training with externTeacher */
    unsigned int m_maxEventsPerClass; /**< Maximum nuber of events per class passed to TMVA */
    unsigned int m_numberOfClasses; /**< Number of classes */

    std::shared_ptr<TMVAInterface::Teacher> m_teacher; /**< Used TMVA method */
    std::map<std::string, const Variable::Manager::Var*> m_discriminating_vars; /**< Variable Pointer to target variable */
    StoreObjPtr<RooDataSetMergeable> m_discriminating_values; /**< Holds the values of the discriminating variables */

    bool isVariableInModel(const std::string variable,
                           std::shared_ptr<RooWorkspace>
                           wspace); /**< Helper function checking if a given variable appears in the model in a given RooWorkspace. */

  };

} // Belle2 namespace


