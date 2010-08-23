/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GBXEXCEPTIONS_H_
#define GBXEXCEPTIONS_H_

#include <string>

namespace Belle2 {

  //===================================================================
  //                           Base classes
  //===================================================================

  //!  The GbxException class.
  /*!
    Base exception class for Gearbox related exceptions.
  */
  class GbxException {

  };


  //-------------------------------------------------------------------
  //!  The GbxExcXPathBase class.
  /*!
    Base exception class for XPath related exceptions.
  */
  class GbxExcXPathBase : public GbxException {

  public:
    //! The GbxExcXPathBase constructor.
    /*!
      \param xpathStatement The XPath statement which resulted in this exception.
    */
    GbxExcXPathBase(std::string xpathStatement) :
        GbxException(), m_xpathStatement(xpathStatement) {};

    //! Returns the XPath statement which resulted in this exception.
    /*!
      \return The XPath statement which resulted in this exception.
    */
    const std::string& getXPathStatement() const {return m_xpathStatement; };

  private:
    std::string m_xpathStatement;
  };


  //-------------------------------------------------------------------
  //!  The GbxExcConversionBase class.
  /*!
    Base exception class for number/string conversion related exceptions.
  */
  class GbxExcConversionBase : public GbxException {

  public:
    //! The GbxExcConversionBase constructor.
    /*!
      \param convString The string which could not be converted.
    */
    GbxExcConversionBase(std::string convString) :
        GbxException(), m_convString(convString) {};

    //! Returns the string which could not be converted.
    /*!
      \return The string which could not be converted.
    */
    const std::string& getConvString() const {return m_convString; };

  private:
    std::string m_convString;
  };


  //===================================================================
  //                    GearboxIO related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The GbxExcIONotAvailable class.
  /*!
    This exception is thrown if no GearboxIO object was created. In order
    to create a GearboxIO object, call one of the open* methods in the
    Gearbox.
  */
  class GbxExcIONotAvailable : public GbxException {

  public:
    //! The GbxExcIONotAvailable constructor.
    GbxExcIONotAvailable() : GbxException() {};
  };

  //-------------------------------------------------------------------
  //!  The GbxExcIONotConnected class.
  /*!
    This exception is thrown if the user tries to retrieve a parameter from
    a storage medium but the GearboxIO is not connected to any storage medium
    at that time.
  */
  class GbxExcIONotConnected : public GbxException {

  public:
    //! The GbxExcIONotConnected constructor.
    GbxExcIONotConnected() : GbxException() {};
  };


  //===================================================================
  //                    XPath related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The GbxExcPathNotValid class.
  /*!
    This exception is thrown if the XPath statement could not be evaluated
    because it contains errors.
  */
  class GbxExcPathNotValid : public GbxExcXPathBase {

  public:
    //! The GbxExcPathNotValid constructor.
    /*!
      \param xpathStatement The XPath statement which resulted in this exception.
    */
    GbxExcPathNotValid(std::string xpathStatement) : GbxExcXPathBase(xpathStatement) {};
  };

  //-------------------------------------------------------------------
  //!  The GbxExcParamNotExists class.
  /*!
    This exception is thrown if the parameter does not exist.
  */
  class GbxExcParamNotExists : public GbxExcXPathBase {

  public:
    //! The GbxExcParamNotExists constructor.
    /*!
      \param xpathStatement The XPath statement which resulted in this exception.
    */
    GbxExcParamNotExists(std::string xpathStatement) : GbxExcXPathBase(xpathStatement) {};
  };

  //-------------------------------------------------------------------
  //!  The GbxExcPathEmptyResult class.
  /*!
    This exception is thrown if the result of the XPath query is empty.
  */
  class GbxExcPathEmptyResult : public GbxExcXPathBase {

  public:
    //! The GbxExcPathEmptyResult constructor.
    /*!
      \param xpathStatement The XPath statement which resulted in this exception.
    */
    GbxExcPathEmptyResult(std::string xpathStatement) : GbxExcXPathBase(xpathStatement) {};
  };

  //-------------------------------------------------------------------
  //!  The GbxExcPathResultNotValid class.
  /*!
    This exception is thrown if the result of the XPath query is either
    not valid or not supported by the method sending the XPath query.
  */
  class GbxExcPathResultNotValid : public GbxExcXPathBase {

  public:
    //! The GbxExcPathResultNotValid constructor.
    /*!
      \param xpathStatement The XPath statement which resulted in this exception.
    */
    GbxExcPathResultNotValid(std::string xpathStatement) : GbxExcXPathBase(xpathStatement) {};
  };


  //===================================================================
  //               String/Num conversion related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The GbxExcStringNumConvFailed class.
  /*!
    This exception is thrown if the conversion of a string to a numerical
    value failed.
  */
  class GbxExcStringNumConvFailed : public GbxExcConversionBase {

  public:
    //! The GbxExcStringNumConvFailed constructor.
    /*!
      \param convString The string which could not be converted to a numerical value.
    */
    GbxExcStringNumConvFailed(std::string convString) : GbxExcConversionBase(convString) {};
  };

}

#endif /* GBXEXCEPTIONS_H_ */
