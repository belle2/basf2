/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "cdc/utilities/ClosestApproach.h"

namespace Belle2 {
  namespace CDC {

    double ClosestApproach(const B2Vector3D& bwp, const B2Vector3D& fwp, const B2Vector3D& posIn, const B2Vector3D& posOut,
                           B2Vector3D& hitPosition, B2Vector3D& wirePosition)
    {
      //----------------------------------------------------------
      /* For two lines r=r1+t1.v1 & r=r2+t2.v2
      the closest approach is d=|(r2-r1).(v1 x v2)|/|v1 x v2|
      the point where closest approach are
      t1=(v1 x v2).[(r2-r1) x v2]/[(v1 x v2).(v1 x v2)]
      t2=(v1 x v2).[(r2-r1) x v1]/[(v1 x v2).(v1 x v2)]
      if v1 x v2=0 means two lines are parallel
      d=|(r2-r1) x v1|/|v1|
      */

      double t2, distance;

      //--------------------------
      // Get wirepoint @ endplate
      //--------------------------
      /*  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();
      B2Vector3D tfwp = cdcgp.wireForwardPosition(layerId, cellId);
      G4ThreeVector fwp(tfwp.X(), tfwp.Y(), tfwp.Z());
      B2Vector3D tbwp = cdcgp.wireBackwardPosition(layerId, cellId);
      G4ThreeVector bwp(tbwp.X(), tbwp.Y(), tbwp.Z());
      */

      B2Vector3D wireLine = fwp - bwp;
      B2Vector3D hitLine = posOut - posIn;

      B2Vector3D hitXwire = hitLine.Cross(wireLine);
      B2Vector3D wire2hit = fwp - posOut;

      //----------------------------------------------------------------
      // Hitposition is the position on hit line where closest approach
      // of two lines, but it may out the area from posIn to posOut
      //----------------------------------------------------------------
      if (hitXwire.Mag() == 0) {
        distance = wireLine.Cross(wire2hit).Mag() / wireLine.Mag();
        hitPosition = posIn;
        t2 = (posIn - fwp).Dot(wireLine) / wireLine.Mag2();
      } else {
        double t1 = hitXwire.Dot(wire2hit.Cross(wireLine)) / hitXwire.Mag2();
        hitPosition = posOut + t1 * hitLine;
        t2 = hitXwire.Dot(wire2hit.Cross(hitLine)) / hitXwire.Mag2();

        //should not constrain hitPosition inside the cell
        //        double dInOut = (posOut - posIn).Mag();
        //        double dHitIn = (hitPosition - posIn).Mag();
        //        double dHitOut = (hitPosition - posOut).Mag();
        //        if (dHitIn <= dInOut && dHitOut <= dInOut) { //Between point in & out
        distance = fabs(wire2hit.Dot(hitXwire) / hitXwire.Mag());
        /*
              } else if (dHitOut > dHitIn) { // out posIn
                distance = wireLine.Cross(posIn - fwp).Mag() / wireLine.Mag();
                hitPosition = posIn;
                t2 = (posIn - fwp).Dot(wireLine) / wireLine.Mag2();
              } else { // out posOut
                distance = wireLine.Cross(posOut - fwp).Mag() / wireLine.Mag();
                hitPosition = posOut;
                t2 = (posOut - fwp).Dot(wireLine) / wireLine.Mag2();
              }
        */
      }

      wirePosition = fwp + t2 * wireLine;
      return distance;
    }

  }
}
