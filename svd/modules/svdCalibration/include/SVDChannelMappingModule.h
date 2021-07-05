/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef svdchannelmappingModule_H
#define svdchannelmappingModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <vxd/geometry/GeoCache.h>

#include <tracking/dataobjects/SVDIntercept.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <unordered_map>
#include <utility>
#include <functional>
#include <TH1.h>
#include <TFile.h>

namespace Belle2 {

  /** The Channel Mapping Check Module
   *
   */
  class SVDChannelMappingModule : public Module {

  public:

    /** Constructor defining the parameters */
    SVDChannelMappingModule();

    /**initialize*/
    void initialize() override;
    /** event*/
    void event() override;
    /** temrinate*/
    void terminate() override;

    std::string m_SVDShaperDigitsName; /**< shaper digit list name*/
    std::string m_SVDClustersName; /**< reco digit list name*/

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TList* m_histoList_digits = nullptr; /**< list of histograms for the strips*/
    TList* m_histoList_clusters = nullptr; /**< list of histograms for the clusters*/

  private:

    StoreArray<SVDIntercept> m_Intercepts; /**< SVDintercept StoreArray*/
    StoreArray<SVDCluster> m_clusters; /**< SVDClusters StoreArray*/
    StoreArray<SVDShaperDigit> m_shapers; /**<SVDSHaperDigits StoreArray*/

    VXD::GeoCache& m_aGeometry = VXD::GeoCache::getInstance(); /**< the geometry */

    std::string m_InterceptsName; /**< Name of the SVDIntercept StoreArray */

    /** typedef: histograms to be filled once per intercept + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const SVDIntercept*) > > InterHistoAndFill;
    /** map of histograms to be filled once per intercept */
    std::unordered_multimap<Belle2::VxdID, InterHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hInterDictionary;

    void createHistosDictionaries(); /**< create the dictionary*/
    void fillSensorInterHistos(const SVDIntercept* inter); /**< fill histograms per sensor, filled once per intercept */


    int n_events; /**< number of events*/


  };//end class declaration


} // end namespace Belle2

#endif // SVDCHANNELMAPPINGtModule_H
