/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/FrameworkExceptions.h>
#include <framework/gearbox/Unit.h>

#include <string>

class TObject;

namespace Belle2 {

  class GearDir;

  namespace gearbox {
    /** Exception to be thrown in case of an empty result */
    BELLE2_DEFINE_EXCEPTION(PathEmptyError, "Path '%1%' does not exist or is empty")
    /** Exception to be thrown in case of an conversion error */
    BELLE2_DEFINE_EXCEPTION(ConversionError, "Path '%1%': Could not convert '%2%' to numerical value")
    /** getTObject() couldn't deserialize data. */
    BELLE2_DEFINE_EXCEPTION(TObjectConversionError, "Path '%1%': Could not convert to TObject! Malformed data?")

    /** common interface for Gearbox and GearDir. */
    class Interface {
    public:

      /** default constructor initializing to toplevel parameter space */
      Interface(): m_path("/") {}

      /** empty, virtual destructor */
      virtual ~Interface() {}

      /**
       * Return the number of nodes a given path will expand to
       * @return number of nodes, 0 if path does not exist
       */
      virtual int getNumberNodes(const std::string& path = "") const = 0;

      /**
       * Check if a given parameter path exists
       * @return true if path exists, false otherwise
       */
      bool exists(const std::string& path = "") const { return getNumberNodes(path) > 0; }

      /**
       * Convert to bool to simplify checking of existence
       * @return true if path exists, false otherwise
       */
      operator bool() const { return exists(); }


      /**
       * Get vector of GearDirs which point to all the nodes the given path evaluates to
       * @param path Path of the parameters to get
       */
      std::vector<GearDir> getNodes(const std::string& path = "") const;

      /** Return path of the current interface */
      std::string getPath() const { return m_path; }

      /**
       * Get the parameter path as a string.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      virtual std::string getString(const std::string& path = "") const noexcept(false) = 0;

      /**
       * Get the parameter path as string and also return the unit it was defined with.
       *
       * If no unit was defined, an empty string will be returned for the unit.
       * No parsing of the unit is performed, this funtion is primarily used by
       * getWithUnit.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      virtual std::pair<std::string, std::string> getStringWithUnit(const std::string& path) const noexcept(false) = 0;

      /**
       * Get the parameter path as a string.
       * if the parameter is empty or does not exist, the defaultvalue will be
       * returned.
       * @param path path of the parameter to get
       * @param defaultValue value to return if the path es empty or does not exist
       * @return value of the parameter
       */
      std::string getString(const std::string& path, const std::string& defaultValue) const;

      /**
       * Get the parameter path as a TObject
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::TObjectConversionError if the value could not be deserialized
       * @param path Path of the parameter to get
       * @return pointer to object, owned and managed by gearbox. Object will
       *         be deleted once it is no longer valid (e.g. after the current
       *         run if it belongs to this run)
       */
      virtual const TObject* getTObject(const std::string& path) const noexcept(false) = 0;

      /**
       * Get the parameter path as a double.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getDouble(const std::string& path = "") const noexcept(false);

      /**
       * Get the parameter path as a double.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getDouble(const std::string& path, double defaultValue) const noexcept(false);

      /**
       * Get the parameter path as a int.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      int getInt(const std::string& path = "") const noexcept(false);

      /**
       * Get the parameter path as a int.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      int getInt(const std::string& path, int defaultValue) const noexcept(false);

      /**
       * Get the parameter path as a bool.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      bool getBool(const std::string& path = "") const noexcept(false);

      /**
       * Get the parameter path as a bool.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      bool getBool(const std::string& path, bool defaultValue) const;

      /**
       * Get the parameter path as a double converted to the standard unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getWithUnit(const std::string& path) const noexcept(false);

      /**
       * Get the parameter path as a double converted to the standard unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getWithUnit(const std::string& path, double defaultValue) const noexcept(false);

      /**
       * Get the parameter path as a list of double values converted to the standard unit.
       *
       * The content of the parameter will be split at whitespace, comma and
       * semi-colon and returned as a vector of numbers
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */

      std::vector<double> getArray(const std::string& path) const noexcept(false);

      /**
       * Get the parameter path as a list of double values converted to the standard unit.
       *
       * The content of the parameter will be split at whitespace, comma and
       * semi-colon and returned as a vector of numbers
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      std::vector<double> getArray(const std::string& path, const std::vector<double>& defaultValue) const noexcept(false);


      /**
       * Get the parameter path as a double converted to the standard length unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getLength(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard length unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getLength(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

      /**
       * Get the parameter path as a double converted to the standard angle unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getAngle(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard angle unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getAngle(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

      /**
       * Get the parameter path as a double converted to the standard energy unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getEnergy(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard energy unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getEnergy(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

      /**
       * Get the parameter path as a double converted to the standard density unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getDensity(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard density unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getDensity(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

      /**
       * Get the parameter path as a double converted to the standard time unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getTime(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard time unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getTime(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

      /**
       * Get the parameter path as a double converted to the standard frequency unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getFrequency(const std::string& path = "") const noexcept(false)
      {
        return getWithUnit(path);
      }

      /**
       * Get the parameter path as a double converted to the standard frequency unit.
       *
       * Gearbox will try to determine the unit with which the parameter was
       * saved and convert it to the default system. If no unit can be
       * determinded, it will be assumed that the parameter does not need to be
       * converted.
       *
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       *
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getFrequency(const std::string& path, double defaultValue) const noexcept(false)
      {
        return getWithUnit(path, defaultValue);
      }

    protected:

      /** make sure the path really corresponds to an XPath node expression by removing trailing slashes */
      std::string ensureNode(const std::string& path) const ;
      /** make sure the path really corresponds to a path by appending a trailing slash if neccessary */
      std::string ensurePath(const std::string& path) const;
      /** add [index] to the path (after stripping trailing slashes) */
      std::string addIndex(const std::string& path, int index) const;
      /** joind to paths, inserting a slash if neccessary */
      std::string joinPath(const std::string& path, const std::string& subpath) const;

      /** path of the current interface */
      std::string m_path;

    };
  }
}
