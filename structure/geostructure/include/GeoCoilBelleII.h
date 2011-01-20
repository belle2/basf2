/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#ifndef GEOCOILBELLEII_H_
#define GEOCOILBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoCoilBelleII class.
  /*!
     The creator for the the Belle II coil.
  */
  class GeoCoilBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoCoilBelleII class.
    GeoCoilBelleII();

    //! The destructor of the GeoCoilBelleII class.
    virtual ~GeoCoilBelleII();

    //! Creates the ROOT Objects for the coil geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

  };

}

#endif /* GEOCOILBELLEII_H_ */
