#ifndef _Belle2_RecieverManager_hh
#define _Belle2_RecieverManager_hh

#include "daq/slc/base/NSMNode.h"

namespace Belle2 {

  class RecieverManager {

  public:
    RecieverManager(NSMNode* node, const std::string& buf_path,
                    const std::string& fifo_path)
      : _node(node), _buf_path(buf_path), _fifo_path(fifo_path) {}
    virtual ~RecieverManager() {}

  public:
    virtual void run() throw();

  private:
    NSMNode* _node;
    std::string _buf_path;
    std::string _fifo_path;

  };

}

#endif
