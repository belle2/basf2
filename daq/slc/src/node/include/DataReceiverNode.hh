#ifndef _B2DAQ_DataReceiverNode_hh
#define _B2DAQ_DataReceiverNode_hh

#include "NSMNode.hh"
#include "DataSender.hh"

#include <vector>
#include <iostream>

#define MAX_SENDERS 20

namespace B2DAQ {

  class DataReceiverNode : public NSMNode {

  public:
    typedef std::vector<DataSender*> DataSenderList;

  public:
    DataReceiverNode(const std::string& name = "")
      : NSMNode(name, "receiver_node"), 
	_sender_v(MAX_SENDERS), _sender_i(0) {}
    virtual ~DataReceiverNode() throw() {}

  public:
    const std::string& getScript() const throw() { return _script; }
    void setScript(const std::string& script) throw() { _script = script; }
    void addSender(DataSender* sender) throw();
    void clearSenders() throw();
    int getNSenders() const throw() { return _sender_i; }
    DataSender* getSender(int i) throw() { return _sender_v[i]; }
    DataSenderList& getSenders() throw() { return _sender_v; }
    const DataSenderList& getSenders() const throw() { return _sender_v; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();
    
  private:
    DataSenderList _sender_v;
    int _sender_i;
    std::string _script;

  };

}

#endif
