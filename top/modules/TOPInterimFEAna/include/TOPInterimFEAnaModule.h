#ifndef TOPINTERIMFEANAMODULE_H
#define TOPINTERIMFEANAMODULE_H

#include <boost/shared_ptr.hpp>

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

// dataobjects
#include <framework/dataobjects/EventMetaData.h>

#include <TTree.h>

const Int_t g_nMaxHit = 2000;

namespace Belle2 {

  class TOPInterimFEAnaModule : public HistoModule {

    // Public functions
  public:
    //! Constructor / Destructor
    TOPInterimFEAnaModule();
    virtual ~TOPInterimFEAnaModule();

    //! Module functions to be called from main process
    virtual void initialize();
    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();
    //! Module funcions to define histograms
    virtual void defineHisto();

  private:
    TTree* m_tree;
    Short_t m_CalCh; /**< asic channel number where the calibration pulses are routed */
    //Float_t m_TimePerWin;

    Int_t m_nHit;
    Short_t m_SlotNum[g_nMaxHit];
    Short_t m_PixelId[g_nMaxHit];
    Bool_t m_IsCalCh[g_nMaxHit]; /**< true if the hit is in the cal. channel */
    UInt_t m_EventNum[g_nMaxHit];
    Short_t m_WinNum[g_nMaxHit]; /**< "m_firstWindow" in TOPDigit */
    Float_t m_time[g_nMaxHit]; /**< "m_time" in TOPDigit, hit time after time base correction (thus in ns unit), but not yet available */
    Float_t m_TdcRaw[g_nMaxHit]; /**< "m_TDC" in TOPDigit, divided by 16 to make it in sample unit */
    Float_t m_RefTdc[g_nMaxHit]; /**< "m_TdcRaw" in cal. channel */
    Float_t m_height[g_nMaxHit]; /**< "m_ADC" in TOPDigit */
    Float_t m_q[g_nMaxHit]; /**< "m_integral" in TOPDigit, but not available */
    Float_t m_width[g_nMaxHit]; /**< "m_pulseWidth" in TOPDigit, full width at half maximum of the pulse */
    Short_t m_nPixelProcessed;

    Bool_t IsCalCh(Short_t PixelId);
  };

}  //namespace Belle2


#endif
