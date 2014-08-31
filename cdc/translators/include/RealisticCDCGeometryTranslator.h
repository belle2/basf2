/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REALISTICCDCGEOMETRYTRANSLATOR_H_
#define REALISTICCDCGEOMETRYTRANSLATOR_H_

#include <cdc/dataobjects/CDCGeometryTranslatorBase.h>
#include <cdc/geometry/CDCGeometryPar.h>

namespace Belle2 {
  namespace CDC {
    /** This class uses the realistic detector geometry (the one after alignment procedure) for the translation of wire IDs into geometric positions. */
    class RealisticCDCGeometryTranslator  : public CDCGeometryTranslatorBase {
    public:
      /** Constructor, with the additional parameter to control wire sag */
      RealisticCDCGeometryTranslator(bool wireSag = false) : m_wireSag(wireSag) {}

      /** Destructor. */
      ~RealisticCDCGeometryTranslator() {}

      /** Get wire position at forward end. */
      const TVector3 getWireForwardPosition(const WireID& wireID) {
        return CDCGeometryPar::Instance().wireForwardPosition(wireID, CDCGeometryPar::c_Aligned);
      }

      /** Get virtual wire position at forward end, corresponding to tangent line to wire at input z-position. */
      const TVector3 getWireForwardPosition(const WireID& wireID, float z) {
        TVector3 wPos = (m_wireSag) ?
                        CDCGeometryPar::Instance().wireForwardPosition(wireID, z, CDCGeometryPar::c_Aligned) :
                        CDCGeometryPar::Instance().wireForwardPosition(wireID,    CDCGeometryPar::c_Aligned);
        return wPos;
      }

      /** Get wire position at backward end. */
      const TVector3 getWireBackwardPosition(const WireID& wireID) {
        return CDCGeometryPar::Instance().wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
      }

      /** Get virtual wire position at backward end, corresponding to tangent line to wire at input z-position. */
      const TVector3 getWireBackwardPosition(const WireID& wireID, float z) {
        TVector3 wPos = m_wireSag ?
                        CDCGeometryPar::Instance().wireBackwardPosition(wireID, z, CDCGeometryPar::c_Aligned) :
                        CDCGeometryPar::Instance().wireBackwardPosition(wireID,    CDCGeometryPar::c_Aligned);
        return wPos;
      }

    private:
      /**
       * Flag to activate the sense wire sag effect.
       * true: activated; false: no effect.
       *
       */
      bool m_wireSag;
    };
  }
}
#endif /* REALISTICCDCGEOMETRYTRANSLATOR_H */
