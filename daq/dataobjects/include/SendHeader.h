//+
// File : SendHeader.h
// Description : Module to handle SendHeader attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SENDHEADER_H
#define SENDHEADER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>


namespace Belle2 {

  class SendHeader : public TObject {
  public:

    //! Default constructor
    SendHeader();

    //! Constructor using existing pointer to raw data buffer
    SendHeader(int*);
    //! Destructor
    ~SendHeader();

    //! Get Header contents
    int* GetBuffer(void);

    //! set buffer
    void SetBuffer(int* hdr);

    void Initialize(); //! initialize Header

    void SetNwords(int total_data_nwords);    //! set contents of Header

    void SetNumEventsinPacket(int num_events);

    int GetTotalNwords();  //! get contents of Header
    int GetHdrNwords();  //! get contents of Header
    int GetNumEventsinPacket();

    /* Data Format : Node info */
    enum {
      SENDHDR_NWORDS = 4
    };

  private:

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUM_EVENTS_IN_PACKET = 2,
      POS_RESERVED_0 = 3,
    };



    int m_buffer[ SENDHDR_NWORDS ];

    ClassDef(SendHeader, 1);
  };




}

#endif
