//+
// File : prootoutput.cc
// Description : ROOT output module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/modules/prootoutput/prootoutputModule.h>

#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pRootOutputModule::pRootOutputModule() : pOutputServer()
{
  //Set module properties
  setDescription("ROOT output with parallel capability");
  setPropertyFlags(c_Output | c_ParallelProcessingCertified);

  m_steerTreeNames.push_back("treeName");
  m_steerTreeNames.push_back("treeNameRun");
  m_steerTreeNames.push_back("treeNamePersistent");

  m_steerBranchNames.push_back("branchNames");
  m_steerBranchNames.push_back("branchNamesRun");
  m_steerBranchNames.push_back("branchNamesPersistent");

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "TFile name.", string("pRootOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);

  addParam(m_steerTreeNames[0], m_treeNames[0], "TTree name for event data. NONE for no output.", string("tree"));
  addParam(m_steerTreeNames[1], m_treeNames[1], "TTree name for run data. NONE for no output.", string("NONE"));
  addParam(m_steerTreeNames[2], m_treeNames[2], "TTree name for peristent data. NONE for no output.", string("NONE"));

  vector<string> branchNames;
  addParam(m_steerBranchNames[0], m_branchNames[0], "Names of branches to be written from event map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[1], m_branchNames[1], "Names of branches to be written from run map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[2], m_branchNames[2], "Names of branches to be written from persistent map. Empty means all branches.", branchNames);


  B2INFO("pRootOutput: Constructor done.");
}


pRootOutputModule::~pRootOutputModule()
{
}

void pRootOutputModule::initialize()
{

  // get iterators
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    m_obj_iter[ii]   = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(ii));
    m_array_iter[ii] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(ii));
    m_done[ii]     = false;
  }

  // Attach to ring buffer if nprocess > 0
  m_nproc = Framework::nprocess();

  //  printf ( "pRootInput : nproc = %d\n", m_nproc );
  B2WARNING("pRootInput : nproc = " << m_nproc)
  if (m_nproc > 0) {
    char temp[] = "PROUTXXXXXX";
    char* rbufname = mktemp(temp);
    m_rbuf = new RingBuffer(rbufname, RINGBUF_SIZE);
  } else {
    setupTFile();
    m_rbuf = NULL;
  }
  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("pRootOutput initialized.");
}


void pRootOutputModule::beginRun()
{
  B2INFO("beginRun called.");
}

void pRootOutputModule::event()
{
  //fill Event data
  if (Framework::nprocess() == 0) {
    fillTree(DataStore::c_Event);
  } else {
    fillRingBuf(DataStore::c_Event);
  }

  //  B2INFO ( "Event sent : " << m_nsent++ )
}

void pRootOutputModule::fillTree(const DataStore::EDurability& durability)
{
  if (!m_done[durability]) {
    setupTTree(durability);
    B2INFO("SetupTTree done!!!!");
  }

  // Loop over objects
  m_obj_iter[durability]->first();
  int nobj = 0;
  while (!m_obj_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_objects[durability][nobj] = m_obj_iter[durability]->value();
      m_objbrs[durability][nobj]->SetAddress(&m_objects[durability][nobj]);
      nobj++;
    } else {
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_obj_iter[durability]->key()) {
          m_objects[durability][nobj] = m_obj_iter[durability]->value();
          m_objbrs[durability][nobj]->SetAddress(&m_objects[durability][nobj]);
          nobj++;
        }
      }
    }
    m_obj_iter[durability]->next();
  }

  // Loop over arrays
  m_array_iter[durability]->first();
  int narray = 0;
  while (!m_array_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_arrays[durability][narray] = (TClonesArray*)m_array_iter[durability]->value();
      m_arraybrs[durability][narray]->SetAddress(&m_arrays[durability][nobj]);
      narray++;
    } else {
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_array_iter[durability]->key()) {
          m_arrays[durability][narray] = (TClonesArray*)m_array_iter[durability]->value();
          m_arraybrs[durability][narray]->SetAddress(&m_arrays[durability][nobj]);
          narray++;
        }
      }
    }
    m_array_iter[durability]->next();
  }
  m_tree[durability]->Fill();
}


