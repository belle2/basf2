#ifndef _B2DAQ_NSMDataManager_hh
#define _B2DAQ_NSMDataManager_hh

#include <xml/NodeLoader.hh>
#include <nsm/RunStatus.hh>
#include <nsm/RunConfig.hh>

#include <map>
#include <vector>

namespace B2DAQ {

  class NSMDataManager {

  private:
    typedef void* createCOPPERData_t(void*, const char*);

  public:
    NSMDataManager(NodeLoader* loader = NULL) throw();
    ~NSMDataManager() throw();

  public:
    RunStatus* allocateRunStatus() throw();
    RunConfig* allocateRunConfig() throw();
    RunStatus* getRunStatus() throw() { return _status; }
    RunConfig* getRunConfig() throw() { return _config; }
    bool writeRunStatus() throw();
    bool writeRunConfig() throw();

  private:
    NodeLoader* _loader;
    RunStatus* _status;
    RunConfig* _config;
    std::vector<void*> _handler_v;
    std::map<std::string, createCOPPERData_t*> _func_m;

  };

}

#endif
