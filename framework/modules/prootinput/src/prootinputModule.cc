//+
// File : prootinput.cc
// Description : Input module with parallel processing support
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Jun - 2010
//-

#include <framework/modules/prootinput/prootinputModule.h>

#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pRootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pRootInputModule::pRootInputModule() : pEventServer()
{
  setDescription("pBasf2: ROOT input module");
  setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData | c_Input | c_ParallelProcessingCertified);

  m_file = NULL;

  //Parameter definition
  // Input file Name
  addParam("inputFileName", m_inputFileName, "TFile name.", string("SimpleInput.root"));

  // Event number to be skipped
  addParam("eventNumber", m_eventNumber, "Skip this number of events before starting.", 0);

  // Compression level for message passing
  addParam("compLevel", m_complevel, "Compression factor for msg_handler", 0);

  // Tree names
  addParam("EventTree", m_treeNames[0], "TTree name for event data. NONE for no input.", string("tree"));
  addParam("RunTree", m_treeNames[1], "TTree name for run data. NONE for no input.", string("NONE"));
  addParam("PersistentTree", m_treeNames[2], "TTree name for persistent data. NONE for no input.", string("NONE"));

  // Branch names
  vector<string> tmpbranch;
  addParam("EventBranch", m_branchNames[0], "Names of branches to be read into event map. Empty means all branches.", tmpbranch);
  addParam("RunBranch", m_branchNames[1], "Names of branches to be read into run map. Empty means all branches.", tmpbranch);
  addParam("PersistentBranch", m_branchNames[2], "Names of branches to be read into persistent map. Empty means all branches.", tmpbranch);

  B2INFO("pRootInput:Constructor done.")
}


pRootInputModule::~pRootInputModule()
{
}

void pRootInputModule::initialize()
{
  static TClonesArray* sarray;

  B2INFO(boost::format("address of sarray ptr of ptr = %1%, ptr = %2%\n") % &sarray % sarray)

  //Open TFile
  m_file = new TFile(m_inputFileName.c_str(), "READ");
  m_file->cd();
  if (!m_file) {B2FATAL("Input file " + m_inputFileName + " doesn't exist");}
  B2INFO("Opened file " + m_inputFileName);

  // Setup TTrees
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) { // Loop over durabilities
    //Get TTree
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = dynamic_cast<TTree*>(m_file->Get(m_treeNames[ii].c_str()));
      if (!m_tree[ii]) {
        B2FATAL("TTree " + m_treeNames[ii] + " doesn't exist");
        continue;
      }
      B2INFO("Opened tree " + m_treeNames[ii]);

      //Count number of objects and arrays in the TTree
      TObjArray* branches = m_tree[ii]->GetListOfBranches();
      for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
        TBranch* branch = validBranch(jj, branches);
        if (!branch) continue;
        if (static_cast<string>(branch->GetClassName()) != "TClonesArray") {
          TObject* obj = new TObject();
          obj = NULL;
          branch->SetAddress(&obj);
          m_objects[ii].push_back(obj);
          m_objnames[ii].push_back((string)branch->GetName());
        } else  {
          TClonesArray* array = new TClonesArray();
          //    TObject** array = new TObject*;
          //    TClonesArray** array = &sarray;
          array = NULL;
          branch->SetAddress(&array);
          m_arrays[ii].push_back((TClonesArray*)array);
          m_arraynames[ii].push_back((string)branch->GetName());

        }
        branch->GetEntry(0);
      }
    }
  }

  // Number of events in c_Event tree
  m_nevt = m_tree[DataStore::c_Event]->GetEntries();

  // Attach to ring buffer if nprocess > 0
  m_nproc = Framework::nprocess();

  //  printf ( "pRootInput : nproc = %d\n", m_nproc );
  B2WARNING("pRootInput : nproc = " << m_nproc)
  if (m_nproc > 0) {
    char rbName[] = "PRIN";
    m_rbuf = new RingBuffer(rbName, RINGBUF_SIZE);
    m_msghandler = new MsgHandler(m_complevel);
  } else {
    m_rbuf = NULL;
    m_msghandler = NULL;
  }

}


void pRootInputModule::beginRun()
{
  //  cout << "beginRun called" << endl;
}


void pRootInputModule::event()
{

  int status;
  if (m_nproc == 0) {
    m_file->cd();
    status = readTree(DataStore::c_Event);
  } else {
    status = readRingBuf(DataStore::c_Event);
  }
  if (status == MSG_TERMINATE)
    setProcessRecordType(prt_EndOfData); // EoF detected

}


void pRootInputModule::endRun()
{
  //  cout << "endRun called" << endl;
}


