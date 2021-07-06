/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BgoModule_H
#define BgoModule_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the BGO detector */
  namespace bgo {

    /**
     * The Bgo module.
     *
     * This is an almost empty module which just illustrates how to define
     * parameters and use the datastore
     */
    class BgoModule : public Module {

    public:

      /** Constructor */
      BgoModule();

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

  } // bgo namespace
} // end namespace Belle2

#endif // BgoModule_H
