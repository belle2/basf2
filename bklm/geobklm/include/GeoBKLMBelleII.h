/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOBKLMBELLEII_H
#define GEOBKLMBELLEII_H

#include <geometry/geodetector/CreatorBase.h>

#include <TGeoVolume.h>

namespace Belle2 {

  class GearDir;

  //!   This class creates the BKLM geometry of the Belle II detector

  class GeoBKLMBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoBKLMBelleII class
    GeoBKLMBelleII();

    //! The destructor of the GeoBKLMBelleII class
    virtual ~GeoBKLMBelleII();

    //! Creates the ROOT Objects for the BKLM geometry
    virtual void create(GearDir&);

    //! returns top volume for BKLM
    inline TGeoVolume * getTopmostVolume()
    { return volGrpBKLM; }

  protected:

  private:

    //! top volume in the BKLM hierarhy
    TGeoVolumeAssembly* volGrpBKLM;

  };


}  // end of namespace Belle2

#endif // GEOBKLMBelleII_H
