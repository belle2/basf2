/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa, Eugenio Paoloni                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef svdchannelmappingModule_H
#define svdchannelmappingModule_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/dataobjects/SVDIntercept.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <unordered_map>
#include <map>
#include <vector>
#include <utility>
#include <functional>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <iostream>

namespace Belle2 {

  /** The HLT ROI DQM module.
   *
   * Creates basic DQM for ROI creation on HLT
   */
  class SVDChannelMappingModule : public Module {

  public:

    /** Constructor defining the parameters */
    SVDChannelMappingModule();

    void initialize();
    void event();
    void terminate();

    std::string m_SVDShaperDigitsName; /**< shaper digit list name*/
    std::string m_SVDClustersName; /**< reco digit list name*/

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */
    TList* m_histoList_digits;
    TList* m_histoList_clusters;

  private:

    StoreArray<SVDIntercept> m_Intercepts;
    StoreArray<SVDCluster> m_clusters;
    StoreArray<SVDShaperDigit> m_shapers;

    VXD::GeoCache& m_aGeometry = VXD::GeoCache::getInstance(); /**< the geometry */

    std::string m_InterceptsName; /**< Name of the SVDIntercept StoreArray */

    /** typedef: histograms to be filled once per intercept + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const SVDIntercept*) > > InterHistoAndFill;
    /** map of histograms to be filled once per intercept */
    std::unordered_multimap<Belle2::VxdID, InterHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hInterDictionary;

    void createHistosDictionaries(); /**< create the dictionary*/
    void fillSensorInterHistos(const SVDIntercept* inter); /**< fill histograms per sensor, filled once per intercept */

    int m_numModules; /**< number of modules*/

    int n_events; /**< number of events*/


  };//end class declaration


} // end namespace Belle2

#endif // SVDCHANNELMAPPINGtModule_H
