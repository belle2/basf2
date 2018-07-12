//+
// File : CheckErrorEvent.h
// Description : Count the number of CRC error events by checking RawCOPPER's header/trailer and EventMetaData
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef CHECKERROREVENTMODULE_H
#define CHECKERROREVENTMODULE_H

#include <rawdata/modules/PrintDataTemplate.h>


namespace Belle2 {

  /*! Count the number of CRC error events by checking RawCOPPER's header/trailer and EventMetaData */

  class CheckErrorEventModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor / Destructor
    CheckErrorEventModule();
    virtual ~CheckErrorEventModule();

    //! initialization
    virtual void initialize();

    //! event module
    virtual void event();

    //! termination
    virtual void terminate();

    //! Check if a RawCOPPER object contains CRC error flag in RawCOPPER header/trailer
    void checkCRCError(RawCOPPER* rawcpr, int i);

#ifndef REDUCED_RAWCOPPER
#else
    //    void printReducedCOPPEREvent(ReducedRawCOPPER* reduced_raw_copper, int i);
#endif

  protected :

    //! # of packet CRC errors
    int m_packetCRCerr_cpr = 0;

    //! # of event(-fragment) CRC errors
    int m_eventCRCerr_cpr = 0;

    //! # of packet CRC errors in one event
    int m_packetCRCerr_evt = 0;

    //! # of event(-fragment) CRC errors in one event
    int m_eventCRCerr_evt = 0;

    //! # of event(-fragment) CRC errors stored in EventMetaData
    int m_eventCRCerr_evtmetadata = 0;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
