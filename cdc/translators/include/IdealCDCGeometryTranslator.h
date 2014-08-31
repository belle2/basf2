/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IDEALCDCGEOMETRYTRANSLATOR_H_
#define IDEALCDCGEOMETRYTRANSLATOR_H_

#include <cdc/dataobjects/CDCGeometryTranslatorBase.h>
#include <cdc/geometry/CDCGeometryPar.h>

namespace Belle2 {
  namespace CDC {
    /** This class uses the ideal detector geometry for the translation of wire IDs into geometric positions. */
    class IdealCDCGeometryTranslator  : public CDCGeometryTranslatorBase {
    public:
      /** Constructor. */
      IdealCDCGeometryTranslator() {}

      /** Destructor. */
      ~IdealCDCGeometryTranslator() {}

      /** As this is for the ideal geometry, I take simply the one used in the simulation. */
      const TVector3 getWireForwardPosition(const WireID& wireID,
                                            float) {
        return CDCGeometryPar::Instance().wireForwardPosition(wireID);
      }

      /** The following (dummy) is needed to make this class compilable */
      const TVector3 getWireForwardPosition(const WireID& wireID) {
        return CDCGeometryPar::Instance().wireForwardPosition(wireID);
      }

      /** As this is for the ideal geometry, I take simply the one used in the simulation. */
      const TVector3 getWireBackwardPosition(const WireID& wireID,
                                             float) {
        return CDCGeometryPar::Instance().wireBackwardPosition(wireID);
      }

      /** The following (dummy) is needed to make this class compilable */
      const TVector3 getWireBackwardPosition(const WireID& wireID) {
        return CDCGeometryPar::Instance().wireBackwardPosition(wireID);
      }
    };
  }
}
#endif /* IDEALCDCGEOMETRYTRANSLATOR_H */
