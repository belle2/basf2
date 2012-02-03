#include <daq/modules/HLTInput/HLTInputModule.h>

using namespace Belle2;

REG_MODULE(HLTInput)

HLTInputModule::HLTInputModule() : Module()
{
  setDescription("HLTInput module");
  setPropertyFlags(c_Input);
}

HLTInputModule::~HLTInputModule()
{
}

void HLTInputModule::initialize()
{
  B2INFO("Module HLTInput initializing...");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_DataOutPort)).c_str(), gBufferSize);

  m_msgHandler = new MsgHandler(0);
  m_msgHandler->clear();
}

void HLTInputModule::beginRun()
{
  B2INFO("Module HLTInput starts a run");
}

void HLTInputModule::event()
{
  B2INFO("Module HLTInput starts an event");

  getData();

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
  while ((size = m_buffer->remq((int*)buffer)) <= 0)
    usleep(100);

  writeFile(buffer, size * 4);

  std::string termChecker(buffer);
  if (termChecker == gTerminate)
    return c_TermCalled;

  EvtMessage* msg = new EvtMessage(buffer);
  m_msgHandler->decode_msg(msg, objectList, nameList);

  B2INFO("\x1b[33m[HLTInput] Storing data into DataStore..(size="
         << msg->size() << " bytes)\x1b[0m");
  msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nObjects = msg->header()->reserved[1];
  int nArrays = msg->header()->reserved[2];

  B2INFO("\x1b[33m[HLTInput] nObjects = " << nObjects << " / nArrays = " << nArrays << "\x1b[0m");

  for (int i = 0; i < nObjects; i++) {
    if (!DataStore::Instance().storeObject(objectList[i], nameList[i]), durability)
      return c_FuncError;
  }

  for (int i = 0; i < nArrays; i++) {
    if (!DataStore::Instance().storeArray((TClonesArray*)objectList[nObjects + i], nameList[nObjects + i]), durability)
      return c_FuncError;
  }

  B2INFO("\x1b[33m[HLTInput] Received data is stored in DataStore!\x1b[0m");

  if (m_buffer->numq() > 0) {
    B2INFO("[HLTInput] Events existing in the ring buffer (numq=" << m_buffer->numq() << ")");
  } else {
    B2INFO("[HLTInput] No events left in the ring buffer");
  }

  //return c_Success;
  return c_TermCalled;
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