void pRootInputModule::terminate()
{
  //  cout << "Term called" << endl;
}


void pRootInputModule::setupTFile()
{
}


int pRootInputModule::readTree(const DataStore::EDurability& durability)
{
  // Fill m_objects
  //  B2WARNING("Durability" << durability)
  m_tree[durability]->GetEntry(m_eventNumber);

  // Restore objects in DataStore
  int nobjs = m_objects[durability].size();
  for (int i = 0; i < nobjs; i++) {
    DataStore::Instance().storeObject(m_objects[durability].at(i),
                                      m_objnames[durability].at(i));
  }
  // Restore arrays in DataStore
  int narrays = m_arrays[durability].size();
  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray(m_arrays[durability].at(i),
                                     m_arraynames[durability].at(i));
  }

  if (durability == DataStore::c_Event) {
    m_eventNumber++;
    if (m_eventNumber > m_nevt)
      return MSG_TERMINATE;
    else
      return MSG_EVENT;
  } else
    return MSG_BEGIN_RUN;
}

int pRootInputModule::readRingBuf(const DataStore::EDurability& indurability)
{
  // Get a record from ringbuf
  int size;

  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    usleep(100);
  }

  // Build EvtMessage and decompose it
  vector<TObject*> objlist;
  vector<string> namelist;
  EvtMessage* msg = new EvtMessage(evtbuf);    // Have EvtMessage by ptr cpy
  if (msg->type() == MSG_TERMINATE)
    return msg->type(); // EOF
  m_msghandler->decode_msg(msg, objlist, namelist);

  // Get Object info
  RECORD_TYPE type = msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = (msg->header())->reserved[1];
  int narrays = (msg->header())->reserved[2];


  delete[] evtbuf;

  // Restore objects in DataStore
  for (int i = 0; i < nobjs; i++) {
    DataStore::Instance().storeObject(objlist.at(i),
                                      m_objnames[durability].at(i));
  }
  // Restore arrays in DataStore
  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs),
                                     m_arraynames[durability].at(i));
  }
  return type;
}

TBranch* pRootInputModule::validBranch(int& ibranch, TObjArray* branches)
{
  TBranch* branch = static_cast<TBranch*>(branches->At(ibranch));
  if (!branch) {
    return 0;
  }

  // if there is a branch list, count only, if the name is on the list.
  string name = "";
  name = static_cast<string>(branch->GetName());
  string branchNames = "";
  vector<string>::iterator it;
  for (it = m_branchNames[0].begin(); it < m_branchNames[0].end(); it++) {
    branchNames = *it + " ";
  }

  if ((m_branchNames[0].size()) && (branchNames.find(name) == string::npos)) {
    return 0;
  }

  return branch;
}

//
// Event server which is executed in a separate process
//
void pRootInputModule::event_server(void)
{
  //  printf ( "----> Event Server Invoked\n" );
  B2INFO("----> Event Server Invoked");
  while (1) {
    if (m_eventNumber > m_nevt) {
      // Send termination record
      EvtMessage* term = m_msghandler->encode_msg(MSG_TERMINATE);
      for (int i = 0; i < m_nproc; i++) {
        int sval;
        while ((sval = m_rbuf->insq((int*)(term->buffer()),
                                    (term->size() - 1) / sizeof(int) + 1)) < 0) {
          usleep(200);
        }
      }
      exit(0);
    }

    DataStore::EDurability durability = DataStore::c_Event;
    // Fill m_objects
    m_tree[durability]->GetEntry(m_eventNumber);
    //    m_tree->GetEntry(m_eventNumber);

    // Clear msghandler
    m_msghandler->clear();

    // Stream objects in msg_handler
    int nobjs = m_objects[durability].size();
    for (int i = 0; i < nobjs; i++) {
      m_msghandler->add(m_objects[durability].at(i),
                        m_objnames[durability].at(i));
    }
    // Stream arrays in msg_handler
    int narrays = m_arrays[durability].size();
    for (int i = 0; i < narrays; i++) {
      m_msghandler->add(m_arrays[durability].at(i),
                        m_arraynames[durability].at(i));
    }

    // Encode event message
    EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);

    (msg->header())->reserved[0] = (int)durability;
    (msg->header())->reserved[1] = nobjs;       // No. of objects
    (msg->header())->reserved[2] = narrays;    // No. of arrays

    // Put the message in ring buffer
    for (;;) {
      int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
      if (stat >= 0) break;
      usleep(200);
    }

    //    printf ( "----> Msg queued in RingBuffer : size=%d (words)\n",
    //       (msg->size()-1)/4+1 );

    // Count up event numbder
    m_eventNumber++;

  }
}
