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

#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <pxd/dataobjects/PXDDigit.h>

#include <unordered_map>
#include <utility>
#include <functional>
#include <TH1.h>
#include <TH1F.h>

namespace Belle2 {

  /** The HLT ROI DQM module.
   *
   * Creates basic DQM for ROI creation on HLT
   */
  class ROIDQMModule : public HistoModule {

  public:

    ROIDQMModule();

  private:
    StoreArray<PXDDigit> m_pxdDigits; /**< the PXDDigits dataobjects collection */
    StoreArray<ROIid> m_roiIDs; /**< the ROIids dataobjects collection */
    StoreArray<PXDIntercept> m_pxdIntercept; /**< the PXDIntercepts dataobjects collection */

    std::string m_PXDDigitsName; /**< digit list name*/

    VXD::GeoCache& m_aGeometry = VXD::GeoCache::getInstance(); /**< the geometry */

    std::string m_ROIsName; /**< Name of the ROIid StoreArray */
    std::string m_InterceptsName; /**< Name of the PXDIntercept StoreArray */

    TDirectory* m_InterDir; /**< intercepts directory in the root file */
    TDirectory* m_ROIDir; /**< ROI directory in the root file */

    /** typedef: histograms to be filled once per intercept + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const PXDIntercept*) > > InterHistoAndFill;
    /** map of histograms to be filled once per intercept */
    std::unordered_multimap<Belle2::VxdID, InterHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > m_hInterDictionary;

    /** typedef: histograms to be filled once per roi + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const ROIid*) > > ROIHistoAndFill;
    /** map of histograms to be filled once per roi */
    std::unordered_multimap<Belle2::VxdID, ROIHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > m_hROIDictionary;

    /** struct: histograms to be filled once per event + filling fucntion + accumulate function*/
    struct ROIHistoAccumulateAndFill {
      TH1* hPtr;  /**< histogram pointer */
      std::function< void (const ROIid*, double&) > accumulate;   /**< accumulate function*/
      std::function< void (TH1*, double&) > fill;   /**< fill function */
      double value; /**< value used to fill*/
    };
    /** map of histograms to be filled once per event */
    std::unordered_multimap<Belle2::VxdID, ROIHistoAccumulateAndFill&, std::function<size_t (const Belle2::VxdID&) > >
    m_hROIDictionaryEvt;

    void createHistosDictionaries(); /**< create the dictionary*/
    void fillSensorROIHistos(const ROIid* roi); /**< fill histograms per sensor, filled once per ROI */
    void fillSensorInterHistos(const PXDIntercept* inter); /**< fill histograms per sensor, filled once per intercept */

    int m_numModules; /**< number of modules*/

    TH1F* m_hnROIs; /**< number of ROIs*/
    TH1F* m_hnInter; /**< number of intercpets*/
    TH1F* m_harea; /**< ROis area */
    TH1F* m_hredFactor; /**< reduction factor*/

    void initialize(void) override final;

    void event(void) override final;

    void terminate(void) override final;

    void defineHisto() override final; /**< define histograms*/

  };//end class declaration


} // end namespace Belle2
