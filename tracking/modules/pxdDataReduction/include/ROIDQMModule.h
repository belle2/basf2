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

#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <unordered_map>
#include <map>
#include <vector>
#include <utility>
#include <functional>
#include <TH1.h>
#include <TFile.h>
#include <iostream>

namespace Belle2 {

  /** \addtogroup modules
   * @{
   */
  /** The HLT ROI DQM module.
   *
   * Creates basic DQM for ROI creation on HLT
   */
  class ROIDQMModule : public HistoModule {

  public:

    /** Constructor defining the parameters */
    ROIDQMModule();

    virtual void initialize();

    virtual void event();

    virtual void endRun();

  private:

    std::string m_ROIsName;
    std::string m_InterceptsName;

    //map of histograms to be filled once per intercept
    typedef std::pair< TH1*, std::function< void(TH1* , const PXDIntercept*) > > InterHistoAndFill;
    std::unordered_multimap<Belle2::VxdID, InterHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hInterDictionary;

    //map of histograms to be filled once per roi
    typedef std::pair< TH1*, std::function< void(TH1* , const ROIid*) > > ROIHistoAndFill;
    std::unordered_multimap<Belle2::VxdID, ROIHistoAndFill, std::function<size_t (const Belle2::VxdID&)> > hROIDictionary;

    //map of histograms to be filled once per event
    struct ROIHistoAccumulateAndFill {
      TH1* hPtr;
      std::function< void (const ROIid*, double&) > accumulate;   //accumulate
      std::function< void (TH1*, double&) > fill;   //fill
      double value;
    };
    std::unordered_multimap<Belle2::VxdID, ROIHistoAccumulateAndFill&, std::function<size_t (const Belle2::VxdID&) > > hROIDictionaryEvt;

    void createHistosDictionaries();
    void fillSensorROIHistos(const ROIid* roi);
    void fillSensorInterHistos(const PXDIntercept* inter);

    int m_numModules;

    TH1F* hnROIs;
    TH1F* hnInter;
    TH1F* harea;
    TH1F* hredFactor;

    virtual void defineHisto();

  };//end class declaration

  /** @}*/

} // end namespace Belle2

#endif // ROIDQMModule_H
