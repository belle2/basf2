/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

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
      virtual void initialize() override;
      /** do the clustering */
      virtual void event() override;
    private:
      /** Name of the first PXDCluster StoreArray */
      std::string m_clustersOld;
      /** Name of the second PXDCluster StoreArray */
      std::string m_clustersNew;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
