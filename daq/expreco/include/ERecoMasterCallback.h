/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RFMasterCallback_h
#define _Belle2_RFMasterCallback_h

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <vector>

namespace Belle2 {

  class ERecoRunControlCallback;

  class ERecoMasterCallback : public RCCallback {

  public:
    ERecoMasterCallback(ConfigFile& config);
    virtual ~ERecoMasterCallback() {}

  public:
    void initialize(const DBObject& obj) override;
    void configure(const DBObject& obj) override;
    void monitor() override;
    void ok(const char* node, const char* data) override;
    void error(const char* node, const char* data) override;
    void load(const DBObject& obj, const std::string& runtype) override;
    void start(int expno, int runno) override;
    void stop() override;
    void recover(const DBObject& obj, const std::string& runtype) override;
    bool resume(int subno) override;
    bool pause() override;
    void abort() override;
    bool perform(NSMCommunicator& com) override;

  public:
    void setState(NSMNode& node, const RCState& state);
    void setCallback(ERecoRunControlCallback* callback) { m_callback = callback; }
    void addData(const std::string& dataname, const std::string& format);

  private:
    ERecoRunControlCallback* m_callback;
    StringList m_dataname;
    typedef std::vector<NSMNode> NSMNodeList;
    NSMNodeList m_nodes;
    NSMNode m_rcnode;
    std::string m_script;
  };

}

#endif
