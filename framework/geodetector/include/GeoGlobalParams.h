/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geodetector/CreatorBase.h>

#ifndef GEOGLOBALPARAMS_H_
#define GEOGLOBALPARAMS_H_


namespace Belle2 {

  class GearDir;

  //!  The GeoGlobalParams class.
  /*!
     The creator for the global parameters of the Belle II detector.
  */
  class GeoGlobalParams : public CreatorBase {

  public:

    //! Constructor of the GeoGlobalParams class.
    GeoGlobalParams();

    //! The destructor of the GeoGlobalParams class.
    virtual ~GeoGlobalParams();

    //! Creates the global ROOT objects and prepares everything for other creators.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);


  protected:

  private:

  };


}

#endif /* GEOGLOBALPARAMS_H_ */
