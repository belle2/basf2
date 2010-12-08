#include <iostream>
#include <fstream>
#include "framework/dcore/EventServer.h"

using namespace Belle2;

EventServer::EventServer(void)
{
}

//EventServer::EventServer (HLTBuffer* inBuffer, HLTBuffer* outBuffer) {
EventServer::EventServer(RingBuffer* inBuffer, RingBuffer* outBuffer)
{
  m_inBuffer = inBuffer;
  m_outBuffer = outBuffer;
}

EventServer::EventServer(std::vector<std::string> inBuffer, std::vector<std::string> outBuffer)
{
};

EventServer::~EventServer(void)
{
}

void EventServer::process(const std::string data)
{
  /*
  std::ifstream in (data.c_str ());
  std::string buf;
  while (std::getline (in, buf)) {
    m_outBuffer->put (buf);
  }
  */
}
