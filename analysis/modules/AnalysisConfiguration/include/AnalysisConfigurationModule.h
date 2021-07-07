/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
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
   * mypath.add_module(conf)
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
                              Possible values are listed in m_style.
                              If user gives wrong style name, it is switched to "default"
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




