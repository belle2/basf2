/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUNPACKERDQMModule_H
#define PXDUNPACKERDQMModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/modules/pxdUnpacking/PXDUnpackerModule.h>
#include <string>
#include <cstring>
#include <TH2.h>
#include <TH1.h>
#include <TAxis.h>
#include <iostream>
#include <vector>

namespace Belle2 {

  namespace PXD {
    /** The PXD Unpacker DQM module.
     *
     * Creates basic DQM for PXD Unpacker errors
     */
    class PXDUnpackerDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDUnpackerDQMModule();

      // virtual ~PXDUnpackerDQMModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      TH1F* hUnpackErrorCount;          /** 1D Histogram  */
      TH2F* hUnpackErrorCountWeight;    /** 1D Histogram  */

      PXDUnpackerModule errors;

      int m_sizeOfErrorBlock;
      bool m_showStats = false;
      virtual void defineHisto();

      std::vector<std::string> err;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerDQMModule_H
