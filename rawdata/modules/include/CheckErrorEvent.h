//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef CHECKERROREVENTMODULE_H
#define CHECKERROREVENTMODULE_H

#include <rawdata/modules/PrintDataTemplate.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class CheckErrorEventModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor / Destructor
    CheckErrorEventModule();
    virtual ~CheckErrorEventModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //! Module functions to be called from event process
    virtual void terminate();

    //! Check CRC error
    void checkCRCError(RawCOPPER* rawcpr, int i);

#ifndef REDUCED_RAWCOPPER
#else
    //    void printReducedCOPPEREvent(ReducedRawCOPPER* reduced_raw_copper, int i);
#endif

  protected :
    int m_packetCRCerr_cpr = 0;
    int m_eventCRCerr_cpr = 0;
    int m_packetCRCerr_evt = 0;
    int m_eventCRCerr_evt = 0;

    int m_eventCRCerr_evtmetadata = 0;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
