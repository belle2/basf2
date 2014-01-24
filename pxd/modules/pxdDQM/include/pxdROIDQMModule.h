/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdROIDQMModule_H
#define pxdROIDQMModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <string>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** \addtogroup modules
    * @{
    */
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class pxdROIDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      pxdROIDQMModule();

      // virtual ~pxdROIDQMModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:

      /** Storearray for ROIs  */
      StoreArray<PXDRawROIs> m_storeROIs;

      /** Histogram 2d hitmap */
      TH1F* hrawROIcount;     /** Histogram  */
      TH2F* hrawROImap;      /** Histogram  */
      TH1F* hrawROIDHHID;     /** Histogram  */
      TH1F* hrawROItype;     /** Histogram  */
      TH1F* hrawROIrow1;     /** Histogram  */
      TH1F* hrawROIrow2; /** Histogram  */
      TH1F* hrawROIcol1; /** Histogram  */
      TH1F* hrawROIcol2; /** Histogram  */

      //map<int,int> sensor_to_histindex;

      virtual void defineHisto();

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // pxdROIDQMModule_H
