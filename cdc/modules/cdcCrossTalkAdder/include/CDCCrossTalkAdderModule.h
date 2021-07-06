/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

//cdc package headers
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCCrossTalkLibrary.h>
#include <cdc/dbobjects/CDCFEElectronics.h>

//C++/C standard lib elements.
#include <string>

namespace Belle2 {

  /** The Class for overlaying signal-induced asic cross-talk.
   *
   *  @todo CDCCrossTalkAdderModule: Update to be able to handle 2nd TDC hits as well(?). Speed up.
   */
  class CDCCrossTalkAdderModule : public Module {

  public:
    /** Constructor.*/
    CDCCrossTalkAdderModule();

    /** Initialize variables */
    void initialize() override;

    /** Event func. */
    void event() override;

    /** Set FEE parameters (from DB) */
    void setFEElectronics();

    /** Terminate func. */
    void terminate() override
    {
      if (m_xTalkFromDB) delete m_xTalkFromDB;
      if (m_fEElectronicsFromDB) delete m_fEElectronicsFromDB;
    };

  private:
    StoreArray<CDCHit>     m_hits;        /**< CDCHit array */

    std::string m_inputCDCHitsName;       /**< Input array name.  */

    CDC::CDCGeometryPar* m_cdcgp = nullptr;  /**< Cached Pointer to CDCGeometryPar */

    DBObjPtr<CDCCrossTalkLibrary>* m_xTalkFromDB = nullptr; /*!< Pointer to cross-talk from DB. */

    DBArray<CDCFEElectronics>* m_fEElectronicsFromDB = nullptr; /*!< Pointer to FE electronics params. from DB. */
    float m_lowEdgeOfTimeWindow[nBoards] = {0}; /*!< Lower edge of time-window */
    float m_uprEdgeOfTimeWindow[nBoards] = {0}; /*!< Upper edge of time-window */
    unsigned short m_widthOfTimeWindow [nBoards] = {0}; /*!< Width of time window */

    double m_invOfTDCBinWidth = 0;    /**< Inv. of TDC-bin width (in ns^-1)*/

    bool m_issue2ndHitWarning; /**< Flag to switch on/off a warning on the 2nd TDC hit */
    bool m_includeEarlyXTalks; /**< Flag to switch on/off xtalks earlier than the hit */

    int m_debugLevel; /**< Debug level */

    /** Structure for saving the x-talk information. */
    struct XTalkInfo {
      /** Constructor that initializes all members. */
      XTalkInfo(unsigned short tdc, unsigned short adc, unsigned short tot, unsigned short status) :
        m_tdc(tdc), m_adc(adc), m_tot(tot), m_status(status) {}
      unsigned short m_tdc; /**< TDC count */
      unsigned short m_adc; /**< ADC count */
      unsigned short m_tot; /**< TOT       */
      unsigned short m_status; /**< status */
    };
  };

} // end of Belle2 namespace
