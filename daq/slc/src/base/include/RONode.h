#ifndef _Belle2_RONode_hh
#define _Belle2_RONode_hh

#include "NSMNode.h"
#include "DataSender.h"

#include <vector>
#include <iostream>

namespace Belle2 {

  class RONode : public NSMNode {

  public:
    static const size_t MAX_SENDERS = 20;

  public:
    typedef std::vector<std::string> DataSenderList;

  public:
    RONode(const std::string& name = "")
      : NSMNode(name, "receiver_node"),
        _sender_v(MAX_SENDERS), _sender_i(0) {}
    virtual ~RONode() throw() {}

  public:
    const std::string& getScript() const throw() { return _script; }
    void setScript(const std::string& script) throw() { _script = script; }
    void addSender(const std::string& sender) throw();
    void clearSenders() throw();
    int getNSenders() const throw() { return _sender_i; }
    const std::string& getSender(int i) const throw() { return _sender_v[i]; }
    DataSenderList& getSenders() throw() { return _sender_v; }
    const DataSenderList& getSenders() const throw() { return _sender_v; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();
    virtual int getParams(const Command& command, unsigned int* pars,
                          std::string& datap);
    virtual void setParams(const Command& command, int npar,
                           const unsigned int* pars, const std::string& datap);

  private:
    DataSenderList _sender_v;
    size_t _sender_i;
    std::string _script;

  };

}

#endif
