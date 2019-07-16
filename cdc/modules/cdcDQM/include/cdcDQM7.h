/* Nanae Taniguchi 2017.07.12 */
/* Nanae Taniguchi 2018.02.06 */
/* Nanae Taniguchi 2019.02.17 */

#ifndef CDCDQM7MODULE_H
#define CDCDQM7MODULE_H

#include <framework/core/HistoModule.h>
// add
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
//

#include <vector>
#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /**
   * The module for Data Quality Monitor.
   */
  class cdcDQM7Module : public HistoModule {

  public:

    //! Constructor
    cdcDQM7Module();

    //! Destructor
    virtual ~cdcDQM7Module();

    //! Module functions
    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    //! function to define histograms
    void defineHisto() override;

  private:
    TH1D* h_nhits_L[56] = {nullptr}; /**<histogram hit in each layer*/

    TH1D* h_tdc_sL[9] = {nullptr};/**<tdc each super layer*/
    TH1D* h_adc_sL[9] = {nullptr};/**<adc each super layer*/

    TH1D* h_fast_tdc = nullptr;/**<fastest TDC in each event*/
    TH1D* h_board_out_tdc = nullptr;/**<board w/ out of range TDC*/

    TH2D* bmap_2 = nullptr;/**<board status map 2D*/
    // add
    TH1D* h_occ = nullptr;/**<occupancy*/
    TH1D* h_occ_L[56] = {nullptr}; /**<occupancy-layer-dep.*/


  };
}
#endif
