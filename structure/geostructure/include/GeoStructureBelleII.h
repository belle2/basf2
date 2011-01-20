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
#include <structure/geostructure/StructureBarrel.h>
#include <structure/geostructure/StructureEndcap.h>
#include <structure/geostructure/StructurePoletip.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>

#include <string>
#include <vector>

#include "TVector3.h"

#ifndef GeoStructureBelleII_H_
#define GeoStructureBelleII_H_

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

    void readParameters(GearDir& content);

    const TVector3 endcapModulePos(int, int) const;

    double THICK_EIRON;
    double THICK_EIRON_MEAS;
    double THICK_ESLOT;
    double THICK_ESLOT_MEAS;

    double THICK_EMOD;
    double RMIN_EMOD;
    double RMAX_EMOD;
    double RMIN_ESLOT;
    double RMAX_ESLOT;

    double RSHIFT_ESLOT;
    double RMAX_GLASS;

    //static const char Bironname;
    //TGeoVolume* strBironname[15];

    //static const char Bmodname;
    //TGeoVolume* strBmodname[15];

  protected:

  private:

    StructureBarrel BarrelMgr;
    StructureEndcap EndcapMgr;
    StructurePoletip PoletipMgr;

  };

}

#endif /* GeoStructureBelleII_H_ */
