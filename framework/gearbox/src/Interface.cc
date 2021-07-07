/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/Interface.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace Belle2::gearbox {

  std::vector<GearDir> Interface::getNodes(const std::string& path) const
  {
    int size = getNumberNodes(path);
    std::vector<GearDir> result;
    result.reserve(size);
    for (int i = 1; i <= size; ++i) {
      result.emplace_back(*this, path, i);
    }
    return result;
  }

  std::string Interface::getString(const std::string& path, const std::string& defaultValue) const
  {
    try {
      return getString(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }

  double Interface::getDouble(const std::string& path) const noexcept(false)
  {
    std::string value = getString(path);
    try {
      return boost::lexical_cast<double>(value);
    } catch (boost::bad_lexical_cast&) {
      throw ConversionError() << path << value;
    }
  }

  double Interface::getDouble(const std::string& path, double defaultValue) const noexcept(false)
  {
    try {
      return getDouble(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }

  int Interface::getInt(const std::string& path) const noexcept(false)
  {
    std::string value = getString(path);
    try {
      //hide spurious warning in boost
      return boost::lexical_cast<int>(value);
    } catch (boost::bad_lexical_cast&) {
      throw ConversionError() << path << value;
    }
  }

  int Interface::getInt(const std::string& path, int defaultValue) const noexcept(false)
  {
    try {
      return getInt(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }

  bool Interface::getBool(const std::string& path) const noexcept(false)
  {
    std::string value = getString(path);
    boost::to_lower(value);
    if (value.empty() || value == "false" || value == "off" || value == "0") return false;
    return true;
  }

  bool Interface::getBool(const std::string& path, bool defaultValue) const
  {
    try {
      return getBool(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }

  double Interface::getWithUnit(const std::string& path) const noexcept(false)
  {
    std::pair<std::string, std::string> value = getStringWithUnit(path);
    double numValue(0);
    try {
      numValue = boost::lexical_cast<double>(value.first);
    } catch (boost::bad_lexical_cast&) {
      throw ConversionError() << path << value.first;
    }
    if (value.second.empty()) {
      B2WARNING("Requested unit conversion for parameter '" << path << "' but no unit found");
    } else {
      numValue = Unit::convertValue(numValue, value.second);
    }
    return numValue;
  }

  double Interface::getWithUnit(const std::string& path, double defaultValue) const noexcept(false)
  {
    try {
      return getWithUnit(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }

  std::vector<double> Interface::getArray(const std::string& path) const noexcept(false)
  {
    using tokenizer = boost::tokenizer<boost::char_separator<char> >;
    boost::char_separator<char> sep(",; \t\n\r");

    std::pair<std::string, std::string> value = getStringWithUnit(path);
    tokenizer tokens(value.first, sep);
    std::vector<double> result;
    double numValue(0);
    for (const std::string& tok : tokens) {
      try {
        numValue = boost::lexical_cast<double>(tok);
      } catch (boost::bad_lexical_cast&) {
        throw (ConversionError() << path << value.first);
      }
      if (!value.second.empty()) {
        numValue = Unit::convertValue(numValue, value.second);
      }
      result.push_back(numValue);
    }
    return result;
  }

  std::vector<double> Interface::getArray(const std::string& path, const std::vector<double>& defaultValue) const noexcept(false)
  {
    try {
      return getArray(path);
    } catch (PathEmptyError&) {
      return defaultValue;
    }
  }


  std::string Interface::ensureNode(const std::string& path) const
  {
    return boost::trim_right_copy_if(path, boost::is_any_of("/"));
  }

  std::string Interface::ensurePath(const std::string& path) const
  {
    return ensureNode(path) + '/';
  }

  std::string Interface::addIndex(const std::string& path, int index) const
  {
    static boost::format index_fmt("%1%[%2%]");
    return (index_fmt % ensureNode(path) % index).str();
  }

  std::string Interface::joinPath(const std::string& path, const std::string& subpath) const
  {
    return ensurePath(path) + boost::trim_left_copy_if(subpath, boost::is_any_of("/"));
  }

}
