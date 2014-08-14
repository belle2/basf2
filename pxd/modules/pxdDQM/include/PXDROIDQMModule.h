/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDROIDQMModule_H
#define PXDROIDQMModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <string>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDROIDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDROIDQMModule();

      // virtual ~PXDROIDQMModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:

      /** Storearray for ROIs  */
      StoreArray<PXDRawROIs> m_storeROIs;

      /** Histogram 2d hitmap */
      TH1F* hrawROIcount;     /** Histogram  */
      TH1F* hrawROItype;     /** Histogram  */

      TH2F* hrawROIHLTmap;      /** Histogram  */
      TH2F* hrawROIHLTsize;      /** Histogram  */
      TH1F* hrawROIHLT_DHHID;     /** Histogram  */
      TH1F* hrawROIHLTrow1;     /** Histogram  */
      TH1F* hrawROIHLTrow2; /** Histogram  */
      TH1F* hrawROIHLTcol1; /** Histogram  */
      TH1F* hrawROIHLTcol2; /** Histogram  */

      TH2F* hrawROIDCmap;      /** Histogram  */
      TH2F* hrawROIDCsize;      /** Histogram  */
      TH1F* hrawROIDC_DHHID;     /** Histogram  */
      TH1F* hrawROIDCrow1;     /** Histogram  */
      TH1F* hrawROIDCrow2; /** Histogram  */
      TH1F* hrawROIDCcol1; /** Histogram  */
      TH1F* hrawROIDCcol2; /** Histogram  */

      //map<int,int> sensor_to_histindex;

      virtual void defineHisto();

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDROIDQMModule_H
