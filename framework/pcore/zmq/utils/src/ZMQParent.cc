/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

using namespace std;
using namespace Belle2;

ZMQParent::~ZMQParent()
{
  terminate();
}

void ZMQParent::terminate()
{
  if (m_context) {
    m_context->close();
    m_context.reset();
  }
}

void ZMQParent::reset()
{
  m_context.release();
}

std::string ZMQParent::createIdentity(unsigned int pid)
{
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  if (pid == 0) {
    pid = getpid();
  }

  return std::string(hostname) + "_" + std::to_string(pid);
}

void ZMQParent::initialize()
{
  if (m_context) {
    return;
  }
  m_context = std::make_unique<zmq::context_t>(1);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
unsigned int ZMQParent::poll(const std::vector<zmq::socket_t*>& socketList, int timeout)
{
  unsigned int return_bitmask = 0;
  zmq::pollitem_t items[socketList.size()];

  for (unsigned int i = 0; i < socketList.size(); i++) {
    items[i].socket = static_cast<void*>(*socketList[i]);
    items[i].events = ZMQ_POLLIN;
    items[i].revents = 0;
  }


  try {
    zmq::poll(items, socketList.size(), timeout);

    for (unsigned int i = 0; i < socketList.size(); i++) {
      if (static_cast<bool>(items[i].revents & ZMQ_POLLIN)) {
        return_bitmask = return_bitmask | 1 << i;
      }
    }
    return return_bitmask;
  } catch (zmq::error_t& error) {
    if (error.num() == EINTR) {
      return 0;
    } else {
      // cannot handle, rethrow exception
      throw;
    }
  }
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif