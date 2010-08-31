/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FWEXCEPTIONS_H_
#define FWEXCEPTIONS_H_

#include <string>

//Each library should have its own definitions of exceptions.

namespace Belle2 {

  //===================================================================
  //                           Base classes
  //===================================================================

  //!  The FwException class.
  /*!
    Base exception class for framework related exceptions.
  */
  class FwException {

  };

  //-------------------------------------------------------------------
  //!  The FwExcMessage class.
  /*!
    Base class for exceptions which store a message string.
  */
  class FwExcMessage : public FwException {

  public:
    //! The FwExcMessage constructor.
    /*!
      \param message The message text of the exception.
    */
    FwExcMessage(const std::string& message) :
        FwException(), m_message(message) {};

    //! Returns the message of the exception.
    /*!
      \return The message of the exception.
    */
    const std::string& getMessage() const {return m_message; };

  private:
    std::string m_message; /*!< The message of the exception. */
  };

  //-------------------------------------------------------------------
  //!  The FwExcModuleTypeBase class.
  /*!
    Base exception class for module type related exceptions.
  */
  class FwExcModuleTypeBase : public FwException {

  public:
    //! The FwExcModuleTypeBase constructor.
    /*!
      \param moduleType The module type which is involved in the exception.
    */
    FwExcModuleTypeBase(std::string moduleType) :
        FwException(), m_moduleType(moduleType) {};

    //! Returns the module type which is involved in the exception.
    /*!
      \return The module type which is involved in the exception.
    */
    const std::string& getModuleType() const {return m_moduleType; };

  private:
    std::string m_moduleType; /*!< The module type which is involved in the exception. */
  };


  //-------------------------------------------------------------------
  //!  The FwExcModuleParamBase class.
  /*!
    Base exception class for module parameter related exceptions.
  */
  class FwExcModuleParamBase : public FwException {

  public:
    //! The FwExcModuleParamBase constructor.
    /*!
      \param paramInfo1 String which stores information about the module parameter.
      \param paramInfo2 String which stores information about the module parameter.
    */
    FwExcModuleParamBase(std::string paramInfo1, std::string paramInfo2) :
        FwException(), m_paramInfo1(paramInfo1), m_paramInfo2(paramInfo2) {};

    //! Returns the parameter information 1.
    /*!
      \return Parameter info 1.
    */
    const std::string& getParamInfo1() const {return m_paramInfo1; };

    //! Returns the parameter information 2.
    /*!
      \return Parameter info 2.
    */
    const std::string& getParamInfo2() const {return m_paramInfo2; };

  private:
    std::string m_paramInfo1; /*!< First parameter information. */
    std::string m_paramInfo2; /*!< Second parameter information. */
  };

  //===================================================================
  //               Python embedding related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The FwExcPythonModuleNotEmbedded class.
  /*!
    This exception is thrown if the basf2 python module could not be embedded.
  */
  class FwExcPythonModuleNotEmbedded : public FwExcMessage {

  public:
    //! The FwExcPythonModuleNotEmbedded constructor.
    FwExcPythonModuleNotEmbedded() : FwExcMessage("Failed to add 'basf2' to the interpreter's builtin modules !") {};
  };


  //===================================================================
  //                    Module related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The FwExcModuleTypeNotFound class.
  /*!
    This exception is thrown if the type of requested module was not found.
  */
  class FwExcModuleTypeNotFound : public FwExcModuleTypeBase {

  public:
    //! The FwExcModuleTypeNotFound constructor.
    /*!
      \param moduleType The module type which could not be found.
    */
    FwExcModuleTypeNotFound(std::string moduleType) : FwExcModuleTypeBase(moduleType) {};
  };


  //-------------------------------------------------------------------
  //!  The FwExcModuleNotCreated class.
  /*!
    This exception is thrown if the requested module could not be created.
  */
  class FwExcModuleNotCreated : public FwExcModuleTypeBase {

  public:
    //! The FwExcModuleNotCreated constructor.
    /*!
      \param moduleType The module type which could not be created.
    */
    FwExcModuleNotCreated(std::string moduleType) : FwExcModuleTypeBase(moduleType) {};
  };


  //-------------------------------------------------------------------
  //!  The FwExcModuleNotRegistered class.
  /*!
    This exception is thrown if the module of the given type could not be registered.
  */
  class FwExcModuleNotRegistered : public FwExcModuleTypeBase {

  public:
    //! The FwExcModuleNotRegistered constructor.
    /*!
      \param moduleType The module type which could not be registered.
    */
    FwExcModuleNotRegistered(std::string moduleType) : FwExcModuleTypeBase(moduleType) {};
  };


  //-------------------------------------------------------------------
  //!  The FwExcModuleParameterNotFound class.
  /*!
    This exception is thrown if the requested module parameter was not found.
  */
  class FwExcModuleParameterNotFound : public FwExcModuleParamBase {

  public:
    //! The FwExcModuleParameterNotFound constructor.
    /*!
      \param paramName The module parameter name which could not be found.
    */
    FwExcModuleParameterNotFound(std::string paramName) :
        FwExcModuleParamBase(paramName, "") {};
  };


  //-------------------------------------------------------------------
  //!  The FwExcModuleParameterType class.
  /*!
    This exception is thrown if the requested module parameter has the wrong type.
  */
  class FwExcModuleParameterType : public FwExcModuleParamBase {

  public:
    //! The FwExcModuleParameterType constructor.
    /*!
      \param paramTypeName The module parameter type name.
      \param requestedTypeName The requested module parameter type name.
    */
    FwExcModuleParameterType(std::string paramTypeName, std::string requestedTypeName) :
        FwExcModuleParamBase(paramTypeName, requestedTypeName) {};
  };


  //===================================================================
  //                      Path related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The FwExcPathNotCreated class.
  /*!
    This exception is thrown if the requested path could not be created.
  */
  class FwExcPathNotCreated : public FwException {

  public:
    //! The FwExcPathNotCreated constructor.
    FwExcPathNotCreated() : FwException() {};
  };

  //-------------------------------------------------------------------
  //!  The FwExcPathNotAdded class.
  /*!
    This exception is thrown if the requested path could not be added.
  */
  class FwExcPathNotAdded : public FwException {

  public:
    //! The FwExcPathNotAdded constructor.
    FwExcPathNotAdded() : FwException() {};
  };


} // end namespace Belle2

#endif /* FWEXCEPTIONS_H_ */
