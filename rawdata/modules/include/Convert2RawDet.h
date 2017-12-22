//+
// File : Convert2RawDet.h
// Description : Module to convert from RawCOPPER or RawDataBlock to RawDetector objects
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 24 - Oct - 2014
//-

#ifndef CONVERT2RAWDET_H
#define CONVERT2RAWDET_H

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

//#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>


#include <framework/dataobjects/EventMetaData.h>


namespace Belle2 {

  /*! A class definition of a module to convert from RawCOPPER or RawDataBlock to RawDetector objects */

  class Convert2RawDetModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Convert2RawDetModule();
    virtual ~Convert2RawDetModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  protected:
    //! function to register data buffer in DataStore as RawDetector
    virtual void convertDataObject(RawDataBlock* raw_dblk);

    //! No. of sent events
    int m_nevt;

    // Data members
  private:
    StoreArray<RawDataBlock>  m_rawDataBlock ; /** Array for RawDataBlock*/
    StoreArray<RawCOPPER>     m_rawCOPPER    ; /** Array for RawCOPPER   */
    StoreArray<RawSVD>        m_rawSVD       ; /** Array for RawSVD      */
    StoreArray<RawCDC>        m_rawCDC       ; /** Array for RawCDC      */
    StoreArray<RawTOP>        m_rawTOP       ; /** Array for RawTOP      */
    StoreArray<RawARICH>      m_rawARICH     ; /** Array for RawARICH    */
    StoreArray<RawECL>        m_rawECL       ; /** Array for RawECL      */
    StoreArray<RawKLM>        m_rawKLM       ; /** Array for RawKLM      */
    StoreArray<RawTRG>        m_rawTRG       ; /** Array for RawTRG      */
    StoreArray<RawFTSW>       m_rawFTSW      ; /** Array for RawFTSW     */


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
