//+
// File : PrintEventRate.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTEVENTRATE_H
#define PRINTEVENTRATE_H
#include <rawdata/modules/PrintDataTemplate.h>

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

    //! print the contents of a RawCOPPER event
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);

    virtual void endRun();

  protected:

    //! Run #
    int m_run;

    //! current UNIX time
    unsigned int m_cur_utime;

    //! current event number
    unsigned int m_cur_event;

    //! UNIX time of the run start
    unsigned int start_utime;

    //! the initial event number
    unsigned int start_event;

    //! UNIX time of the prev. event
    unsigned int prev_utime;

    //! # of the prev. event
    unsigned int prev_event;

    //! Size
    int m_size;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
