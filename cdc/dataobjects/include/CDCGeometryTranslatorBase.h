/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCGEOMETRYTRANSLATORBASE_H
#define CDCGEOMETRYTRANSLATORBASE_H

#include <cdc/dataobjects/WireID.h>

#include <TVector3.h>

namespace Belle2 {
  namespace CDC {
    /** Abstract Base class for the geometry translator. */
    class CDCGeometryTranslatorBase {
    public:
      /** Constructor. */
      CDCGeometryTranslatorBase() {}

      /** Destructor. */
      virtual ~CDCGeometryTranslatorBase() {}

      /** End of wire in the more forward position.
       */
      virtual const TVector3 getWireForwardPosition(const WireID& wireID) = 0;

      /** Beginning of wire in the more backward position. */
      virtual const TVector3 getWireBackwardPosition(const WireID& wireID) = 0;

      /** End of imaginary wire in the more forward position.
       *
       *  The GFWireHitPolicy needs the endpoints of the wires and assumes a straight line
       *  between those endpoints. Around these lines circular drift-isochrones are assumed.
       *  During the tracking the z-position of the actual hit is known quite well, which means,
       *  that individual corrections due to wire sag, etc. can be applied by moving the endpoints accordingly.
       */
      virtual const TVector3 getWireForwardPosition(const WireID& wireID,
                                                    //float z = 0) = 0;
                                                    float z) = 0;

      /** Beginning of imaginary wire in the more backward position. */
      virtual const TVector3 getWireBackwardPosition(const WireID& wireID,
                                                     //float z = 0) = 0;
                                                     float z) = 0;
    };
  }
}
#endif /* CDCGEOMETRYTRANSLATORBASE_H */
