/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoShape.h>
#include <TGeoMedium.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>

#include <string>
#include <vector>

#ifndef GEOEKLMBELLEII_H_
#define GEOEKLMBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!   The GeoEKLMBelleII class.
  //!   The creator for the outer EKLM geometry of the Belle II detector.

  class GeoEKLMBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoEKLMBelleII class.
    GeoEKLMBelleII();

    //! The destructor of the GeoEKLMBelleII class.
    virtual ~GeoEKLMBelleII();

    //! Creates the ROOT Objects for the EKLM geometry.
    virtual void create(GearDir& content);

    //! returns top volume for EKLM
    inline TGeoVolume * getTopmostVolume()
    {return volGrpEKLM;}
  protected:

  private:

    //! top volume in the EKLM hierarhy
    TGeoVolumeAssembly* volGrpEKLM ;

  };


}

#endif /* GEOEKLMBelleII_H_ */
