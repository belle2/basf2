#ifndef _Belle2_HVControlMaster_h
#define _Belle2_HVControlMaster_h

#include "HVCrateInfo.h"
#include "HVCommand.h"

#include "nsm/NSMCommunicator.h"

#include "system/Mutex.h"

#include "base/NSMNode.h"

#include <map>

namespace Belle2 {

  class DBInterface;
  class TCPSocket;

  class HVControlMaster {

  public:
    typedef  std::map<std::string, HVCrateInfo*> HVCrateInfoMap;
    typedef  std::map<std::string, NSMNode*> NSMNodeMap;

  public:
    HVControlMaster() {}
    ~HVControlMaster() {}

  public:
    void add(const std::string& name, HVCrateInfo* crate);
    void perform(const std::string& command, const std::string& str);
    void accept(const TCPSocket& socket);
    void setCommunicator(NSMCommunicator* comm) { _comm = comm; }

  public:
    bool help(const std::string& str);
    bool show(const std::string& str);
    bool set(const std::string& str);
    bool get(const std::string& str);
    bool save(const std::string& str);
    bool recall(const std::string& str);
    bool list(const std::string& str);
    bool remove(const std::string& str);
    bool execute(const std::string& str, HVCommand command);
    bool quit(const std::string& str);

  private:
    HVCrateInfoMap _crate_m;
    NSMNodeMap _node_m;
    NSMCommunicator* _comm;
    Mutex _mutex;

  private:
    typedef bool (*HVControlFunc_t)(HVChannelInfo*, HVChannelStatus*,
                                    const std::string&, const std::string&);
    bool handle(HVControlFunc_t func, HVCrateInfo* crate, size_t slot, size_t channel,
                const std::string& pname, const std::string& value);
    static bool show_local(HVChannelInfo*, HVChannelStatus*, const std::string& pname, const std::string& value);
    static bool set_local(HVChannelInfo*, HVChannelStatus*, const std::string& pname, const std::string& value);
    static bool get_local(HVChannelInfo*, HVChannelStatus*, const std::string& pname, const std::string& value);
    bool save_local(DBInterface& db, const std::string& nodename);

  };

}

#endif
