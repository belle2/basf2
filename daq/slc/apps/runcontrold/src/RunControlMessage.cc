#include "daq/slc/apps/runcontrold/RunControlMessage.h"

using namespace Belle2;

RunControlMessage::RunControlMessage(NSMNode* node)
{
  _node = node;
}

RunControlMessage::RunControlMessage(NSMNode* node,
                                     const Command& command)
{
  _node = node;
  _command = command;
}

RunControlMessage::RunControlMessage(NSMNode* node,
                                     const Command& command,
                                     const NSMMessage& msg)
{
  _node = node;
  _command = command;
  _msg = msg;
}

RunControlMessage::RunControlMessage(NSMNode* node,
                                     const Command& command, int exp_no,
                                     int cold_start_no, int hot_restart_no)
{
  _node = node;
  _command = command;
  _msg.setNParams(3);
  _msg.setParam(0, exp_no);
  _msg.setParam(1, cold_start_no);
  _msg.setParam(2, hot_restart_no);
}

RunControlMessage::RunControlMessage(NSMNode* node,
                                     const Command& command,
                                     const std::string& text)
{
  _node = node;
  _command = command;
  _msg.setNParams(0);
  _msg.setData(text);
}

RunControlMessage::~RunControlMessage() throw()
{

}
