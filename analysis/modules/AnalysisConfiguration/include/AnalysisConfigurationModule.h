/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ANALYSISCONFIGURATIONMODULE_H
#define ANALYSISCONFIGURATIONMODULE_H
#include <string>
#include <set>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Class to hold general basf2 configuration
   * Used to intiate and configure CreateAnalysis object
   *
   * Steering file syntax:
   *
   * conf = register_module('AnalysisConfiguration')
   * conf.param('tupleStyle', "Laconic")
   * analysis_main.add_module(conf)
   *
   */
  class AnalysisConfigurationModule : public Module {

  public:

    /** Constructor */
    AnalysisConfigurationModule();

    /** Initialize the module */
    virtual void initialize() override;


  protected:
    std::string m_tupleStyle; /**< Style of naming of variables in nTuple configured by user.
                              Possible values are listd in m_style.
                              If user gives wrong style name, it is switchd to "default"
                              */
    std::set<std::string> m_styles = {"default", "laconic", "semilaconic" };/**< List of possible styles of variables in nTuple
                                                                            Possible styles on example of PX variable of pi0 from D
                                                                            in decay B->(D->pi0 pi) pi0:
                                                                            'default': B_D_pi0_PX
                                                                            'semilaconic': D_pi0_PX
                                                                            'laconic': pi01_PX"
                                                                            */

    std::string m_mcMatchingVersion; /**< specifies the version of MC matching algorithm to be used */
  };

} // Belle2 namespace

#endif



