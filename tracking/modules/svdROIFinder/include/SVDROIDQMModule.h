/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

#include <vxd/geometry/GeoCache.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/SVDIntercept.h>

#include <unordered_map>
#include <utility>
#include <functional>
#include <TH1.h>
#include <TH1F.h>

namespace Belle2 {

  /** Creates basic DQM for ROI creation on ExpressReco  */

  class SVDROIDQMModule : public HistoModule {

  public:

    /** Constructor defining the parameters */
    SVDROIDQMModule();

    void initialize() override; /**< register histograms*/

    void event() override;/**< fill per-event histograms*/

    void endRun() override; /**< fill per-run histograms*/

  private:

    std::string m_SVDShaperDigitsName; /**< shaper digit list name*/
    std::string m_SVDRecoDigitsName; /**< reco digit list name*/
    std::string m_SVDClustersName; /**< cluster list name*/

    StoreArray<ROIid> m_ROIs; /**< ROis store array*/
    StoreArray<SVDIntercept> m_Intercepts; /**< SVDIntercept Store Arrays*/
    StoreArray<SVDShaperDigit> m_SVDShaperDigits; /**< shaper digit store array */
    StoreArray<SVDRecoDigit> m_SVDRecoDigits; /**< reco digit store array */
    StoreArray<SVDCluster> m_SVDClusters; /**< svd cluster store array*/

    VXD::GeoCache& m_geoCache = VXD::GeoCache::getInstance(); /**< the geo cache instance*/

    std::string m_ROIsName; /**< Name of the ROIid StoreArray */
    std::string m_InterceptsName; /**< Name of the SVDIntercept StoreArray */

    TDirectory* m_InterDir; /**< intercepts directory in the root file */
    TDirectory* m_ROIDir; /**< ROI directory in the root file */

    /** typedef: histograms to be filled once per intercept + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const SVDIntercept*) > > InterHistoAndFill;
    /** map of histograms to be filled once per intercept */
    std::unordered_multimap<Belle2::VxdID, InterHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hInterDictionary;

    /** typedef: histograms to be filled once per roi + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const ROIid*) > > ROIHistoAndFill;
    /** map of histograms to be filled once per roi */
    std::unordered_multimap<Belle2::VxdID, ROIHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hROIDictionary;

    /** struct: histograms to be filled once per event + filling fucntion + accumulate function*/
    struct ROIHistoAccumulateAndFill {
      TH1* hPtr;  /**< histogram pointer */
      std::function< void (const ROIid*, double&) > accumulate;   /**< accumulate function*/
      std::function< void (TH1*, double&) > fill;   /**< fill function */
      double value; /**< value used to fill*/
    };
    /** map of histograms to be filled once per event */
    std::unordered_multimap<Belle2::VxdID, ROIHistoAccumulateAndFill&, std::function<size_t (const Belle2::VxdID&) > >
    hROIDictionaryEvt;

    void createHistosDictionaries(); /**< create the dictionary*/
    void fillSensorROIHistos(const ROIid* roi); /**< fill histograms per sensor, filled once per ROI */
    void fillSensorInterHistos(const SVDIntercept* inter); /**< fill histograms per sensor, filled once per intercept */

    int m_numModules; /**< number of hardware modules*/
    int m_specificLayer =
      -1; /**< specific layer selected for which to produce the plots. If it is not a SVD layer (3, 4, 5, 6) than the plots for all layers are produced. Default is (-1), i.e. plots for all SVD layers are produced.*/
    bool m_plotRecoDigits = false; /**< Produce plots for SVDRecoDigits when True */

    TH1F* hnROIs; /**< number of ROIs*/
    TH1F* hnInter; /**< number of intercpets*/
    TH1F* harea; /**< ROis area */
    TH1F* hredFactor; /**< reduction factor*/
    TH1F* hCellU; /**< U cells */
    TH1F* hCellV; /**< V cells */
    int n_events; /**< number of events*/

    void defineHisto() override; /**< define histograms*/

  };//end class declaration


} // end namespace Belle2

