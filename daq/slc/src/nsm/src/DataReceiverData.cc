#include "DataReceiverData.hh"

#include <node/DataReceiverNode.hh>

extern "C" {
#include <nsm2.h>
#include "receiver_data.h"
}

#include <cstring>
#include <iostream>

using namespace B2DAQ;

DataReceiverData::DataReceiverData(const std::string& data_name) throw()
  : NSMData(data_name, "receiver_data", receiver_data_revision) {}

void DataReceiverData::read(NSMNode* node) throw(NSMHandlerException)
{
  receiver_data* data = (receiver_data*)get();
  const size_t ncopper = data->ncopper;
  DataReceiverNode* recv = (DataReceiverNode*)node;
  recv->setScript(data->script);
  if (ncopper != recv->getSenders().size()) {
    recv->clearSenders();
    for (size_t i = 0; i < ncopper; i++) {
      recv->addSender(new DataSender());
    }
  }
  //std::cout << __FILE__ << ":" << __LINE__ << ":" << data->ncopper << " " << recv->getNSenders() << std::endl;
  for (size_t i = 0; i < ncopper; i++) {
    DataSender* sender = recv->getSender(i);
    if (sender != NULL) {
      sender->setHost((const char*)(data->host + i * 64));
      sender->setPort(data->port[i]);
      sender->setEventSize(data->event_size[i]);
    }
  }
}

void DataReceiverData::write(NSMNode* node) throw(NSMHandlerException)
{
  receiver_data* data = (receiver_data*)get();
  DataReceiverNode* recv = (DataReceiverNode*)node;
  data->ncopper = (short)recv->getNSenders();
  strncpy(data->script, recv->getScript().c_str(), 64);
  //std::cout << __FILE__ << ":" << __LINE__ << ":" << data->ncopper << " " << recv->getNSenders() << std::endl;
  for (int i = 0; i < recv->getNSenders(); i++) {
    DataSender* sender = recv->getSender(i);
    if (sender != NULL) {
      //std::cout << __FILE__ << ":" << __LINE__ << ":" << sender->getHost() << std::endl;
      strncpy((char*)(data->host + i * 64), sender->getHost().c_str(), 64);
      data->port[i] = sender->getPort();
      data->event_size[i] = sender->getEventSize();
    }
  }
}

