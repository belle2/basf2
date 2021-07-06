/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VXDMisalignmentModule_H
#define VXDMisalignmentModule_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {
  /** Namespace to encapsulate common PXD and SVD code */
  namespace VXD {

    /** The VXD misalignment module.
     * This module is responsible for initialization of misalignment data.
     */
    class VXDMisalignmentModule : public Module {
    public:
      /** Constructor.  */
      VXDMisalignmentModule();

      /** Initialize the module and check the parameters */
      virtual void initialize() override {}
      /** Initialize the list of existing VXD Sensors */
      virtual void beginRun() override;
      /** Digitize one event */
      virtual void event() override {}
      /** Terminate the module */
      virtual void terminate() override {}

    protected:
      /** Name of the xml filename with misalignment data */
      std::string m_xmlFilename;

    };//end class declaration


  } // end namespace VXD
} // end namespace Belle2

#endif // VXDMisalignmentModule_H
