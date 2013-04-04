/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#ifndef VXDHITMODULE_H
#define VXDHITMODULE_H

#include <framework/core/Module.h>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of the VXD */
  namespace VXD {

    /** The Telescope Digitizer/Clusterizer module. */
    class VXDHitModule : public Module {
    public:
      /** Constructor.  */
      VXDHitModule();

      /** Initialize the module and check the parameters */
      virtual void initialize();
      /** Initialize the list of existing PXD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:

      /** Name of the DataStore collection for the MCParticles */
      std::string m_MCParticles;


    };//end class declaration
  } // end namespace VXD
} // end namespace Belle2

#endif // TBDigiClusterModule_H
