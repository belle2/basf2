/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARBOX_H_
#define GEARBOX_H_

#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>

#include <framework/gearbox/GearboxIOAbs.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GbxExceptions.h>

#include <string>

namespace Belle2 {

  //!  The Gearbox class.
  /*!
    The Gearbox class provides an interface to access the parameters
    describing the Belle II detector geometry. It uses the GearboxIOAbs
    interface to handle the user request for parameters and therefore allows
    to be connected to a wide variety of parameter sources (e.g. XML, databases).
    It is designed as a singleton.
  */
  class Gearbox {

  public:

    //! The content return types
    enum EGearboxContentType {
      c_GbxGlobal,       /*!< The content section of the global parameters. */
      c_GbxMaterial,     /*!< The content section of the material sets. */
      c_GbxSubdetectors /*!< The content section of the subdetector parameters. */
    };

    //! Static method to get a reference to the Gearbox instance.
    /*!
      \return A reference to an instance of this class.
    */
    static Gearbox& Instance();

    //! Connects the Gearbox to a GearboxIO object.
    /*!
      The GearboxIO object handles the parameters requests from the user.
      The connection to the parameter source is not opened by the gearbox,
      it has to be done externally.
      The Gearbox takes ownership of the GearboxIO object.

      \param gearboxIO Pointer to a GearboxIOAbs object. The Gearbox takes ownership of the GearboxIO object.
    */
    void connect(GearboxIOAbs* gearboxIO);

    //! Returns a reference to the current GearboxIO object.
    /*!
      If the GearboxIO object is not defined or not connected to a detector
      parameter source an exception of type GbxExcIONotConnected is thrown.

      \return A reference to the current GearboxIO object.
    */
    GearboxIOAbs& getGearboxIO() const throw(GbxExcIONotConnected);

    //! Returns a GearDir pointing to the content section of the specified parameter set.
    /*!
      \param paramSetType The name of the parameter set type.
      \return A GearDir object pointing to the content part of the specified parameter set.
    */
    GearDir getContent(const std::string& paramSetType, EGearboxContentType contentType = c_GbxSubdetectors)
    throw(GbxExcIONotConnected);

    //! Returns the path to the content section of the given global parameter set.
    /*!
      \param globalParam The name of the global parameter set.
      \return The full path to the content section of the global parameter set.
    */
    std::string getGlobalParamPath(const std::string& globalParam) const;

    //! Returns the path to the content section of the given material set.
    /*!
      \param globalParam The name of the material set.
      \return The full path to the content section of the material set.
    */
    std::string getMaterialParamPath(const std::string& materialParam) const;

    //! Returns the path to the content section of the given subdetector.
    /*!
      \param subdetector The name of the subdetector.
      \return The full path to the content section of the subdetector.
    */
    std::string getSubdetectorPath(const std::string& subdetector) const;

    //! Enables the validation of all paths and parameters.
    /*!
      If set to true, a validity check of all paths and parameters is performed
      each time they are accessed. By default the check is turned on.
      Turn it off, in order to speed up the parameter access.

      Different types of exceptions can be thrown:
      GbxExcIONotConnected: if the GearboxIO is not connected to a storage medium.

      \param pathCheck If set to true, a check of a path/parameter is performed each time it is accessed.
    */
    void enableParamCheck(bool paramCheck)
    throw(GbxExcIONotConnected);

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


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    //! Exposes methods of the Gearbox class to Python.
    static void exposePythonAPI();

  protected:


  private:

    GearboxIOAbs* m_gearboxIO; /*!< Pointer to the current Gearbox IO object. */

    //! Closes the connection to the current GearboxIO object.
    /*!
      \return True if the connection could be closed.
    */
    bool close();

    //! The constructor is hidden to avoid that someone creates an instance of this class.
    Gearbox();

    //! Disable/Hide the copy constructor
    Gearbox(const Gearbox&);

    //! Disable/Hide the copy assignment operator
    Gearbox& operator=(const Gearbox&);

    //! The Gearbox destructor
    ~Gearbox();

    static Gearbox* m_instance; /*!< Pointer that saves the instance of this class. */

    //! Destroyer class to delete the instance of the Gearbox class when the program terminates.
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (Gearbox::m_instance != NULL) delete Gearbox::m_instance;
      }
    };
    friend class SingletonDestroyer;

  };

}

#endif /* GEARBOX_H_ */
