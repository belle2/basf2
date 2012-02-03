#include <wait.h>
#include <daq/modules/RingBufferTest/RingBufferTestModule.h>

using namespace Belle2;

REG_MODULE(RingBufferTest)

RingBufferTestModule::RingBufferTestModule() : Module()
{
  setDescription("RingBufferTest module");
  setPropertyFlags(c_Input);
}

RingBufferTestModule::~RingBufferTestModule()
{
  if (m_flag)
    delete m_buffer;
}

void RingBufferTestModule::initialize()
{
  B2INFO("Module RingBufferTest initializing...");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_DataOutPort)).c_str(), gBufferSize);
  m_output.open("output.log", std::fstream::out | std::fstream::app);

  m_flag = false;
}

void RingBufferTestModule::beginRun()
{
  B2INFO("Module RingBufferTest starts a run");
}

void RingBufferTestModule::event()
{
  B2INFO("Module RingBufferTest starts an event");
  int i = 0;
  pid_t a, b;

  a = fork();
  if (a == 0) {
    m_buffer2 = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_DataOutPort)).c_str(), gBufferSize);
    for (int aa = 0; aa < 100000000; aa++) {
      std::string testMessage("This is a test message");
      i++;
      while (m_buffer->insq((int*)testMessage.c_str(), testMessage.size() / 4 + 1) < 0) {
        usleep(100);
      }
      if (i % 10000 == 0)
        B2INFO(i << "th events inserted");
    }
  } else {
    b = fork();
    if (b == 0) {
      sleep(1);
      m_buffer3 = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_DataOutPort)).c_str(), gBufferSize);
      for (int bb = 0; bb < 100000000; bb++) {
        char testMessage2[gMaxReceives];
        i++;
        while (m_buffer->remq((int*)testMessage2) <= 0) {
          usleep(100);
        }
        if (i % 10000 == 0)
          B2INFO(i << "th events popped");
      }
    } else {
      m_flag = true;
      int status;
      waitpid(a, &status, 0);
      waitpid(b, &status, 0);
    }
  }
}

void RingBufferTestModule::endRun()
{
  B2INFO("Module RingBufferTest ends a run");
}

void RingBufferTestModule::terminate()
{
  m_output.close();
  B2INFO("Module RingBufferTest terminating...");
}
