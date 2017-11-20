/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <regex>


namespace Belle2 {

  std::map<std::string, std::string> getSubstitutionMap()
  {
    return std::map<std::string, std::string> {
      {" ", "__sp"},
      {",", "__cm"},
      {":", "__cl"},
      {"=", "__eq"},
      {"<", "__st"},
      {">", "__gt"},
      {".", "__pt"},
      {"+", "__pl"},
      {"-", "__mi"},
      {"(", "__bo"},
      {")", "__bc"},
      {"{", "__co"},
      {"}", "__cc"},
      {"[", "__so"},
      {"]", "__sc"},
      {"`", "__to"},
      {"´", "__tc"},
      {"^", "__ha"},
      {"°", "__ci"},
      {"$", "__do"},
      {"§", "__pa"},
      {"%", "__pr"},
      {"!", "__em"},
      {"?", "__qm"},
      {";", "__sm"},
      {"#", "__hs"},
      {"*", "__mu"},
      {"/", "__sl"},
      {"\\", "__bl"},
      {"'", "__sq"},
      {"\"", "__dq"},
      {"~", "__ti"},
      {"-", "__da"},
      {"|", "__pi"},
      {"&", "__am"},
      {"@", "__at"},
    };
  }

  std::string makeROOTCompatible(std::string str)
  {
    if (str.find("__") != std::string::npos) {
      B2WARNING("String passed to makeROOTCompatible contains double-underscore __, which is used internally for escaping special characters. "
                "It is recommended to avoid this. However escaping a string twice with makeROOTCompatible is safe, but will print this warning. "
                "Passed string: " + str);
    }
    auto replace = getSubstitutionMap();
    for (auto& pair : replace) {
      boost::replace_all(str, pair.first, pair.second);
    }
    const static std::regex blackList("[^a-zA-Z0-9_]");
    return std::regex_replace(str, blackList, "");
  }

  std::string invertMakeROOTCompatible(std::string str)
  {
    auto replace = getSubstitutionMap();
    for (auto& pair : replace) {
      boost::replace_all(str, pair.second, pair.first);
    }
    return str;
  }

}
