#ifndef _B2DAQ_RunControlMessage_hh
#define _B2DAQ_RunControlMessage_hh

#include <node/Command.hh>

#include <nsm/NSMMessage.hh>

#include <queue>

namespace B2DAQ {

  class RunControlMessage {

  public:
    static const unsigned int FLAG_MODE;
    static const unsigned int FLAG_OPERATORS;
    static const unsigned int FLAG_RUN_TYPE;
    static const unsigned int FLAG_RUN_VERSION;
    static const unsigned int FLAG_RUN_LENGTH;
    static const unsigned int FLAG_EXP_NO;
    static const unsigned int FLAG_RUN_NO;
    static const unsigned int FLAG_START_TIME;
    static const unsigned int FLAG_END_TIME;
    static const unsigned int FLAG_TOTAL_TRIGGER;

  public:
    enum HostID {
      RUNCONTROLLER = 0,
      LOCALNSM,
      BRIGDE,
      GUI
    };

  public:
    RunControlMessage() {}
    RunControlMessage(HostID id) {
      _id = id;
    }
    RunControlMessage(HostID id, const NSMMessage& msg) {
      _id = id;
      _msg = msg;
    }
    ~RunControlMessage() throw() {}

  public:
    void setId(HostID id) { _id = id; }
    void setMessage(const NSMMessage& msg) { _msg = msg; }
    HostID getId() { return _id; }
    const NSMMessage& getMessage() const { return _msg; }
    void setCommand(const Command& command) throw() {
      _command = command;
    }
    const Command& getCommand() const throw() {
      return _command;
    }

  private:
    HostID _id;
    NSMMessage _msg;
    Command _command;

  };

};

#endif

