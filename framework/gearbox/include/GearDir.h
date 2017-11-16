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

#include <framework/gearbox/Interface.h>
#include <framework/gearbox/Gearbox.h>

namespace Belle2 {

  /**
   * GearDir is the basic class used for accessing the parameter store.
   *
   * To access parameters, just create a GearDir with the correct path and use the getter functions defined by
   * gearbox::Interface to retrieve any parameter in the given path
   *
   * GearDir parameters("/test/");
   * int foo = parameters.getInt("bar");
   *
   * will return an integer value of the parameter /test/bar
   */
  class GearDir: public gearbox::Interface {
  public:
    /** Create a GearDir starting at a given base path and appending some subpath and optionally an index */
    GearDir(const gearbox::Interface& start, const std::string& path, int index = 0)
    {
      m_path = ensurePath(start.getPath()) + path;
      if (index > 0) m_path = addIndex(m_path, index);
    }
    /** Create a GearDir with an absolute path and optionally appending an index */
    GearDir(const std::string& path = "", int index = 0)
    {
      m_path = path;
      if (index > 0) m_path = addIndex(path, index);
    }
    /** Copy constructor */
    GearDir(const GearDir& other): gearbox::Interface(other)
    {
      m_path = other.m_path;
    }

    /** Append something to the current path, modifying the GearDir in place */
    void append(const std::string& path) { m_path += path; }

    /**
     * Return the number of nodes a given path will expand to
     * @return number of nodes, 0 if path does not exist
     */
    virtual int getNumberNodes(const std::string& path = "") const
    {
      return Gearbox::getInstance().getNumberNodes(ensurePath(m_path) + path);
    }

    /**
     * Get the parameter path as a string.
     * @exception gearbox::PathEmptyError if path is empty or does not exist
     * @param path Path of the parameter to get
     * @return value of the parameter
     */
    virtual std::string getString(const std::string& path = "") const noexcept(false)
    {
      return Gearbox::getInstance().getString(ensurePath(m_path) + path);
    }

    /**
     * Get the parameter path as a string.
     * if the parameter is empty or does not exist, the defaultvalue will be
     * returned.
     * @param path path of the parameter to get
     * @param defaultValue value to return if the path es empty or does not exist
     * @return value of the parameter
     */
    std::string getString(const std::string& path, const std::string& defaultValue) const
    {
      return gearbox::Interface::getString(path, defaultValue);
    }

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
    virtual std::pair<std::string, std::string> getStringWithUnit(const std::string& path = "") const noexcept(false)
    {
      return Gearbox::getInstance().getStringWithUnit(ensurePath(m_path) + path);
    }

    /**
     * Get the parameter path as a TObject
     * @exception gearbox::PathEmptyError if path is empty or does not exist
     * @exception gearbox::TObjectConversionError if the value could not be deserialized
     * @param path Path of the parameter to get
     * @return pointer to object, owned and managed by gearbox. Object will
     *         be deleted once it is no longer valid (e.g. after the current
     *         run if it belongs to this run)
     */
    virtual const TObject* getTObject(const std::string& path) const noexcept(false)
    {
      return Gearbox::getInstance().getTObject(ensurePath(m_path) + path);
    }
  };

}
