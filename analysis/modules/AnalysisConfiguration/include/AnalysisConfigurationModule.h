/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONFIGUREMOD_H
#define CONFIGUREMOD_H
#include <string>
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Class to hold general basf2 configuration
   */
  class AnalysisConfigurationModule : public Module {

  public:

    /**
     * Constructor
     */
    AnalysisConfigurationModule();

    virtual void initialize() override;

    /** Set configuration. */
    //void setTupleStyle(std::string);

    /** Get configuration. */
    //const std::string& getTupleStyle();

    /**
     * Exposes getConfiguration function of the AnalysisConfiguration class to Python.
     */
    //static void exposePythonAPI();

  protected:
    std::string m_TupleStyle; /**< Style of variables in nTuple */

  };

} // Belle2 namespace

#endif



