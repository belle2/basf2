/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONPXDExtrapolationModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DATCONPXDExtrapolation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DATCONPXDExtrapolationModule::DATCONPXDExtrapolationModule() : Module()
{
  //Set module properties
  setDescription("DATCONPXDExtrapolationModule: Extrapolates the tracks found by "
                 "the DATCONTrackingModule to the PXD and stores the extrapolated "
                 "hits (called Most Probable Hit, MPH) as PXDIntercept.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("DATCONTracks", m_storeDATCONTracksName,
           "Name of the DATCONTrack StoreArray", string(""));
  addParam("DATCONPXDIntercepts", m_storeDATCONPXDInterceptsName,
           "Name of the DATCONPXDIntercepts StoreArray", string("DATCONPXDIntercepts"));
  addParam("DATCONMPHs", m_storeDATCONMPHName,
           "Name of the DATCONMPH StoreArray", string(""));


}


void DATCONPXDExtrapolationModule::initialize()
{

  storeDATCONTracks.isRequired(m_storeDATCONTracksName);
  m_storeDATCONTracksName = storeDATCONTracks.getName();

  storeDATCONPXDIntercepts.registerInDataStore(m_storeDATCONPXDInterceptsName);
  m_storeDATCONPXDInterceptsName = storeDATCONPXDIntercepts.getName();

  storeDATCONMPHs.registerInDataStore(m_storeDATCONMPHName);
  m_storeDATCONMPHName = storeDATCONMPHs.getName();


}


void
DATCONPXDExtrapolationModule::event()
{
  const double centerZShiftLayer1[2] = {3.68255, -0.88255};    // for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ      // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
  const double centerZShiftLayer2[2] = {5.01455, -1.21455};    // for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ      // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
  const double sensorMinY = -0.36;              // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
  const double sensorMaxY = 0.89;                 // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
  const double shiftY = (sensorMaxY + sensorMinY) / 2.0;
  const double layerRadius[2] = {1.42854, 2.21218};      // ATTENTION: hard coded values from pxd/data/PXD-Components.xml
  const double sensorLength[2] = {4.48, 6.144};
  VxdID sensorID;

  double shiftZ;
  double angleDiff;

  double x = 0., y = 0., z = 0.;
  TVector2 mostProbableHitLocal(0, 0);
  double a, b, cp; /**< helper variables */
  double qualityOfHit;
  double trackRadius, trackCurvsign, trackPhi, trackTheta;
  double sensorPerpRadius;
  double sensorPhi;

  for (auto& track : storeDATCONTracks) {
    trackRadius   = track.getTrackRadius();
    trackPhi      = track.getTrackPhi();
    trackTheta    = track.getTrackTheta();
    trackCurvsign = track.getTrackCurvature();

    trackRadius = trackCurvsign * fabs(trackRadius);

    /* Determine qualityOfHit */
    qualityOfHit = 1.0 / fabs(trackRadius);

    /** Loop over both PXD layer */
    for (int layer = 1; layer <= 2; layer++) {
      /** Loop over all ladders of layer */
      for (int ladder = 1; ladder <= 12; ladder++) {
        /** Loop over both modules of a ladder */
        for (int sensor = 1; sensor <= 2; sensor++) {
          sensorID = VxdID(layer, ladder, sensor);

          sensorPerpRadius = layerRadius[layer - 1];

          x = std::numeric_limits<double>::max();
          y = std::numeric_limits<double>::max();
          z = std::numeric_limits<double>::max();

          if (layer == 1) {
            sensorPhi = M_PI / 4. * (ladder - 1);
            shiftZ = centerZShiftLayer1[sensor - 1];
          } else {
            sensorPhi = M_PI / 6. * (ladder - 1);
            shiftZ = centerZShiftLayer2[sensor - 1];
          }

          angleDiff = trackPhi - sensorPhi;
          if (angleDiff > M_PI) {
            angleDiff -= 2 * M_PI;
          }
          if (angleDiff < -M_PI) {
            angleDiff += 2 * M_PI;
          }

          if (trackCurvsign == +1 /* =negative charge */ && fabs(trackRadius) < 1) {
            a  = sensorPerpRadius + fabs(trackRadius) * sin(angleDiff);
            b  = fabs(trackRadius * trackRadius - a * a);
            cp = trackRadius * cos(angleDiff) + sqrt(b);
            x  = sensorPerpRadius;
            y  = cp;
          } else if (trackCurvsign == -1 /* positive charge */ && fabs(trackRadius) < 1) {
            a  = sensorPerpRadius - fabs(trackRadius) * sin(angleDiff);
            b  = fabs(trackRadius * trackRadius - a * a);
            cp = trackRadius * cos(angleDiff) + sqrt(b);
            x  = sensorPerpRadius;
            y  = cp;
          } else if (trackCurvsign == 0 /* assume straight line */ || fabs(trackRadius) >= 1) {
            x = sensorPerpRadius;
            y = sensorPerpRadius * tan(angleDiff);
          }

          if (trackTheta < (M_PI / 2.0)) {
            z = sqrt(x * x + y * y) / tan(trackTheta);
          } else if (trackTheta > (M_PI / 2.0)) {
            z = sqrt(x * x + y * y) / tan(trackTheta);
          } else {
            z = 0.0;
          }

          if (z >= ((sensorLength[layer - 1] / -2.0) + shiftZ) && z <= ((sensorLength[layer - 1] / 2.0) + shiftZ)) {
            if (y >= sensorMinY && y <= sensorMaxY) {

              double localUPosition = y - shiftY;
              double localVPosition = z - shiftZ;
              mostProbableHitLocal.SetX(localUPosition);
              mostProbableHitLocal.SetY(localVPosition);

              storeDATCONMPHs.appendNew(DATCONMostProbableHit(sensorID, mostProbableHitLocal, qualityOfHit));
              PXDIntercept intercept;
              intercept.setCoorU(mostProbableHitLocal.X());
              intercept.setCoorV(mostProbableHitLocal.Y());
              intercept.setVxdID(sensorID);
              storeDATCONPXDIntercepts.appendNew(intercept);

              break;
            }
          } else {
            continue;
          }
        }

        if (layer == 1 && ladder == 8) {
          ladder = 13; // don't loop over non-existing layer 1 ladders
        }

      }
    }
  }
}
