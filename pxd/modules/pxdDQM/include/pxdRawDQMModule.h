/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: kl                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdRawDQMModule_H
#define pxdRawDQMModule_H

#include <framework/core/HistoModule.h>
#include <boost/format.hpp>
#include <string>
#include <deque>
#include <framework/datastore/DataStore.h>
#include <TH2.h>
#include <TCanvas.h>
#include <pxd/dataobjects/PXDRawHit.h>

namespace Belle2 {

  namespace PXD {
    /** \addtogroup modules
    * @{
    */
    class pxdRawDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      pxdRawDQMModule();

      // virtual ~pxdRawDQMModule();

      virtual void initialize();

      virtual void event();

    private:

      //void Draw();

      TH1F* hrawPxdPackets;
      TH1F* hrawPxdPacketSize;
      TH1F* hrawPxdHitsCount;
      TH2F* hrawPxdHits;
      TH1F* hrawPxdHitsCharge;

      std::string m_storeRawHitsName;

      virtual void defineHisto();

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // pxdRawDQMModule_H
