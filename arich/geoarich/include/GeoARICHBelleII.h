/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#include <string>

#ifndef GEOARICHBELLEII_H
#define GEOARICHBELLEII_H

namespace Belle2 {

  class GearDir;

  //!  The GeoARICHBelleII class.
  /*!
     The creator for the ARICH geometry of the Belle II detector.
  */
  class GeoARICHBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoARICHBelleII class.
    GeoARICHBelleII();

    //! The destructor of the GeoARICHBelleII class.
    virtual ~GeoARICHBelleII();

    //! Creates the ROOT Objects for the ARICH geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);


  protected:

  private:

  };

}

#endif /* GEOARICHBELLEII_H */
