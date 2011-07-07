/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
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

#ifndef GEOBKLMBELLEII_H_
#define GEOBKLMBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!   The GeoBKLMBelleII class.
  //!   The creator for the outer BKLM geometry of the Belle II detector.

  class GeoBKLMBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoBKLMBelleII class.
    GeoBKLMBelleII();

    //! The destructor of the GeoBKLMBelleII class.
    virtual ~GeoBKLMBelleII();

    //! Creates the ROOT Objects for the BKLM geometry.
    virtual void create(GearDir& content);

    //! returns top volume for BKLM
    inline TGeoVolume * getTopmostVolume()
    {return volGrpBKLM;}
  protected:

  private:

    //! top volume in the BKLM hierarhy
    TGeoVolumeAssembly* volGrpBKLM ;

  };


}

#endif /* GEOBKLMBelleII_H_ */
