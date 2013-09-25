#include "State.hh"

using namespace B2DAQ;

State::State() throw() : Enum(Enum::UNKNOWN) {}

State::State(const Enum& num) throw() : Enum(num) {}

State::State(const State& st) throw() : Enum(st) {}

State::State(int id, const char* label, const char* alias)
throw() : Enum(id, label, alias) {}

State::~State() throw() {}

