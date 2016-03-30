/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/FANNMLP.h>
#include <analysis/dataobjects/FANNMLPData.h>
#include <analysis/VariableManager/Manager.h>

#include <memory>

namespace Belle2 {

  class Particle;

  /**
   * This module adds an ExtraInfo to the Particle objects in a given ParticleList.
   * The ExtraInfo is calculated by a FANN Multi-Layer-Perceptron (MLP) and represents the SignalProbability
   * of the particle with respect to the training. The desired FANN MLP has to be
   * trained via the FANNTeacherModule before one can use in this FANNExpertModule.
   */

  class FANNExpertModule : public Module {
  public:

    /**
     * Constructor
     */
    FANNExpertModule();

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

    /** Load MLPs from file.
     * @param filename name of the TFile to read from
     * @param arrayname name of the TObjArray holding the MLPs in the file
     * @return true if the MLPs were loaded correctly
     */
    bool load(const std::string& filename, const std::string& arrayname);

    /**
     * Analyse a Particle with the given MLP and calculates signal probability
     * @param particle the particle which should be analysed
     * @return output of the MLP for the particle
     */
    float analyse(const Particle*);

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_methodName; /**< name of the FANN method specified in the training by FANNTeacher */
    std::string m_methodPrefix; /**< common prefix for the methods trained by FANNTeacher */
    std::string m_workingDirectory; /**< Working directory in which the expert finds the FANNweightFile directory */
    std::string m_expertOutputName; /**< Name under which the SignalProbability is stored in the ExtraInfo of the Particle object. */
    std::string m_filename;  /**< Filename where the weights of the trained MLPs are saved.*/
    std::vector<const Variable::Manager::Var*> m_variables; /**< Input variables for the FANN MLP */
    int m_signalClass; /**< Class which is considered signal.  */
    double m_signalFraction; /**< signalFraction to calculate probability, -1 the training signal fraction is used */
    FANNMLP m_MLP; /**< Trained neural Network */

  };

} // Belle2 namespace

