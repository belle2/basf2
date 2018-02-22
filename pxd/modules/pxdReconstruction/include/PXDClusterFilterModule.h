/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDClusterFilterModule_H
#define PXDClusterFilterModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <string>
#include <memory>

namespace Belle2 {
  namespace PXD {

    /** The PXDClusterFilter module.
     *
     * This module filter an existing PXDClusters StoreArray to exclude clusters
     * with cluster size/charge/seed charge outside of a prescribed interval.
     * The module restores clusters' relations to MCParticles, TrueHits and Digits.
     */
    class PXDClusterFilterModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDClusterFilterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the filtering */
      virtual void event() override;
      /** fitering function */
      bool goodCluster(const PXDCluster& cls) const
      {
        return (
                 cls.getCharge() > m_minCharge &&
                 cls.getSize() > m_minSize
               );
      }


    private:
      /** Name of the collection to use for the PXDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the PXDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the PXDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between PXDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between PXDClusters and PXDTrueHits */
      std::string m_relClusterTrueHitName;
      /** Name of the relation between PXDClusters and PXDDigits */
      std::string m_relClusterDigitName;

      /** Minimum claster charge */
      double m_minCharge;
      /** Minimum cluster size */
      unsigned int m_minSize;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDClusterFilterModule_H
