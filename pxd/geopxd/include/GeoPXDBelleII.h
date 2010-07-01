/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geodetector/CreatorBase.h>

#include <string>

#ifndef GEOPXDBELLEII_H_
#define GEOPXDBELLEII_H_

namespace Belle2 {

  class GearDir;

  //!  The GeoPXDBelleII class.
  /*!
     The creator for the PXD geometry of the Belle II detector.
  */
  class GeoPXDBelleII : public CreatorBase {

  public:

    //! Constructor of the GeoPXDBelleII class.
    GeoPXDBelleII();

    //! The destructor of the GeoPXDBelleII class.
    virtual ~GeoPXDBelleII();

    //! Creates the ROOT Objects for the PXD geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(GearDir& content);


  protected:

  private:

  };

}

#endif /* GEOPXDBELLEII_H_ */
