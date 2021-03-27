/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the AWESOME detector */
  namespace awesome {

    /**
     * The Awesome module.
     *
     * This is an almost empty module which just illustrates how to define
     * parameters and use the datastore
     */
    class AwesomeModule : public Module {

    public:

      /** Constructor */
      AwesomeModule();

      /** Init the module */
      virtual void initialize();

      /** Called for every begin run */
      virtual void beginRun();

      /** Called for every end run */
      virtual void event();

      /** Called for every end run */
      virtual void endRun();

      /** Called on termination */
      virtual void terminate();

    protected:
      /** Useless variable showing how to create integer parameters */
      int m_intParameter;

      /** Useless variable showing how to create double parameters */
      double m_doubleParameter;

      /** Useless variable showing how to create string parameters */
      std::string m_stringParameter;

      /** Useless variable showing how to create array parameters */
      std::vector<double> m_doubleListParameter;
    };

  }
}
