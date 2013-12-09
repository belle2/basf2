#ifndef _Belle2_RunControlMessage_hh
#define _Belle2_RunControlMessage_hh

#include <daq/slc/base/Command.h>
#include <daq/slc/base/NSMNode.h>

#include <daq/slc/nsm/NSMMessage.h>

namespace Belle2 {

  class RunControlMessage {

  public:
    RunControlMessage(NSMNode* node = NULL);
    RunControlMessage(NSMNode* node, const Command& command);
    RunControlMessage(NSMNode* node, const Command& command,
                      const NSMMessage& msg);
    RunControlMessage(NSMNode* node,
                      const Command& command, int exp_no,
                      int cold_start_no, int hot_restart_no);
    RunControlMessage(NSMNode* node, const Command& command,
                      const std::string& text);
    ~RunControlMessage() throw();

  public:
    void setNode(NSMNode* node) { _node = node; }
    NSMNode* getNode() throw() { return _node; }
    const NSMNode* getNode() const throw() { return _node; }
    void setMessage(const NSMMessage& msg) { _msg = msg; }
    const NSMMessage& getMessage() const { return _msg; }
    NSMMessage& getMessage() { return _msg; }
    void setCommand(const Command& command) throw() { _command = command; }
    const Command& getCommand() const throw() { return _command; }
    Command& getCommand() throw() { return _command; }

  private:
    NSMNode* _node;
    Command _command;
    NSMMessage _msg;

  };

};

#endif
