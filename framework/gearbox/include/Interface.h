/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOX_INTERFACE_H
#define GEARBOX_INTERFACE_H

#include <string>
#include <framework/core/FrameworkExceptions.h>
#include <framework/gearbox/Unit.h>

namespace Belle2 {

  class GearDir;

  namespace gearbox {
    /** Exception to be thrown in case of an empty result */
    BELLE2_DEFINE_EXCEPTION(PathEmptyError, "Path '%1%' does not exist or is empty");
    /** Exception to be thrown in case of an conversion error */
    BELLE2_DEFINE_EXCEPTION(ConversionError, "Path '%1%': Could not convert '%2%' to numerical value");

    class Interface {
    public:

      /** default constructor initializing to toplevel parameter space */
      Interface(): m_path("/") {}

      /**
       * Return the number of nodes a given path will expand to
       * @return number of nodes, 0 if path does not exist
       */
      virtual int getNumberNodes(const std::string &path = "") const = 0;

      /**
       * Check if a given parameter path exists
       * @return true if path exists, false otherwise
       */
      bool exists(const std::string &path = "") const { return getNumberNodes(path) > 0; }

      /**
       * Convert to bool to simplify checking of existence
       * @return true if path exists, false otherwise
       */
      operator bool() const { return exists(); }


      /**
       * Get vector of GearDirs which point to all the nodes the given path evaluates to
       * @param path Path of the parameters to get
       */
      std::vector<GearDir> getNodes(const std::string &path = "") const;

      /** Return path of the current interface */
      std::string getPath() const { return m_path; }

      /**
       * Get the parameter path as a string.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      virtual std::string getString(const std::string &path = "") const throw(PathEmptyError) = 0;

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
      virtual std::pair<std::string, std::string> getStringWithUnit(const std::string &path) const
      throw(PathEmptyError) = 0;

      /**
       * Get the parameter path as a string.
       * if the parameter is empty or does not exist, the defaultvalue will be
       * returned.
       * @param path path of the parameter to get
       * @param defaultvalue value to return if the path es empty or does not exist
       * @return value of the parameter
       */
      std::string getString(const std::string &path, const std::string &defaultValue) const;

      /**
       * Get the parameter path as a double.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      double getDouble(const std::string &path = "") const throw(PathEmptyError, ConversionError);

      /**
       * Get the parameter path as a double.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getDouble(const std::string &path, double defaultValue) const throw(ConversionError);

      /**
       * Get the parameter path as a int.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      int getInt(const std::string &path = "") const throw(PathEmptyError, ConversionError);

      /**
       * Get the parameter path as a int.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @exception gearbox::ConversionError if the value could not converted to a numerical type
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      int getInt(const std::string &path, int defaultValue) const throw(ConversionError);

      /**
       * Get the parameter path as a bool.
       * @exception gearbox::PathEmptyError if path is empty or does not exist
       * @param path Path of the parameter to get
       * @return value of the parameter
       */
      bool getBool(const std::string &path = "") const throw(PathEmptyError);

      /**
       * Get the parameter path as a bool.
       * If the parameter is empty or does not exist, the defaultValue will be
       * returned.
       * @param path Path of the parameter to get
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      bool getBool(const std::string &path, bool defaultValue) const;

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
       * @param unittype type of the unit, eg. length, angle or energy
       * @return value of the parameter
       */
      double getWithUnit(const std::string &path) const throw(PathEmptyError, ConversionError);

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
       * @param unittype type of the unit, eg. length, angle or energy
       * @param defaultValue Value to return if the Path es empty or does not exist
       * @return value of the parameter
       */
      double getWithUnit(const std::string &path, double defaultValue) const throw(ConversionError);

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
      double getLength(const std::string &path = "") const throw(PathEmptyError, ConversionError) {
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
      double getLength(const std::string &path, double defaultValue) const throw(ConversionError) {
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
      double getAngle(const std::string &path = "") const throw(PathEmptyError, ConversionError) {
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
      double getAngle(const std::string &path, double defaultValue) const throw(ConversionError) {
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
      double getEnergy(const std::string &path = "") const throw(PathEmptyError, ConversionError) {
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
      double getEnergy(const std::string &path, double defaultValue) const throw(ConversionError) {
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
      double getDensity(const std::string &path = "") const throw(PathEmptyError, ConversionError) {
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
      double getDensity(const std::string &path, double defaultValue) const throw(ConversionError) {
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
      double getTime(const std::string &path = "") const throw(PathEmptyError, ConversionError) {
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
      double getTime(const std::string &path, double defaultValue) const throw(ConversionError) {
        return getWithUnit(path, defaultValue);
      }

    protected:

      /** make sure the path really corresponds to an XPath node expression by removing trailing slashes */
      std::string ensureNode(const std::string &path) const ;
      /** make sure the path really corresponds to a path by appending a trailing slash if neccessary */
      std::string ensurePath(const std::string &path) const;
      /** add [index] to the path (after stripping trailing slashes) */
      std::string addIndex(const std::string &path, int index) const;
      /** joind to paths, inserting a slash if neccessary */
      std::string joinPath(const std::string &path, const std::string& subpath) const;

      /** path of the current interface */
      std::string m_path;

    };
  }
}

#endif
