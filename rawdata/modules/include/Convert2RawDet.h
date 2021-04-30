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

#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>


namespace Belle2 {

  /*! A class definition of a module to convert from RawCOPPER or RawDataBlock to RawDetector objects */

  class Convert2RawDetModule : public Module {

    // Public functions
  public:

    //! Constructor
    Convert2RawDetModule();
    //! Destructor
    virtual ~Convert2RawDetModule();
    //! Called at the beginning of data processing.
    virtual void initialize() override;
    //! Called when entering a new run.
    virtual void beginRun() override;
    //! Called for each event.
    virtual void event() override;
    //! Called if the current run ends.
    virtual void endRun() override;
    //! Called at the end of data processing.
    virtual void terminate() override;

  protected:
    //! function to register data buffer in DataStore as RawDetector
    virtual void convertDataObject(RawDataBlock* raw_dblk, std::vector<unsigned int>& cpr_id);

    //! No. of sent events
    int m_nevt;

    //! Data members
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