void pRootOutputModule::fillRingBuf(const DataStore::EDurability& durability)
{
  m_msghandler->clear();

  // Collect objects and place them in msghandler

  // Loop over objects
  m_obj_iter[durability]->first();
  int nobj = 0;
  while (!m_obj_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_msghandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
      nobj++;
    } else {
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_obj_iter[durability]->key()) {
          m_msghandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
          nobj++;
        }
      }
    }
    m_obj_iter[durability]->next();
  }

  // Loop over arrays
  m_array_iter[durability]->first();
  int narray = 0;
  while (!m_array_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      m_msghandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
      narray++;
    } else {
      for (int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_array_iter[durability]->key()) {
          m_msghandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
          narray++;
        }
      }
    }
    m_array_iter[durability]->next();
  }

  // Encode EvtMessage
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobj;       // No. of objects
  (msg->header())->reserved[2] = narray;    // No. of arrays

  // Parallel process: Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    usleep(200);
  }
  delete msg;

  //  B2INFO ( "Event sent : " << m_nsent++ )
}

void pRootOutputModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void pRootOutputModule::terminate()
{
  // Single process mode
  if (Framework::nprocess() == 0)  {
    //write the trees
    m_file->cd();
    for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
      if (m_treeNames[ii] != "NONE") {
        B2INFO("Write TTree " << m_treeNames[ii]);
        m_tree[ii]->Write();
      }
    }
    delete m_file;
  } else { // Parallel processing
    // Send termination packet to output server
    EvtMessage* term = m_msghandler->encode_msg(MSG_TERMINATE);
    int sval;
    while ((sval = m_rbuf->insq((int*)(term->buffer()),
                                (term->size() - 1) / sizeof(int) + 1)) < 0) {
      usleep(200);
    }
  }
  B2INFO("terminate called")
}

size_t pRootOutputModule::getSizeOfObj(const DataStore::EDurability& durability)
{
  int sizeCounter = 0;
  m_obj_iter[durability]->first();
  while (!m_obj_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      sizeCounter++;
    } else {
      for (size_t ii = 0; ii < m_branchNames[durability].size(); ++ii) {
        if (m_branchNames[durability][ii] == m_obj_iter[durability]->key()) {
          sizeCounter++;
        }
      }
    }
    m_obj_iter[durability]->next();
  }
  return sizeCounter;
}

size_t pRootOutputModule::getSizeOfArray(const DataStore::EDurability& durability)
{
  int sizeCounter = 0;
  m_array_iter[durability]->first();
  while (!m_array_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) {
      sizeCounter++;
    } else {
      for (size_t ii = 0; ii < m_branchNames[durability].size(); ++ii) {
        if (m_branchNames[durability][ii] == m_array_iter[durability]->key()) {
          sizeCounter++;
        }
      }
    }
    m_array_iter[durability]->next();
  }
  return sizeCounter;
}

void pRootOutputModule::setupTFile()
{
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = new TTree(m_treeNames[ii].c_str(), m_treeNames[ii].c_str());
      m_tree[ii]->SetAutoSave(1000000000);
    }
  }
}

