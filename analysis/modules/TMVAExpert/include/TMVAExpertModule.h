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
     * Destructor
     */
    virtual ~TMVAExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_methodName; /**< name of the TMVA method specified in the training by TMVATeacher */
    std::string m_methodPrefix; /**< common prefix for the methods trained by TMVATeacher */
    std::string m_workingDirectory; /**< Working directory in which the expert finds the config file and the weight file directory */
    std::string m_signalProbabilityName /**< Name under which the SignalProbability is stored in the ExtraInfo of the Particle object. */;
    int m_signalCluster; /**< Number of the cluster which is considered signal.  */
    TMVAInterface::Expert* m_method; /**< Method used to calculate the target variable */
    float m_signalToBackgroundRatio; /**< Signal to background ration to calculate probability, -1 if no transformation of the method output should be performed */

  };

} // Belle2 namespace

