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

#ifndef GEOBEAMPIPEBELLEII_H_
#define GEOBEAMPIPEBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoBeampipeBelleII class.
  /*!
     The creator for the beampipe geometry of the Belle II detector.
  */
  class GeoBeampipeBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoBeampipeBelleII class.
    GeoBeampipeBelleII();

    //! The destructor of the GeoBeampipeBelleII class.
    virtual ~GeoBeampipeBelleII();

    //! Creates the ROOT Objects for the beampipe geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);


  protected:

  private:

  };


}

#endif /* GEOBEAMPIPEBELLEII_H_ */
