//+
// File : DQMHistAnalysisOutputNSM.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-

#include <dqm/analysis/modules/DQMHistAnalysisOutputNSM.h>

#include <daq/slc/nsm/NSMNotConnectedException.h>
#include <daq/slc/runcontrol/RCNode.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

namespace Belle2 {

  /** The class for the NSM callback. */
  class DQMHistAnalysisCallback : public NSMCallback {

  public:
    /**
     * The constructor
     * @param nodename The name of the NSM node.
     * @param rcnodename The name of the RC NSM node.
     */
    DQMHistAnalysisCallback(const std::string& nodename,
                            const std::string& rcnodename)
      : NSMCallback(5), m_rcnode(rcnodename)
    {
      setNode(NSMNode(nodename));
    }
    virtual ~DQMHistAnalysisCallback() throw() {}

  public:
    /**
     * The timeout function for the NSM communication.
     */
    virtual void timouet(NSMCommunicator&) throw()
    {
      try {
      } catch (const NSMNotConnectedException& e) {
        printf("node %s is not online\n", m_rcnode.getName().c_str());
      } catch (const NSMHandlerException& e) {
        printf("NSM error %s\n", e.what());
      } catch (const IOException& e) {
        printf("timeout\n");
      }
    }

  private:
    /** The RC NSM node. */
    RCNode m_rcnode;

  };

}

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputNSM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputNSMModule::DQMHistAnalysisOutputNSMModule()
  : DQMHistAnalysisOutputModule()
{
  //Parameter definition
  addParam("NSMNodeName", m_nodename, "NSM node name for DQM Hist analyser", string("DQMH"));
  addParam("RunControlName", m_rcnodename, "NSM node name of the run control", string(""));
  B2DEBUG(1, "DQMHistAnalysisOutputNSM: Constructor done.");
}


DQMHistAnalysisOutputNSMModule::~DQMHistAnalysisOutputNSMModule() { }

void DQMHistAnalysisOutputNSMModule::initialize()
{
  ConfigFile config("slowcontrol");
  if (m_callback != nullptr) delete m_callback;
  m_callback = new DQMHistAnalysisCallback(m_nodename, m_rcnodename);
  int port = config.getInt("nsm.port");
  std::string host = config.get("nsm.host");
  PThread(new NSMNodeDaemon(m_callback, host, port));
  sleep(1);
  B2INFO("DQMHistAnalysisOutputNSM: initialized.");
}

void DQMHistAnalysisOutputNSMModule::beginRun()
{
  //B2INFO("DQMHistAnalysisOutputNSM: beginRun called.");
}

void DQMHistAnalysisOutputNSMModule::event()
{
  static bool initialized = false;
  ParamTypeList& parnames(getParNames());
  IntValueList& vints(getIntValues());
  FloatValueList& vfloats(getFloatValues());
  TextList& texts(getTexts());
  if (!initialized) {
    for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); ++i) {
      std::string pname = i->first;
      std::string vname = StringUtil::replace(pname, "/", ".");
      switch (i->second) {
        case c_ParamINT:
          m_callback->add(new NSMVHandlerInt(vname, true, false, vints[pname]));
          break;
        case c_ParamFLOAT:
          m_callback->add(new NSMVHandlerFloat(vname, true, false, vfloats[pname]));
          break;
        case c_ParamTEXT:
          m_callback->add(new NSMVHandlerText(vname, true, false, texts[pname]));
          break;
      }
    }
    initialized = true;
  } else {
    for (ParamTypeList::iterator i = parnames.begin(); i != parnames.end(); ++i) {
      std::string pname = i->first;
      std::string vname = StringUtil::replace(pname, "/", ".");
      switch (i->second) {
        case c_ParamINT:
          m_callback->set(vname, vints[pname]);
          break;
        case c_ParamFLOAT:
          m_callback->set(vname, vfloats[pname]);
          break;
        case c_ParamTEXT:
          m_callback->set(vname, texts[pname]);
          break;
      }
    }
  }
}

void DQMHistAnalysisOutputNSMModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputNSM : endRun called");
}


void DQMHistAnalysisOutputNSMModule::terminate()
{
  B2INFO("terminate called");
}

