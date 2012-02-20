#include <daq/modules/HLTInput/HLTInputModule.h>

using namespace Belle2;

REG_MODULE(HLTInput)

HLTInputModule::HLTInputModule() : Module()
{
  setDescription("HLTInput module");
  setPropertyFlags(c_Input);

  addParam("nodeType", m_nodeType, std::string("Node type of the node"));
  addParam("dataSources", m_nDataSources, std::string("# of data sources"), 1);
}

HLTInputModule::~HLTInputModule()
{
}

void HLTInputModule::initialize()
{
  B2INFO("Module HLTInput initializing...");

  m_inBuffer = new RingBuffer(boost::lexical_cast<std::string>(gDataInBufferKey).c_str(), gBufferSize);
  m_outBuffer = new RingBuffer(boost::lexical_cast<std::string>(gDataOutBufferKey).c_str(), gBufferSize);

  m_msgHandler = new MsgHandler(0);
  m_msgHandler->clear();

  m_eventsTaken = 0;
}

void HLTInputModule::beginRun()
{
  B2INFO("Module HLTInput starts a run");
}

void HLTInputModule::event()
{
  /*
  while (getData () == c_TermCalled) {
    usleep (100);
  }
  */
  getData();
  m_eventsTaken++;

  B2INFO("[HLTInput] " << m_eventsTaken << " events taken!");

  /*
  EHLTStatus status = c_Success;

  status = getData ();
  while (status != c_TermCalled) {
    if (m_buffer->numq () > 0) {
      status = getData ();
    }
  }
  */
}

void HLTInputModule::endRun()
{
  B2INFO("Module HLTInput ends a run");
}

void HLTInputModule::terminate()
{
  B2INFO("Module HLTInput terminating...");
}

EHLTStatus HLTInputModule::getData()
{
  std::vector<TObject*> objectList;
  std::vector<std::string> nameList;

  char buffer[gMaxReceives];
  memset(buffer, 0, gMaxReceives);

  int size = 0;
  while ((size = m_inBuffer->remq((int*)buffer)) <= 0)
    usleep(100);

  //writeFile(buffer, size * 4);

  std::string termChecker(buffer);
  while (termChecker == gTerminate) {
    B2INFO("\x1b[34m[HLTInput] Termination requested\x1b[0m");
    while (m_outBuffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
      usleep(100);
    }
    m_nDataSources--;
    if (m_nDataSources == 0) {
      B2INFO("\x1b[34m[HLTInput] All data taken. Terminating...\x1b[0m");
      return c_TermCalled;
    } else {
      B2INFO("\x1b[34m[HLTInput] " << m_nDataSources << " more data sources are left...\x1b[0m");
      while ((size = m_inBuffer->remq((int*)buffer)) <= 0)
        usleep(100);
      termChecker = std::string(buffer);
      //return c_Success;
    }
  }

  EvtMessage* msg = new EvtMessage(buffer);
  m_msgHandler->decode_msg(msg, objectList, nameList);

  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nObjects = msg->header()->reserved[1];
  int nArrays = msg->header()->reserved[2];

  B2INFO("\x1b[33m[HLTInput] nObjects = " << nObjects << " / nArrays = " << nArrays << "\x1b[0m");

  for (int i = 0; i < nObjects; i++) {
    //B2INFO ("[HLTInput] Storing object " << nameList[i]);
    DataStore::Instance().storeObject(objectList[i], nameList[i], durability);
    /*
    if (!DataStore::Instance().storeObject(objectList[i], nameList[i], durability)) {
      B2ERROR ("[HLTInput] Storing object into DataStore failed!");
      return c_FuncError;
    }
    */
  }

  for (int i = 0; i < nArrays; i++) {
    //B2INFO ("[HLTInput] Storing array " << nameList[nObjects + i]);
    DataStore::Instance().storeArray((TClonesArray*)objectList[nObjects + i], nameList[nObjects + i], durability);
    //DataStore::Instance().storeObject(objectList[nObjects + i], nameList[nObjects + i], durability);
    /*
    if (!DataStore::Instance().storeArray((TClonesArray*)objectList[nObjects + i], nameList[nObjects + i], durability)) {
      B2ERROR ("[HLTInput] Storing array into DataStore failed!");
      return c_FuncError;
    }
    */
  }

  //B2INFO("\x1b[33m[HLTInput] Received data is stored in DataStore!\x1b[0m");

  /*
  if (m_buffer->numq() > 0) {
    B2INFO("[HLTInput] Events existing in the ring buffer (numq=" << m_buffer->numq() << ")");
  } else {
    B2INFO("[HLTInput] No events left in the ring buffer");
  }
  */

  return c_Success;
  //return c_TermCalled;
}

void HLTInputModule::writeFile(char* data, int size)
{
  FILE* fp;
  fp = fopen("input", "a");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fprintf(fp, "\n");
  fclose(fp);
}
