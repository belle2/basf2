/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#ifndef TELDIGICLUSTERMODULE_H
#define TELDIGICLUSTERMODULE_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <TRandom.h>
#include <string>
#include <set>
#include <vector>
#include <boost/array.hpp>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of VXD testbeams */
  namespace TB {

    /** The Telescope Digitizer/Clusterizer module. */
    class TelDigiClusterModule : public Module {
    public:
      /** Constructor.  */
      TelDigiClusterModule();

      /** Initialize the module and check the parameters */
      virtual void initialize();
      /** Initialize the list of existing PXD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:
      /**
       * Smearing resolution in um in U direction
       **/
      float m_resolutionU;
      /**
       * Smearing resolution in um in V direction
       **/
      float m_resolutionV;
      /** Name of MCParticles collection */
      std::string m_storeMCParticlesName;
      /** Name of the TrueHits collection */
      std::string m_storeTrueHitsName;
      /** name of the Clusters collection */
      std::string m_storeClustersName;
      /** Name of the MCParticle-TrueHit relation */
      std::string m_relMCParticleTrueHitName;
      /** Name of the Clusters-MCParticles relation */
      std::string m_relClusterMCParticleName;
      /** Name of the Clusters-TrueHits relation */
      std::string m_relClusterTrueHitName;

    };//end class declaration
  } // end namespace TB
} // end namespace Belle2

#endif // TBDigiClusterModule_H
