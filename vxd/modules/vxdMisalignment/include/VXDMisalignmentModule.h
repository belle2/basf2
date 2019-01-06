/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
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
