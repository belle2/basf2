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

#include <framework/pcore/EvtMessage.h>

#include <vector>
#include <string>

class TObject;
class TMessage;

namespace Belle2 {
  /** A class to encode/decode an EvtMessage */
  class MsgHandler {
  public:
    /** Constructor */
    explicit MsgHandler(int complevel);
    /** Destructor */
    virtual ~MsgHandler();

    /** Clear object list */
    virtual void clear();
    /** Add an object to be streamed */
    virtual bool add(const TObject*, const std::string& name);

    /** Stream object list into an EvtMessage. Caller is responsible for deletion. */
    virtual EvtMessage* encode_msg(RECORD_TYPE rectype);
    /** Decode an EvtMessage into a vector list of objects with names */
    virtual int decode_msg(EvtMessage* msg, std::vector<TObject*>& objlist, std::vector<std::string>& namelist);

  private:
    std::vector<TMessage*> m_buf; /**< list of messages already added. */
    std::vector<std::string> m_name; /**< names of objects stored in m_buf. */
    int m_complevel; /**< compression level, from 0 (none) to 9 (highest). */

  };

} // namespace Belle2

#endif
