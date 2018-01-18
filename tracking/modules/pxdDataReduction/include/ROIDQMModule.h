/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa, Eugenio Paoloni                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef roiDQMModule_H
#define roiDQMModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>

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
  class ROIDQMModule : public HistoModule {

  public:

    /** Constructor defining the parameters */
    ROIDQMModule();

    void initialize() override;

    void event() override;

    void endRun() override;

    std::string m_PXDDigitsName; /**< digit list name*/

  private:

    VXD::GeoCache& m_aGeometry = VXD::GeoCache::getInstance(); /**< the geometry */

    std::string m_ROIsName; /**< Name of the ROIid StoreArray */
    std::string m_InterceptsName; /**< Name of the PXDIntercept StoreArray */

    TDirectory* m_InterDir; /**< intercepts directory in the root file */
    TDirectory* m_ROIDir; /**< ROI directory in the root file */

    /** typedef: histograms to be filled once per intercept + filling function*/
    typedef std::pair< TH1*, std::function< void(TH1*, const PXDIntercept*) > > InterHistoAndFill;
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
    void fillSensorInterHistos(const PXDIntercept* inter); /**< fill histograms per sensor, filled once per intercept */

    int m_numModules; /**< number of modules*/

    TH1F* hnROIs; /**< number of ROIs*/
    TH1F* hnInter; /**< number of intercpets*/
    TH1F* harea; /**< ROis area */
    TH1F* hredFactor; /**< reduction factor*/
    TH2F* hCellUV; /**< U,V cells */
    int n_events; /**< number of events*/

    TH2F* h_HitRow_CellU; /**< pxdRawHit ROW vs CellID U*/
    TH2F* h_HitCol_CellV; /**< pxdRawHit ROW vs CellID U*/

    virtual void defineHisto(); /**< define histograms*/

  };//end class declaration


} // end namespace Belle2

#endif // ROIDQMModule_H
