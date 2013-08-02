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
    SendHeader(unsigned int*);
    //! Destructor
    ~SendHeader();

    //! Get Header contents
    unsigned int* header(void);

    //! set buffer
    void header(unsigned int* hdr);

    void initialize(); //! initialize Header

    void set_nwords(int total_data_nwords);    //! set contents of Header

    void set_num_events_in_packet(int num_events);

    int get_nwords();  //! get contents of Header
    int get_hdr_nwords();  //! get contents of Header
    int get_num_events_in_packet();

  private:

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_NUM_EVENTS_IN_PACKET = 2,
      POS_RESERVED_0 = 3,
    };

    /* Data Format : Node info */
    enum {
      NUM_HDR_WORDS = 4
    };


    unsigned int m_header[ NUM_HDR_WORDS ];

    ClassDef(SendHeader, 1);
  };




}

#endif
