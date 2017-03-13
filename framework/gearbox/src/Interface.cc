/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Interface.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;

namespace Belle2 {
  namespace gearbox {

    vector<GearDir> Interface::getNodes(const string& path) const
    {
      int size = getNumberNodes(path);
      vector<GearDir> result;
      result.reserve(size);
      for (int i = 1; i <= size; ++i) {
        result.push_back(GearDir(*this, path, i));
      }
      return result;
    }

    string Interface::getString(const string& path, const string& defaultValue) const
    {
      try {
        return getString(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }

    double Interface::getDouble(const string& path) const noexcept(false)
    {
      string value = getString(path);
      try {
        return boost::lexical_cast<double>(value);
      } catch (boost::bad_lexical_cast) {
        throw ConversionError() << path << value;
      }
    }

    double Interface::getDouble(const string& path, double defaultValue) const noexcept(false)
    {
      try {
        return getDouble(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }

    int Interface::getInt(const string& path) const noexcept(false)
    {
      string value = getString(path);
      try {
        //hide spurious warning in boost
#if !defined(__GNUG__) || defined(__clang__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        return boost::lexical_cast<int>(value);
#if !defined(__GNUG__) || defined(__clang__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif
      } catch (boost::bad_lexical_cast) {
        throw ConversionError() << path << value;
      }
    }

    int Interface::getInt(const string& path, int defaultValue) const noexcept(false)
    {
      try {
        return getInt(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }

    bool Interface::getBool(const string& path) const noexcept(false)
    {
      string value = getString(path);
      boost::to_lower(value);
      if (value.empty() || value == "false" || value == "off" || value == "0") return false;
      return true;
    }

    bool Interface::getBool(const string& path, bool defaultValue) const
    {
      try {
        return getBool(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }

    double Interface::getWithUnit(const string& path) const noexcept(false)
    {
      pair<string, string> value = getStringWithUnit(path);
      double numValue(0);
      try {
        numValue = boost::lexical_cast<double>(value.first);
      } catch (boost::bad_lexical_cast) {
        throw ConversionError() << path << value.first;
      }
      if (value.second.empty()) {
        B2WARNING("Requested unit conversion for parameter '" << path << "' but no unit found");
      } else {
        numValue = Unit::convertValue(numValue, value.second);
      }
      return numValue;
    }

    double Interface::getWithUnit(const string& path, double defaultValue) const noexcept(false)
    {
      try {
        return getWithUnit(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }

    vector<double> Interface::getArray(const string& path) const noexcept(false)
    {
      typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
      boost::char_separator<char> sep(",; \t\n\r");

      pair<string, string> value = getStringWithUnit(path);
      tokenizer tokens(value.first, sep);
      vector<double> result;
      double numValue(0);
      for (const string& tok : tokens) {
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

    vector<double> Interface::getArray(const string& path, const vector<double>& defaultValue) const noexcept(false)
    {
      try {
        return getArray(path);
      } catch (PathEmptyError) {
        return defaultValue;
      }
    }


    string Interface::ensureNode(const string& path) const
    {
      return boost::trim_right_copy_if(path, boost::is_any_of("/"));
    }

    string Interface::ensurePath(const string& path) const
    {
      return ensureNode(path) + '/';
    }

    string Interface::addIndex(const string& path, int index) const
    {
      static boost::format index_fmt("%1%[%2%]");
      return (index_fmt % ensureNode(path) % index).str();
    }

    string Interface::joinPath(const string& path, const string& subpath) const
    {
      return ensurePath(path) + boost::trim_left_copy_if(subpath, boost::is_any_of("/"));
    }

  }
}
