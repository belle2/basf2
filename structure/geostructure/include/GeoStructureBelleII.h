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
#include <structure/geostructure/StructureEndcap.h>
#include <structure/geostructure/StructurePoletip.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#include "TVector3.h"

#ifndef GEOSTRUCTUREBELLEII_H_
#define GEOSTRUCTUREBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoStructureBelleII class.
  /*!
     The creator for the structure of the Belle II detector.
  */
  class GeoStructureBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoStructureBelleII class.
    GeoStructureBelleII();

    //! The destructor of the GeoIRBelleIISymm class.
    virtual ~GeoStructureBelleII();

    //! Creates the ROOT Objects for the structure geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);

  protected:

  private:

    //! To get the geometrical paramters for the Structure Endcap part
    StructureEndcap EndcapMgr;

    //! To get the geometrical paramters for the Structure Poletip part
    StructurePoletip PoletipMgr;

  };

}

#endif /* GEOSTRUCTUREBELLEII_H_ */
