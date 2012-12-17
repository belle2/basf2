/*
 * pEventPrxocessor.cc
 *
 *  Created on: Oct 26, 2010
 *      Author: R.Itoh, IPNS, KEK
 *  Updated : Jul. 25, 2011
 */

#include <framework/pcore/pEventProcessor.h>
#include <framework/core/Environment.h>
#include <framework/core/PathManager.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>

#include <signal.h>


using namespace std;
using namespace Belle2;

pEventProcessor::pEventProcessor(PathManager& pathManager) : EventProcessor(pathManager),
  procHandler(new ProcHandler())
{ }


pEventProcessor::~pEventProcessor()
{
  delete procHandler;
}


void pEventProcessor::process(PathPtr spath)
{
  if (spath->getModules().size() == 0) return;

  signal(SIGSEGV, SIG_DFL);

  const int numProcesses = Environment::Instance().getNumberProcesses();

  // 0. Should serial processing be used instead?
  if (numProcesses == 0) {
    B2WARNING("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
    EventProcessor::process(spath);
    return;
  }

  // 1. Initialization
  ModulePtrList modulelist = m_pathManager.buildModulePathList(spath);
  //  processInitialize(modulelist);
  //  dump_modules ( "full : ", modulelist );
  ModulePtrList initmodules = init_modules_in_main(modulelist);
  processInitialize(initmodules);
  //  dump_modules ( "extracted : ", initmodules );

  // 2. Analyze start path and split into parallel paths
  m_histoflag = false;
  analyze_path(spath);
  B2INFO("process : inlistpath size = " << m_inpathlist.size());
  B2INFO("process : bodypathlist size = " << m_bodypathlist.size());
  B2INFO("process : outpathlist size = " << m_outpathlist.size());

  dump_path("Input Path ", m_inpathlist[0]);
  for (unsigned int i = 0; i < m_bodypathlist.size(); i++) {
    dump_path("Main Path ", m_bodypathlist[i]);
  }
  for (unsigned int i = 0; i < m_outpathlist.size(); i++) {
    dump_path("Output Path ", m_outpathlist[i]);
  }

  // 3. Fork  input path
  procHandler->init_EvtServer();
  if (procHandler->isEvtServer()) {   // In event server process
    PathPtr& inpath = m_inpathlist[0];
    ModulePtrList inpath_modules = m_pathManager.buildModulePathList(inpath);
    ModulePtrList procinitmodules = init_modules_in_process(inpath_modules);
    if (procinitmodules.size() > 0)
      processInitialize(procinitmodules);
    processCore(inpath, inpath_modules);
    processTerminate(inpath_modules);
    B2INFO("Event Server Terminated");
    exit(0);
  }

  // 4. Fork out output path
  int nout = 0;
  for (std::vector<PathPtr>::iterator it = m_outpathlist.begin(); it != m_outpathlist.end(); ++it) {
    PathPtr& outpath = *it;
    if ((outpath->getModules()).size() > 0) {
      procHandler->init_OutServer(nout);
      if (procHandler->isOutputSrv()) {   // In output server process
        m_master = 0; //allow resetting master module
        ModulePtrList outpath_modules = m_pathManager.buildModulePathList(outpath);
        ModulePtrList procinitmodules = init_modules_in_process(outpath_modules);
        if (procinitmodules.size() > 0)
          processInitialize(procinitmodules);
        processCore(outpath, outpath_modules);
        processTerminate(outpath_modules);
        B2INFO("Output Server Terminated");
        exit(0);
      }
      nout++;
    }
  }

  // 5. Fork out main path
  fflush(stdout);
  procHandler->init_EvtProc(numProcesses);
  if (procHandler->isEvtProc()) {
    m_master = 0; //allow resetting master module
    PathPtr& mainpath = m_bodypathlist[m_bodypathlist.size() - 1];
    ModulePtrList main_modules = m_pathManager.buildModulePathList(mainpath);
    ModulePtrList procinitmodules = init_modules_in_process(main_modules);
    if (procinitmodules.size() > 0)
      processInitialize(procinitmodules);
    processCore(mainpath, main_modules);
    processTerminate(main_modules);
    B2INFO("Event Process Terminated");
    exit(0);
  }

  // 6. Framework process
  if (procHandler->isFramework()) {
    // 6.0 Build End of data message
    EvtMessage* term = new EvtMessage(NULL, 0, MSG_TERMINATE);
    // 6.1 Wait for input path to terminate
    procHandler->wait_event_server();
    // 6.2 Send termination to event processes
    for (int i = 0; i < numProcesses; i++) {
      for (std::vector<RingBuffer*>::iterator it = m_rbinlist.begin();
           it != m_rbinlist.end(); ++it) {
        RingBuffer* rbuf = *it;
        while (rbuf->insq((int*)term->buffer(),
                          (term->size() - 1) / sizeof(int) + 1) < 0) {
          usleep(200);
        }
      }
    }
    // 6.3 Wait for event processes to terminate
    procHandler->wait_event_processes();
    // 6.4 Send termination to output servers
    for (std::vector<RingBuffer*>::iterator it = m_rboutlist.begin();
         it != m_rboutlist.end(); ++it) {
      RingBuffer* rbuf = *it;
      while (rbuf->insq((int*)term->buffer(),
                        (term->size() - 1) / sizeof(int) + 1) < 0) {
        usleep(200);
      }
    }
    procHandler->wait_output_server();
    B2INFO("All processes completed");

    // 6.5 Remove all ring buffers
    for (std::vector<RingBuffer*>::iterator it = m_rbinlist.begin();
         it != m_rbinlist.end(); ++it)
      delete *it;
    for (std::vector<RingBuffer*>::iterator it = m_rboutlist.begin();
         it != m_rboutlist.end(); ++it)
      delete *it;

    delete term;
    B2INFO("process: completed");
  }
}


void pEventProcessor::analyze_path(PathPtr& path, Module* inmod, int cstate)
{

  const ModulePtrList& modlist = path->getModules();
  ModulePtrList::const_iterator iter;

  ModulePtrList inlist;
  ModulePtrList mainlist;
  ModulePtrList outlist;

  // Set state
  int state = 0;
  if (inmod != NULL) {
    state = 1;
    B2DEBUG(0, "Analyze Path: called with condition module");
  }

  // Loop over modules on the path
  for (iter = modlist.begin(); iter != modlist.end(); iter++) {
    ModulePtr modptr = *iter;
    Module* module = iter->get();
    // Check Module property
    if (state == 0) {   // non-parallel modules for input
      /*
      if ( iter == modlist.begin() &&
      module->hasProperties ( Module::c_HistogramManager ) ) {
      if ( module->hasProperties ( Module::c_HistogramManager ) ) {
      m_histoflag = true;
      m_histoman = *iter;  // Register histomanager module if exists
      }
      */
      if (!module->hasProperties(Module::c_ParallelProcessingCertified)) {
        inlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=0, Module " << module->getName() << " added to inlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
        if (module->hasProperties(Module::c_HistogramManager)) {
          // Initialize histogram manager if found in the path
          m_histoflag = true;
          m_histoman = *iter;
          module->initialize();
        }
      } else {
        state = 1;
        if (inlist.size() > 0) {
          // Create RingBuffer
          //          RingBuffer* rbuf = new RingBuffer((char*)"input", RBUFSIZE);
          RingBuffer* rbuf = new RingBuffer(RBUFSIZE);
          m_rbinlist.push_back(rbuf);
          // Insert Tx at the end of current path
          ModulePtr txptr(new TxModule(rbuf));
          inlist.push_back(txptr);
          // Inserv Rx at the top of next path
          ModulePtr rxptr(new RxModule(rbuf));
          mainlist.push_back(rxptr);
          B2DEBUG(0, "Analyze Path : state=0->1, Tx and Rx are inserted");
          if (m_histoflag) {
            mainlist.push_back(m_histoman);
            B2DEBUG(0, "Analyze Path : state=0->1, HistoMan is inserted");
          }
        }
        mainlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=0->1, Module " << module->getName() << " added to mainlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      }
    } else if (state == 1) {
      if (module->hasProperties(Module::c_ParallelProcessingCertified)) {
        mainlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=1, Module " << module->getName() << " added to mainlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      } else {
        state = 2;
        if (mainlist.size() > 0 || (cstate == 1 && mainlist.size() == 0)) {
          // Create RingBuffer
          //          RingBuffer* rbuf = new RingBuffer((char*)"output", RBUFSIZE);
          RingBuffer* rbuf = new RingBuffer(RBUFSIZE);
          m_rboutlist.push_back(rbuf);
          // Insert Tx at the end of current path
          ModulePtr txptr(new TxModule(rbuf));
          mainlist.push_back(txptr);
          // Insert Rx at the top of next path
          ModulePtr rxptr(new RxModule(rbuf));
          outlist.push_back(rxptr);
          B2DEBUG(0, "Analyze Path : state=1->2, Tx and Rx are inserted");
          if (m_histoflag) {
            outlist.push_back(m_histoman);
            B2DEBUG(0, "Analyze Path : state=1->2, HistoMan is inserted");
          }
        }
        outlist.push_back(modptr);
        B2DEBUG(0, "Analyze Path : state=1->2, Module " << module->getName() << " added to outlist");
        if (module->hasCondition()) {
          B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
          PathPtr next = module->getConditionPath();
          analyze_path(next, module, state);
        }
      }
    } else if (state == 2) {
      // Modules after non-parallel module are all placed in output path.
      B2DEBUG(0, "Analyze Path : state=2, Module " << module->getName() << " added to outlist");
      outlist.push_back(modptr);
      if (module->hasCondition()) {
        B2DEBUG(0, "Analyze Path : Condition detected, recursive call");
        PathPtr next = module->getConditionPath();
        analyze_path(next, module, state);
      }
    }
  }
  B2INFO("Analyze Path : mainlist size = " << mainlist.size());
  B2INFO("Analyze Path : inlist size = " << mainlist.size());
  B2INFO("Analyze Path : outlist size = " << mainlist.size());

  PathPtr inpath(new Path);
  if (inlist.size() > 0) {
    inpath->putModules(inlist);
    m_inpathlist.push_back(inpath);
  }
  PathPtr bodypath(new Path);
  if (mainlist.size() > 0) {
    bodypath->putModules(mainlist);
    m_bodypathlist.push_back(bodypath);
  }
  PathPtr outpath(new Path);
  if (outlist.size() > 0) {
    outpath->putModules(outlist);
    m_outpathlist.push_back(outpath);
  }

  // Set new condition path to condition module
  if (inmod != NULL && mainlist.size() > 0) {
    inmod->setConditionPath(bodypath);
  }

  return;
}

void pEventProcessor::dump_path(const std::string title, PathPtr path)
{
  const ModulePtrList& modlist = path->getModules();
  ModulePtrList::const_iterator it;
  std::ostringstream strbuf;
  strbuf << title << "(" << to_hex(path) << ") : ";
  for (it = modlist.begin(); it != modlist.end(); ++it) {
    Module* module = it->get();
    strbuf << module->getName();
    if (module->hasCondition()) {
      PathPtr condpath = module->getConditionPath();
      strbuf << "[->" << to_hex(condpath) << "] ";
    }
    if (*it != modlist.back())
      strbuf << " -> ";
  }
  B2INFO(strbuf.str());
}

void pEventProcessor::dump_modules(const std::string title, const ModulePtrList modlist)
{
  ModulePtrList::const_iterator it;
  std::ostringstream strbuf;
  strbuf << title << " : ";
  for (it = modlist.begin(); it != modlist.end(); ++it) {
    Module* module = it->get();
    strbuf << module->getName();
    if (module->hasCondition()) {
      PathPtr condpath = module->getConditionPath();
      strbuf << "[->" << to_hex(condpath) << "] ";
    }
    if (*it != modlist.back())
      strbuf << " -> ";
  }
  B2INFO(strbuf.str());
}

std::string pEventProcessor::to_hex(PathPtr& path)
{
  ostringstream os;
  os.flags(ios_base::hex);
  os.width(8);
  os.fill('0');
  os << path;
  //  void* adrs = (void*) path;
  //  os << (unsigned int)adrs;
  return os.str();
}

ModulePtrList pEventProcessor::init_modules_in_main(const ModulePtrList& modlist)
{
  ModulePtrList tmpModuleList;
  ModulePtrList::const_iterator listIter;

  for (listIter = modlist.begin(); listIter != modlist.end(); listIter++) {
    Module* module = listIter->get();
    ModulePtr ptr = *listIter;
    if (module->hasProperties(Module::c_InitializeInMain))
      tmpModuleList.push_back(ptr);
  }

  return tmpModuleList;
}

ModulePtrList pEventProcessor::init_modules_in_process(const ModulePtrList& modlist)
{
  ModulePtrList tmpModuleList;
  ModulePtrList::const_iterator listIter;

  for (listIter = modlist.begin(); listIter != modlist.end(); listIter++) {
    Module* module = listIter->get();
    if (!module->hasProperties(Module::c_InitializeInMain))
      tmpModuleList.push_back(*listIter);
  }

  return tmpModuleList;
}
