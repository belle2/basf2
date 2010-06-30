/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GDETEXCEPTIONS_H_
#define GDETEXCEPTIONS_H_

#include <string>

namespace Belle2 {

  //===================================================================
  //                           Base classes
  //===================================================================

  //!  The GDetException class.
  /*!
    Base exception class for GeoDetector related exceptions.
  */
  class GDetException {

  };

  //-------------------------------------------------------------------
  //!  The GDetExcCreatorBase class.
  /*!
    Base exception class for creator related exceptions.
  */
  class GDetExcCreatorBase : public GDetException {

  public:
    //! The GDetExcCreatorBase constructor.
    /*!
      \param name The name of the creator which triggered the exception.
    */
    GDetExcCreatorBase(std::string name) :
        GDetException(), m_name(name) {};

    //! Returns the name of the creator which triggered the exception.
    /*!
      \return The name of the creator which triggered the exception.
    */
    const std::string& getName() const {return m_name; };

  private:
    std::string m_name;
  };


  //===================================================================
  //                   Creator related exceptions
  //===================================================================

  //-------------------------------------------------------------------
  //!  The GDetExcCreatorNameEmpty class.
  /*!
    This exception is thrown if the name of a creator is empty.
  */
  class GDetExcCreatorNameEmpty : public GDetException {

  public:
    //! The GDetExcCreatorNameEmpty constructor.
    GDetExcCreatorNameEmpty() : GDetException() {};
  };

  //-------------------------------------------------------------------
  //!  The GDetExcCreatorExists class.
  /*!
    This exception is thrown if a creator is registered to the CreatorManager
    carrying a name which was already registered.
    value failed.
  */
  class GDetExcCreatorExists : public GDetExcCreatorBase {

  public:
    //! The GDetExcCreatorExists constructor.
    /*!
      \param name The name of the creator which already exists.
    */
    GDetExcCreatorExists(std::string name) : GDetExcCreatorBase(name) {};
  };

  //-------------------------------------------------------------------
  //!  The GDetExcCreatorNotExists class.
  /*!
    This exception is thrown if the name of a creator does not exist.
  */
  class GDetExcCreatorNotExists : public GDetExcCreatorBase {

  public:
    //! The GDetExcCreatorNotExists constructor.
    /*!
      \param name The name of the creator which does not exist.
    */
    GDetExcCreatorNotExists(std::string name) : GDetExcCreatorBase(name) {};
  };

}

#endif /* GDETEXCEPTIONS_H_ */
