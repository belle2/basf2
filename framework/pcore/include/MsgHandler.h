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

  class MsgHandler {
  public:
    MsgHandler(int complevel);
    virtual ~MsgHandler(void);

    virtual void clear(void);
    virtual void add(TObject*, std::string name);

    virtual EvtMessage* encode_msg(RECORD_TYPE rectype);
    virtual int decode_msg(EvtMessage*, std::vector<TObject*>&, std::vector<std::string>&);

  private:
    std::vector<TMessage*> m_buf;
    std::vector<std::string> m_name;
    int m_complevel;
    char* m_cbuf;


  };

} // namespace Belle2

#endif
