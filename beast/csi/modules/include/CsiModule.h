/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CsiModule_H
#define CsiModule_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CSI detector */
  namespace csi {

    /**
     * The Csi module.
     *
     * This is an almost empty module which just illustrates how to define
     * parameters and use the datastore
     */
    class CsiModule : public Module {

    public:

      /** Constructor */
      CsiModule();

      /** Init the module */
      virtual void initialize() override;

      /** Called for every begin run */
      virtual void beginRun() override;

      /** Called for every end run */
      virtual void event() override;

      /** Called for every end run */
      virtual void endRun() override;

      /** Called on termination */
      virtual void terminate() override;

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

  } // csi namespace
} // end namespace Belle2

#endif // CsiModule_H
