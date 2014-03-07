#ifndef _Belle2_HVNodeInfo_h
#define _Belle2_HVNodeInfo_h

#include "daq/slc/apps/hvcontrold/HVState.h"
#include "daq/slc/apps/hvcontrold/HVChannelInfo.h"
#include "daq/slc/apps/hvcontrold/hv_status.h"

#include <daq/slc/nsm/NSMNode.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/base/Serializable.h>

#include <string>
#include <vector>

namespace Belle2 {

  typedef std::vector<HVChannelInfo> HVChannelInfoList;

  class HVNodeInfo {

  public:
    static const int MAX_CHANNELS = 20;

  public:
    HVNodeInfo(NSMNode* node = NULL) : _node(node), _configid(0) {}
    ~HVNodeInfo() throw() {}

  public:
    NSMNode* getNode() throw() { return _node; }
    int getConfigId() throw() { return _configid; }
    HVChannelInfo& getChannel(int index) throw() { return _info_v[index]; }
    HVChannelInfoList& getChannelList() throw() { return _info_v; }
    void setNode(NSMNode* node) throw() { _node = node; }
    void setConfigId(int id) throw() { _configid = id; }
    void reset(size_t ninfos) throw() {
      _info_v = HVChannelInfoList();
      _info_v.resize(ninfos);
    }
    void addChannel(const HVChannelInfo& ch) { _info_v.push_back(ch); }

  public:
    void writeConfig(Writer& writer) const throw(IOException);
    void writeStatus(Writer& writer) const throw(IOException);
    void readConfig(Reader& reader) throw(IOException);
    void readStatus(Reader& reader) throw(IOException);
    void loadStatus(hv_status* status) throw();

  public:
    int create(DBInterface* db) throw();
    int save(DBInterface* db) throw();
    int load(DBInterface* db) throw();
    int getLatestConfigId(DBInterface* db) throw();
    void readFile(const std::string& filename) throw();

  private:
    NSMNode* _node;
    int _configid;
    mutable HVChannelInfoList _info_v;

  };

}

#endif
