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
#include <analysis/TMVAInterface/Teacher.h>
#include <analysis/modules/TMVATeacher/TMVATeacherModule.h>

#include <memory>

namespace Belle2 {

  class TMVATeacher;

  /**
   * This module provides the same functionality as the TMVATeacherModule,
   * in addition it can train the TMVA methods on-the-fly.
   *
   * Either a normal training with the given target and weight is performed,
   * or if the target is empty and a modelFileName as well as discriminating variables
   * are given, an sPlot training is performed.
   *
   */

  class TMVAOnTheFlyTeacherModule : public TMVATeacherModule {
  public:

    /**
     * Constructor
     */
    TMVAOnTheFlyTeacherModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Terminates the module.
     */
    virtual void terminate();

  protected:

    std::string
    m_target; /**< target used by multivariate analysis method has to be integer valued variable which defines clusters in the sample. */
    std::string m_weight; /**< weight used by the method, has to be a variable defined in the variable manager. */

    std::string m_factoryOption; /**< Options which are passed to the TMVA Factory */
    std::string m_prepareOption; /**< Options which are passed to the TMVA Factory::PrepareTrainingAndTestTree */

    std::vector<std::string>
    m_discriminatingVariables; /**< The variables that will be used as discriminating variables in the sPlot algorithm to determine the weights. */
    std::string
    m_modelFileName; /**< Path to the Root file containing the model which describes the distribution of signal and background events in the discriminating variable. This file will only be opened readonly. */

  };

} // Belle2 namespace


