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

#ifndef GEOTOPBELLEII_H_
#define GEOTOPBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoTOPBelleII class.
  /*!
     The creator for the TOP geometry of the Belle II detector.
  */
  class GeoTOPBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoTOPBelleII class.
    GeoTOPBelleII();

    //! The destructor of the GeoTOPBelleII class.
    virtual ~GeoTOPBelleII();

    //! Creates the ROOT Objects for the TOP geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);


  protected:

  private:

  };

}

#endif /* GEOTOPBELLEII_H_ */
