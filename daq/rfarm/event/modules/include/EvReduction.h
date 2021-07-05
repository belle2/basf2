/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef EVREDUCTION_H
#define EVREDUCTION_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawCOPPER.h>

#define RBUFSIZE   100000000
#define MAXEVTSIZE  10000000

/*
#define DETECTOR_MASK 0xFFFFFFFF // tentative
#define SVD_ID 0x00000001 // tentative
#define CDC_ID 0x00000002 // tentative
#define TOP_ID 0x00000003 // tentative
#define ARICH_ID 0x00000004 // tentative
#define ECL_ID 0x00000005 // tentative
#define KLM_ID 0x00000006 // tentative
*/

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class EvReductionModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    EvReductionModule();
    virtual ~EvReductionModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    // Data members
  private:
    //! No. of sent events
    int m_nevt;

    int m_red_factor;

    StoreArray<RawDataBlock> m_rawDataBlock; /**< RawDataBlock. */
    StoreArray<RawCOPPER> m_rawCOPPER; /**< RawCOPPER. */
    StoreArray<RawCDC> m_rawCDC; /**< RawCDC. */
    StoreArray<RawSVD> m_rawSVD; /**< RawSVD. */
    StoreArray<RawTOP> m_rawTOP; /**< RawTOP. */
    StoreArray<RawECL> m_rawECL; /**< RawECL. */
    StoreArray<RawARICH> m_rawARICH; /**< RawARICH. */
    StoreArray<RawKLM> m_rawKLM; /**< RawKLM. */
    StoreArray<RawFTSW> m_rawFTSW; /**< RawFTSW. */
#ifdef DESY
    StoreArray<RawTLU> m_rawTLU; /**< RawTLU. */
#endif
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