// Setup ttree
void pRootOutputModule::setupTTree(const DataStore::EDurability& durability)
{
  B2INFO("pRootOutput: TTree is being set up output server.");

  // Count number of objects in DataStore
  int nobjs = getSizeOfObj(durability);
  int narrays = getSizeOfArray(durability);

  cout << "nobjs = " << nobjs << " narrays = " << narrays << endl;

  // Connect objects to branch
  m_obj_iter[durability]->first();
  while (!m_obj_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) { // branchNames are empty
      TObject* obj = new TObject();
      obj = m_obj_iter[durability]->value();
      TBranch* br = m_tree[durability]->Branch((m_obj_iter[durability]->key()).c_str(), &obj);
      m_objbrs[durability].push_back(br);
      m_objects[durability].push_back(obj);
      m_objnames[durability].push_back(m_obj_iter[durability]->key());
    } else {
      for (size_t jj = 0; jj < m_branchNames[durability].size(); jj++) {
        if (m_branchNames[durability][jj] == m_obj_iter[durability]->key()) {
          TObject* obj = new TObject();
          obj = m_obj_iter[durability]->value();
          TBranch* br = m_tree[durability]->Branch((m_obj_iter[durability]->key()).c_str(), &obj);
          m_objbrs[durability].push_back(br);
          m_objects[durability].push_back(obj);
          m_objnames[durability].push_back(m_obj_iter[durability]->key());
          break; // if branch is found get out of for loop.
        }
      }
    }
    m_obj_iter[durability]->next();
  }

  // Connect arrays to branch
  m_array_iter[durability]->first();
  while (!m_array_iter[durability]->isDone()) {
    if (!(m_branchNames[durability].size())) { // branchNames are empty
      //      TObject* obj = new TObject();
      TClonesArray* obj = new TClonesArray((m_array_iter[durability]->key()).c_str());
      obj = (TClonesArray*)m_array_iter[durability]->value();
      TBranch* br = m_tree[durability]->Branch((m_array_iter[durability]->key()).c_str(), &obj);
      m_arraybrs[durability].push_back(br);
      m_arrays[durability].push_back((TClonesArray*)obj);
      m_arraynames[durability].push_back(m_array_iter[durability]->key());
    } else {
      for (size_t jj = 0; jj < m_branchNames[durability].size(); jj++) {
        if (m_branchNames[durability][jj] == m_array_iter[durability]->key()) {
          //    TClonesArray* obj = new TClonesArray();
          TClonesArray* obj = new TClonesArray((m_array_iter[durability]->key()).c_str());
          obj = (TClonesArray*)m_array_iter[durability]->value();
          //    m_tree[durability]->Branch((m_array_iter[durability]->key()).c_str(), &obj );
          TBranch* br = m_tree[durability]->Branch((m_array_iter[durability]->key()).c_str(), &obj);
          m_arraybrs[durability].push_back(br);
          m_arrays[durability].push_back((TClonesArray*)obj);
          m_arraynames[durability].push_back(m_obj_iter[durability]->key());
          break; // if branch is found get out of for loop.
        }
      }
    }
    m_array_iter[durability]->next();
  }
  m_done[durability] = true;
}


// Output Server function
void pRootOutputModule::output_server(void)
{
  B2INFO("----> Output Server Invoked");

  // Open output ROOT file
  setupTFile();

  char* evtbuf = new char[MAXEVTSIZE];

  int size;
  vector<TObject*> objlist;
  vector<string> namelist;

  while (1) {
    // Pick up one buffer from RingBuffer
    while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
      usleep(100);
    }
    if (size < 0) {
      B2WARNING("pRootOutput : output server : error in remq")
      exit(-99);
    }
    //    printf ( "Output Server: got a record = %d\n", size );

    // Form EvtMessage
    EvtMessage* msg = new EvtMessage(evtbuf);

    // Check for termination
    if (msg->type() == MSG_TERMINATE) {
      m_file->cd();
      for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
        if (m_treeNames[ii] != "NONE") {
          B2INFO("Write TTree " << m_treeNames[ii]);
          m_tree[ii]->Write();
        }
      }
      delete[] evtbuf;
      B2INFO("Output Server terminated")
      exit(0);
    }

    // Decode EvtMessage
    objlist.clear();
    namelist.clear();
    int status = m_msghandler->decode_msg(msg, objlist, namelist);

    // Retrieve record info
    DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
    int nobj = (msg->header())->reserved[1];
    int narray = (msg->header())->reserved[2];

    // Setup TTree for the first event
    if (!m_done[durability]) {
      // Restore datastore
      for (int i = 0; i < nobj; i++)
        DataStore::Instance().storeObject(objlist[i], namelist[i]);
      for (int i = 0; i < narray; i++)
        DataStore::Instance().storeArray((TClonesArray*)objlist[i],
                                         namelist[i]);
      setupTTree(durability);
      B2INFO("Output server : TTrees initialized");
    }

    // Copy objects in Branch buffers
    for (int i = 0; i < nobj; i++) {
      //      m_objects[durability][i] = objlist[i];
      m_objbrs[durability][i]->SetAddress(&objlist[i]);
    }
    for (int i = 0; i < narray; i++) {
      //      m_arrays[durability][i] = (TClonesArray*)objlist[i+nobj];
      m_arraybrs[durability][i]->SetAddress(&objlist[i+nobj]);
    }

    // Fill TTree
    m_tree[durability]->Fill();

    // Clear DataStore
    DataStore::Instance().clearMaps(durability);

    //    B2INFO ( "Event received = " << m_nrecv++ )
  }
}

