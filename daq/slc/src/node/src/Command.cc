#include "Command.hh"
#include "State.hh"

using namespace B2DAQ;

const Command Command::OK(1, "OK", "OK");
const Command Command::ERROR(2, "ERROR", "ERROR");

Command::Command() throw(): Enum(Enum::UNKNOWN) {}

Command::Command(const Enum& num) throw() : Enum(num) {}

Command::Command(const Command& cmd) throw() : Enum(cmd) {}

Command::Command(int id, const char* label, const char* alias)
throw() : Enum(id, label, alias) {}

Command::~Command() throw() {}


