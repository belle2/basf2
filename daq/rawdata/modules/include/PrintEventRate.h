//+
// File : PrintEventRate.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTEVENTRATE_H
#define PRINTEVENTRATE_H
#include <daq/rawdata/modules/PrintDataTemplate.h>

namespace Belle2 {



  /*! A class definition of an input module for Sequential ROOT I/O */

  class PrintEventRateModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintEventRateModule();
    virtual ~PrintEventRateModule();

    //! Module functions to be called from event process
    virtual void event();
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);

  protected:
    unsigned int prev_utime;
    unsigned int prev_event;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
