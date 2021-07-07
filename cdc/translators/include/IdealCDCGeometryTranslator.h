/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
                                            float) override
      {
        return CDCGeometryPar::Instance().wireForwardPosition(wireID);
      }

      /** The following (dummy) is needed to make this class compilable */
      const TVector3 getWireForwardPosition(const WireID& wireID) override
      {
        return CDCGeometryPar::Instance().wireForwardPosition(wireID);
      }

      /** As this is for the ideal geometry, I take simply the one used in the simulation. */
      const TVector3 getWireBackwardPosition(const WireID& wireID,
                                             float) override
      {
        return CDCGeometryPar::Instance().wireBackwardPosition(wireID);
      }

      /** The following (dummy) is needed to make this class compilable */
      const TVector3 getWireBackwardPosition(const WireID& wireID) override
      {
        return CDCGeometryPar::Instance().wireBackwardPosition(wireID);
      }
    };
  }
}
