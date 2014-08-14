/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDClusterCheckModule_H
#define PXDClusterCheckModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/reconstruction/Pixel.h>
#include <string>

namespace Belle2 {
  class RelationArray;
  class RelationElement;

  namespace PXD {

    /** The PXDClusterCheck module.
     *
     * This module compares two sets of clusters to be identical. Useful to
     * check if changes to the clustering code introduce changes to the result.
     *
     * @see PXDClusterizerModule
     */
    class PXDClusterCheckModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDClusterCheckModule();
      /** Initialize the module */
      virtual void initialize();
      /** do the clustering */
      virtual void event();
    private:
      /** Name of the first PXDCluster StoreArray */
      std::string m_clustersOld;
      /** Name of the second PXDCluster StoreArray */
      std::string m_clustersNew;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDClusterCheckModule_H
