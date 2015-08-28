/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/TMVAInterface/Expert.h>
#include <analysis/VariableManager/Manager.h>

#include <memory>

namespace Belle2 {

  class Particle;

  /**
   * This module adds an ExtraInfo to the Particle objects in a given ParticleList.
   * The ExtraInfo is calculated by a TMVA method and represents the SignalProbability
   * of the particle with respect to the training. The desired TMVA method has to be
   * trained via the TMVATeacherModule before one can use in this TMVAExpertModule.
   */

  class TMVAExpertModule : public Module {
  public:

    /**
     * Constructor
     */
    TMVAExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

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
     * Calculates the sPlotPrior
     */
    double sPlotPrior(const Particle* p) const;


    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_methodName; /**< name of the TMVA method specified in the training by TMVATeacher */
    std::string m_methodPrefix; /**< common prefix for the methods trained by TMVATeacher */
    std::string m_workingDirectory; /**< Working directory in which the expert finds the config file and the weight file directory */
    std::string m_expertOutputName; /**< Name under which the SignalProbability is stored in the ExtraInfo of the Particle object. */
    std::string m_splotPrior; /**< sPlotPrior variable name */
    const Variable::Manager::Var* m_splotPrior_func; /**< sPlotPrior function */
    std::vector<double> m_pdf_binning; /**< sPlot pdf binning */
    std::vector<double> m_signal_pdf_bins; /**< sPlotPrior signal pdf bin content */
    std::vector<double> m_background_pdf_bins; /**< sPlotPrior signal pdf bin content */
    int m_signalClass; /**< Class which is considered signal.  */
    double m_signalFraction; /**< signalFraction to calculate probability, -1 the training signal fraction is used */
    bool m_transformToProbability; /**< Transform classifier output to a porbability using given signal fraction */
    std::unique_ptr<TMVAInterface::Expert> m_method; /**< Method used to calculate the target variable */

  };

} // Belle2 namespace

