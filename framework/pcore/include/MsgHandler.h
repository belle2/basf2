//+
// File : MsgHandler.cc
// Description : Encode/Decode EvtMessage
//
// Author : SooHyung Lee, Ryosuke Itoh
// Date : 31 - Jul - 2008
// Modified : 4 - Jun - 2010
//-

#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <vector>
#include "TObject.h"
#include "framework/pcore/EvtMessage.h"

#define MAXEVTMSG   4000000*4         // 4Mwords

namespace Belle2 {
  /*! A class to encode/decode an EvtMessage */
  class MsgHandler {
  public:
    /*! Constructor */
    MsgHandler(int complevel);
    /*! Destructor */
    virtual ~MsgHandler(void);

    /*! Clear object list */
    virtual void clear(void);
    /*! Add an object to be streamed */
    virtual void add(TObject*, std::string name);

    /*! Stream object list into an EvtMessage */
    virtual EvtMessage* encode_msg(RECORD_TYPE rectype);
    /*! Decode an EvtMessage into a vector list of objects with names */
    virtual int decode_msg(EvtMessage*, std::vector<TObject*>&, std::vector<std::string>&);

  private:
    std::vector<TMessage*> m_buf;
    std::vector<std::string> m_name;
    int m_complevel;
    char* m_cbuf;


  };

} // namespace Belle2

#endif
