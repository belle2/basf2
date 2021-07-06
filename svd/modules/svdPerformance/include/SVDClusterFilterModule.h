/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CLUSTERFILTERMODULE_H
#define CLUSTERFILTERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <svd/dataobjects/SVDCluster.h>


namespace Belle2 {
  /**
   * generates a new StoreArray from the input StoreArray which has all specified Clusters removed
   *
   */
  class SVDClusterFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDClusterFilterModule();

    /** if required */
    virtual ~SVDClusterFilterModule();

    /** initializes the module */
    virtual void beginRun() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

  private:

    std::string m_inputArrayName;  /**< StoreArray with the input clusters */
    std::string m_outputINArrayName;  /**< StoreArray with the selectd output clusters */
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output clusters */
    int m_layerNum;  /**< the layer number from which the clusters should be excluded  m_sensorID*/
    int m_xShell;  /**< X shell identificator: +1(+X), -1 (-X), 0(both)*/
    int m_yShell;  /**< Y shell identificator: +1(+Y), -1 (-Y), 0(both)*/
    float m_minClSNR; /**< minimum cluster SNR */

    std::set<VxdID> m_outVxdID; /**<set  containing the VxdID of the DUT sensors */
    void create_outVxdID_set(); /**<creates the set containing the VxdID of the DUT sensors */

    SelectSubset<SVDCluster> m_selectedClusters; /**< all clusters NOT on the layer with m_layerNum */
    SelectSubset<SVDCluster> m_notSelectedClusters; /**< all clusters on the layer with m_layerNum */
  };
}

#endif /* CLUSTERFILTERMODULE_H */
