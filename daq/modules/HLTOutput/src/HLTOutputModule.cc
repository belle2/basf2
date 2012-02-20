#include <daq/modules/HLTOutput/HLTOutputModule.h>

using namespace Belle2;

REG_MODULE(HLTOutput)

HLTOutputModule::HLTOutputModule() : Module()
{
  setDescription("HLTOutput module");
  setPropertyFlags(c_Input);

  std::vector<std::string> branchNames;
  addParam("branchNames", m_branchNames[0], std::string("Names of branches to be written from event"), branchNames);
  addParam("branchNamesRun", m_branchNames[1], std::string("Names of branches to be written from run"), branchNames);
  addParam("branchNamesPersistent", m_branchNames[2], std::string("Names of branches to be written from persistent"), branchNames);

  addParam("nodeType", m_nodeType, std::string("Node type of the node"));
}

HLTOutputModule::~HLTOutputModule()
{
}

void HLTOutputModule::initialize()
{
  B2INFO("Module HLTOutput initializing...");

  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(gDataOutBufferKey).c_str(), gBufferSize);
  m_msgHandler = new MsgHandler(0);

  for (int i = 0; i < DataStore::c_NDurabilityTypes; i++) {
    m_objectIterator[i] = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(i));
    m_arrayIterator[i] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(i));
    m_done[i] = false;
  }

  m_eventsSent = 0;
}

void HLTOutputModule::beginRun()
{
  B2INFO("Module HLTOutput starts a run");
}

void HLTOutputModule::event()
{
  B2INFO("Module HLTOutput starts an event");
  putData(DataStore::c_Event);
  m_eventsSent++;
  B2INFO("[HLTOutput] " << m_eventsSent << " events sent!");
}

void HLTOutputModule::endRun()
{
  B2INFO("Module HLTOutput ends a run");
}

void HLTOutputModule::terminate()
{
  sendTerminate();
  B2INFO("Module HLTOutput terminating...");
}

void HLTOutputModule::putData(const DataStore::EDurability& durability)
{
  m_msgHandler->clear();

  B2INFO("\x1b[33m[HLTOutput] Withdraw objects from DataStore...\x1b[0m");
  m_objectIterator[durability]->first();
  int nObjects = 0;

  while (!m_objectIterator[durability]->isDone()) {
    if (m_branchNames[durability].size() == 0) {
      B2INFO("[HLTOutput] Withdraw object " << m_objectIterator[durability]->key());
      m_msgHandler->add(m_objectIterator[durability]->value(), m_objectIterator[durability]->key());
      nObjects++;
    } else {
      for (unsigned int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_objectIterator[durability]->key()) {
          B2INFO("[HLTOutput] Withdraw object " << m_objectIterator[durability]->key());
          m_msgHandler->add(m_objectIterator[durability]->value(), m_objectIterator[durability]->key());
          nObjects++;
        }
      }
    }
    m_objectIterator[durability]->next();
  }

  B2INFO("\x1b[33m[HLTOutput] Withdraw arrays from DataStore...\x1b[0m");
  m_arrayIterator[durability]->first();
  int nArrays = 0;

  while (!m_arrayIterator[durability]->isDone()) {
    if (m_branchNames[durability].size() == 0) {
      B2INFO("[HLTOutput] Withdraw array " << m_arrayIterator[durability]->key());
      m_msgHandler->add(m_arrayIterator[durability]->value(), m_arrayIterator[durability]->key());
      nArrays++;
    } else {
      for (unsigned int i = 0; i < m_branchNames[durability].size(); i++) {
        if (m_branchNames[durability][i] == m_arrayIterator[durability]->key()) {
          B2INFO("[HLTOutput] Withdraw array " << m_arrayIterator[durability]->key());
          m_msgHandler->add(m_arrayIterator[durability]->value(), m_arrayIterator[durability]->key());
          nArrays++;
        }
      }
    }
    m_arrayIterator[durability]->next();
  }

  EvtMessage* msg = m_msgHandler->encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = static_cast<int>(durability);
  (msg->header())->reserved[1] = nObjects;
  (msg->header())->reserved[2] = nArrays;
  B2INFO("\x1b[33m[HLTOutput] Encoding data...size=" << msg->size() << "\x1b[0m");

  std::string sendingData(msg->buffer());
  B2INFO("\x1b[33m[HLTOutput] nObjects = " << nObjects << " / nArrays = " << nArrays << "\x1b[0m");

  //writeFile(msg->buffer(), msg->size());
  //testData (msg->buffer ());

  //B2INFO("[HLTOutput] Put an event into the ring buffer");
  while (m_buffer->insq((int*)msg->buffer(), msg->size() / 4 + 1) <= 0) {
    usleep(100);
  }
}

EHLTStatus HLTOutputModule::testData(char* buffer)
{
  std::vector<TObject*> objectList;
  std::vector<std::string> nameList;

  B2INFO("\x1b[33m[HLTOutput] Decoding data..\x1b[0m");
  EvtMessage* msg = new EvtMessage(buffer);
  m_msgHandler->decode_msg(msg, objectList, nameList);

  B2INFO("\x1b[33m[HLTOutput] Storing data into DataStore..\x1b[0m");
  msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nObjects = msg->header()->reserved[1];
  int nArrays = msg->header()->reserved[2];

  B2INFO("\x1b[33m[HLTOutput] nObjects = " << nObjects << " / nArrays = " << nArrays << "\x1b[0m");

  B2INFO("\x1b[33m[HLTOutput] Storing objects...\x1b[0m");
  for (int i = 0; i < nObjects; i++) {
    if (!DataStore::Instance().storeObject(objectList[i], nameList[i]), durability)
      return c_FuncError;
  }

  B2INFO("\x1b[33m[HLTOutput] Storing arrays...\x1b[0m");
  for (int i = 0; i < nArrays; i++) {
    if (!DataStore::Instance().storeArray((TClonesArray*)objectList[nObjects + i], nameList[nObjects + i]), durability)
      return c_FuncError;
  }

  return c_Success;
}

void HLTOutputModule::sendTerminate()
{
  //std::string message("Terminate");

  B2INFO("[HLTOutput] Termination code sending");
  //while (m_buffer->insq ((int*)message.c_str (), message.size () / 4 + 1) <= 0) {
  while (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
    usleep(100);
  }
}

bool HLTOutputModule::checkData(std::string data1, char* data2)
{
  char* data1Transform = (char*)data1.c_str();

  for (unsigned int i = 0; i < data1.size(); i++) {
    if (data1Transform[i] != data2[i]) {
      B2INFO("[HLTOutput] They are different at " << i);
      return false;
    }
  }

  B2INFO("[HLTOutput] They are the same");
  return true;
}

void HLTOutputModule::writeFile(char* data, int size)
{
  FILE* fp;
  fp = fopen("output", "a");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fprintf(fp, "\n");
  fclose(fp);
}
