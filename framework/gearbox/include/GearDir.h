/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARDIR_H_
#define GEARDIR_H_

#include <framework/gearbox/GearboxIOAbs.h>
#include <framework/gearbox/GbxExceptions.h>

#include <string>

namespace Belle2 {

  //!  The GearDir class.
  /*!
    Follows the Decorator pattern of [1].

    References:
    [1] Design Patterns: elements of reusable object-oriented software, E. Gamma et al.
        Addison Wesley professional computing series.
  */
  class GearDir : public GearboxIOAbs {

  public:

    //! The GearDir constructor.
    /*!
      \param path The prefix path of the GearDir.
    */
    GearDir(const std::string& path = "");

    //! The GearDir constructor.
    GearDir(GearDir& gearDir, const std::string& path);

    //! The GearGroup destructor.
    virtual ~GearDir();

    //! Closes the connection.
    /*!
      \return True if the connection could be closed.
    */
    virtual bool close();

    //! Returns true if the GearboxIO object is currently connected to a file/database etc.
    /*!
      \return True if the GearboxIO object is currently connected to a file/database etc.
    */
    virtual bool isOpen() const;

    //! Enables the validation of all paths and parameters.
    /*!
      If set to true, a validity check of all paths and parameters is performed
      each time they are accessed. By default the check is turned on.
      Turn it off, in order to speed up the parameter access.

      \param pathCheck If set to true, a check of a path/parameter is performed each time it is accessed.
    */
    void enableParamCheck(bool paramCheck);

    //! Checks if the given path is a valid path.
    /*!
      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.

      \param path The path which should be validated.
      \return True if the path is valid.
    */
    bool isPathValid(const std::string& path) const
    throw(GbxExcIONotConnected);

    //! Checks if a parameter given by the path is available.
    /*!
      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.

      \param path The path to the node which should be checked for existence.
      \return True if the path to the node and the node (parameter) itself exists.
    */
    bool isParamAvailable(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid);

    //! Returns the number of nodes given by the last node in the path.
    /*!
      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.
      GbxExcPathEmptyResult: if the returned result of the path query is empty.
      GbxExcPathResultNotValid: if the returned type of the path query is not supported.

      \param path The path to the nodes which are counted.
      \return The number of nodes.
    */
    int getNumberNodes(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult, GbxExcPathResultNotValid);

    //! Returns a parameter, given by the path, which describes a length.
    /*!
      The path specifies the node, whose value should be returned. Instead of a single
      node, a Path statement can also be used.
      The attribute 'unit' of the node is used to convert the given length parameter to [cm].
      The following units are currently supported: [mum], [mm], [cm], [m], [km]

      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.
      GbxExcParamNotExists: if the parameter does not exist and the parameter check is enabled.
      GbxExcPathEmptyResult: if the returned result of the path query is empty.
      GbxExcPathResultNotValid: if the returned type of the path query is not supported.
      GbxExcStringNumConvFailed: if the conversion of a string to a numerical value failed.

      \param path The path to the node which should be returned.
      \return The value of the length parameter in [cm]. If no unit or an unknown unit is given,
              the value is assumed to be in [cm].
    */
    double getParamLength(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
          GbxExcPathResultNotValid, GbxExcStringNumConvFailed);

    //! Returns a parameter, given by the path, which describes an angle.
    /*!
      The path specifies the node, whose value should be returned. Instead of a single
      node, a Path statement can also be used.
      The attribute 'unit' of the node is used to convert the given angle parameter to [rad].
      The following units are currently supported: [deg], [rad], [mrad]

      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.
      GbxExcParamNotExists: if the parameter does not exist and the parameter check is enabled.
      GbxExcPathEmptyResult: if the returned result of the path query is empty.
      GbxExcPathResultNotValid: if the returned type of the path query is not supported.
      GbxExcStringNumConvFailed: if the conversion of a string to a numerical value failed.

      \param path The path to the node which should be returned.
      \return The value of the length parameter in [rad]. If no unit or an unknown unit is given,
              the value is assumed to be in [rad].
    */
    double getParamAngle(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
          GbxExcPathResultNotValid, GbxExcStringNumConvFailed);

    //! Returns a parameter, given by the path, which describes a general numerical value.
    /*!
      The path specifies the node, which value should be returned. Instead of a single
      node, a Path statement can also be used.

      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.
      GbxExcParamNotExists: if the parameter does not exist and the parameter check is enabled.
      GbxExcPathEmptyResult: if the returned result of the path query is empty.
      GbxExcPathResultNotValid: if the returned type of the path query is not supported.
      GbxExcStringNumConvFailed: if the conversion of a string to a numerical value failed.

      \param path The path to the node which should be returned.
      \return The numerical value.
    */
    double getParamNumValue(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
          GbxExcPathResultNotValid, GbxExcStringNumConvFailed);

    //! Returns a parameter as a string.
    /*!
      The path specifies the node, which value should be returned. Instead of a single
      node, a Path statement can also be used.

      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.
      GbxExcPathNotValid: if the path statement is not valid.
      GbxExcParamNotExists: if the parameter does not exist and the parameter check is enabled.
      GbxExcPathEmptyResult: if the returned result of the path query is empty.
      GbxExcPathResultNotValid: if the returned type of the path query is not supported.

      \param path The path to the node which should be returned.
      \return The string value.
    */
    std::string getParamString(const std::string& path) const
    throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcParamNotExists, GbxExcPathEmptyResult,
          GbxExcPathResultNotValid);

    //! Sets the path which is added automatically as prefix to a given path.
    /*!
      \param path The prefix path which is added in front of a given path.
    */
    void setDirPath(const std::string& path);

    //! Returns the path which is added automatically as prefix to a given path.
    /*!
      \return The prefix path which is added in front of a given path.
    */
    std::string getDirPath() const {return m_dirPath; };

    //! Appends a new path to the current directory path. Useful to go into a deeper level of the hierarchy.
    /*!
      \param path The path which is appended to the end of the current directory path.
    */
    void append(const std::string& path);

    //! Appends the path of the given GearDir to the current directory path. Useful to go into a deeper level of the hierarchy.
    /*!
      \param gearDir The path of the this GearDir is appended to the end of the current directory path.
    */
    void append(GearDir& gearDir);


  protected:

    std::string m_dirPath;     /*!< The prefix path which is added in front of a given path. */


  private:

  };

}

#endif /* GEARDIR_H_ */
