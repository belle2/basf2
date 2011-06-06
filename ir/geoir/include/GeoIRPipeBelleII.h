/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Clement Ng, Andreas Moll, Hiroshi Nakano                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#ifndef GEOIRPIPEBELLEII_H_
#define GEOIRPIPEBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoIRPipeBelleII class.
  /*!
     The creator for the the Belle II IRPipe.
  */
  class GeoIRPipeBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoIRPipeBelleII class.
    GeoIRPipeBelleII();

    //! The destructor of the GeoIRPipeBelleII class.
    virtual ~GeoIRPipeBelleII();

    //! Creates the ROOT Objects for the IRPipe geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

  };

}

#endif /* GEOIRPIPEBELLEII_H_ */
