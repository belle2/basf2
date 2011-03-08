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

#ifndef GEOQCSBELLEII_H_
#define GEOQCSBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoQCsBelleII class.
  /*!
     The creator for the the Belle II QCs.
  */
  class GeoQCsBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoQCsBelleII class.
    GeoQCsBelleII();

    //! The destructor of the GeoQCsBelleII class.
    virtual ~GeoQCsBelleII();

    //! Creates the ROOT Objects for the QCs geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

  };

}

#endif /* GEOQCSBELLEII_H_ */
