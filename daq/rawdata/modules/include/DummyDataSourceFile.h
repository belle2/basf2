//+
// File : DummyDataPacker.h
// Description : Module to store dummy data in RawCOPPER object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-

#ifndef DUMMYDATASOURCEFILE_H
#define DUMMYDATASOURCEFILE_H

#include <daq/rawdata/modules/DummyDataSource.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DummyDataSourceFileModule : public DummyDataSourceModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DummyDataSourceFileModule();
    virtual ~DummyDataSourceFileModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  protected :


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
