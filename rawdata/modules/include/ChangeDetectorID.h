//+
// File : ChangeDetectorID.h
// Description : Module to change DetectorID in RawCOPPER's header
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 24 - Oct - 2014
//-

#ifndef CHANGEDETECTORID_H
#define CHANGEDETECTORID_H

#include <rawdata/modules/Convert2RawDet.h>

namespace Belle2 {

  /*! A class definition of a module to convert from RawCOPPER or RawDataBlock to RawDetector objects */
  class ChangeDetectorIDModule : public Convert2RawDetModule {

    // Public functions
  public:

    //! Constructor / Destructor
    ChangeDetectorIDModule();
    virtual ~ChangeDetectorIDModule();
    void event();

  protected:

    //! function to register data buffer in DataStore as RawDetector
    void convertDataObject(RawDataBlock* raw_dblk);

    // Data members
  private:
    unsigned int m_new_detector_id;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
