#ifndef iTopUtils_H
#define iTopUtils_H

#include <iostream>
#include <sstream>


typedef unsigned int packet_word_t;

template <typename T> std::string NumberToString(T Number)
{
  std::ostringstream ss;
  ss << Number;
  return ss.str();
}

#endif
