/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <geometry/GeometryManager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>
#include <time.h>

#include <TH1F.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/*
 * Wrapper around the extrapolation structure. Choose between
 * different types of extrapolation algorithm.
 */
void
SVDHoughTrackingModule::pxdExtrapolation()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);

  if (!storeExtrapolatedHits.isValid()) {
    storeExtrapolatedHits.create();
  } else {
    storeExtrapolatedHits.getPtr()->Clear();
  }

  if (m_usePhiExtrapolation) {
//    pxdExtrapolationPhi();
    //pxdTestExtrapolationPhi();
//    analyseExtrapolatedPhi();
  } else if (m_useThetaExtrapolation) {
//    pxdExtrapolationTheta();
  } else {
    //pxdSingleExtrapolation();
    pxdExtrapolationFullCW();

    /* Analyse extrapolated hits */
    analyseExtrapolatedFull();

    /* create ROIs */
    if (m_createROI) {
      createROI();
      /* Create a PXD hit map with ROIs for the last event */
      if (m_createPXDMap) {
        createPXDMap();
      }
    }

    analyseROI();
  }
}

/*
 * This function is for testing what influence the not precise angle
 * has on the reconstruction.
 */
void
SVDHoughTrackingModule::pxdTestExtrapolationPhi()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  double pxdShift[] = {0.35, 0.35};
  double x, y;
  double mph_x, mph_y;
  double a, b, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  int nTracks;
  VxdID sensor;

  B2DEBUG(200, "PXD Phi Extrapolation: ");

  /*
   * For testing variations in the reco track parameters only!
   */
  nTracks = 1;
  for (int i = 0; i < nTracks; ++i) {
    //SVDHoughTrack* htrack = storeHoughTrack[0];
    SVDHoughTrack* htrack = storeHoughTrack[i];
    double r = -170.667 * 1.0;
    double tphi = 0.64577 - (M_PI / 2.0);

    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
      BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
        sensor = ladder;
        sensor.setSensorNumber(1);
        const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
        transfMatrix = currentSensor->getTransformation();
        rotMatrix = transfMatrix.GetRotationMatrix();
        transMatrix = transfMatrix.GetTranslation();
        double width = currentSensor->getWidth();
        double length = currentSensor->getLength();
        double shift = pxdShift[sensor.getLayerNumber() - 1];
        double z_shift = 0.0;
        double phi = acos(rotMatrix[2]);
        double radius = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));

        if (asin(rotMatrix[0]) > 0.01) {
          phi = 2 * M_PI - phi;
        }

        if (phi > M_PI) {
          phi = phi - M_PI * 2 ;
        }

        angle = tphi - phi;
        a = radius * cos(angle) - fabs(r) * (pow(fabs(sin(angle)), 2.0));
        B2DEBUG(200, "  -> a: " << a << " angle: " << angle << " Phi: " << phi
                << " tphi-phi: " << fabs((htrack->getTrackPhi() - phi)));
        if ((pow(fabs(a), 2.0) - pow(radius, 2.0)) >= 0.0 &&
            fabs((htrack->getTrackPhi() - phi)) < (M_PI / 2.0)) {
          x = a + sqrt(pow(fabs(a), 2.0) - pow(radius, 2.0));
          b = -1 * pow(fabs(x), 2.0) + (2 * x * r);
          B2DEBUG(200, "    -> x: " << x << " b: " << b);

          if (b >= 0.0) {
            y = x * sin(angle) + sqrt(b) * cos(angle);

            B2DEBUG(200, "    --> y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
            if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
              B2DEBUG(200, "  Found in PXD Sensor ID: " << sensor << "\tPhi: "
                      << phi / Unit::deg << " Radius: " << radius << " Shift: " << z_shift
                      << " Width: " << width << " Length: " << length);
              B2DEBUG(200, "      Candidate is: " << radius << ", " << y);
              mph_x = radius * cos(phi) - y * sin(phi);
              mph_y = radius * sin(phi) + y * cos(phi);
              B2DEBUG(200, "       -> Transformed: " << mph_x << ", " << mph_y);
              mph_pos.SetXYZ(mph_x, mph_y, 0.0);

              storeExtrapolatedHits.appendNew(SVDHoughCluster(0, mph_pos, sensor));
            }
          }
        }

        /* Debug output for transformation- and translation Matrix */
        B2DEBUG(2000, "  Transformation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[0]
                << "\t" << boost::format("%2.3f") % rotMatrix[1]
                << "\t" << boost::format("%2.3f") % rotMatrix[2] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[3]
                << "\t" << boost::format("%2.3f") % rotMatrix[4]
                << "\t" << boost::format("%2.3f") % rotMatrix[5] << endl
                << "\t" << boost::format("%2.3f") % rotMatrix[6]
                << "\t" << boost::format("%2.3f") % rotMatrix[7]
                << "\t" << boost::format("%2.3f") % rotMatrix[8] << endl);
        B2DEBUG(2000, "  Translation Matrix: " << endl
                << "\t" << boost::format("%2.3f") % transMatrix[0]
                << "\t" << boost::format("%2.3f") % transMatrix[1]
                << "\t" << boost::format("%2.3f") % transMatrix[2] << endl);
      }
    }
  }
}

/*
 * Full Extrapolation to PXD layers.
 */
void
SVDHoughTrackingModule::pxdExtrapolationFullCW()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  //double pxdZ1Shift[] = {4.8013, -2.0013};    // old values of Michael
  //double pxdZ2Shift[] = {6.1513, -2.3513};    // old values of Michael
  //double pxdZ1Shift[] = {1.42225, -3.10255};    // for use of mph_z > z_shift && mph_z < z_shift+length
  //double pxdZ2Shift[] = {1.92255, -4.26505};    // for use of mph_z > z_shift && mph_z < z_shift+length
  double pxdZ1Shift[] = {3.66255, -0.86255};    // for use of mhp_z > (lengh/-2)+z_shift &&  mhp_z < (lengh/2)+z_shift
  double pxdZ2Shift[] = {4.99455, -1.19455};    // for use of mhp_z > (lengh/-2)+z_shift &&  mhp_z < (lengh/2)+z_shift
  //double minY[] = { -0.42, -0.42};
  //double maxY[] = {0.83, 0.83};
  double minY[] = { -0.36, -0.36}; //{ -0.42, -0.42};
  double maxY[] = {0.90, 0.90}; //{0.83, 0.83};
  double z_shift;
  /** x and y are projected coordinates in simplified x-y-plane for extrapolation.
   * They are NOT global coordinates, and also not local coordinates
   */
  double x = 0., y = 0.;
  double mph_x, mph_y, mph_z;
  double a, b, cm, cp, angle;
  TVector3 mph_pos;
  TGeoHMatrix transfMatrix;
  double* rotMatrix, *transMatrix;
  unsigned int qOH; /* quality of hit */
  int nTracks, idx;
  VxdID sensorID;
  bool found;

  B2DEBUG(200, "PXD Full Extrapolation: ");

  extrapolatedHits.clear(); /* Clear last extrapolated hits */

  nTracks = storeHoughTrack.getEntries();
  idx = 0;


  for (int i = 0; i < nTracks; ++i) {
    SVDHoughTrack* htrack = storeHoughTrack[i];
    double rt = htrack->getTrackR();
    double tcurvsign = htrack->getTrackCurvature();         // CWessel, 05012016
    //double tphi = htrack->getTrackPhi() - (M_PI / 2.0);
    double tphi = htrack->getTrackPhi() - (M_PI / 1.0);
    //double tphi = htrack->getTrackPhi();
    double ttheta = htrack->getTrackTheta();

    rt = tcurvsign * fabs(rt);          // CWessel, 05012016

    B2DEBUG(200, "  Track info r:" << rt << " phi: " << tphi << " theta: " << ttheta);

    /* Determine qOH */
    // replace rt by fabs(rt), CWessel, 05012016
    if (fabs(rt) > 500.0) {
      qOH = 2;
    } else if (fabs(rt) > 250.0) {
      qOH = 1;
    } else {
      qOH = 0;
    }


    if (m_PXDTbExtrapolation) {

      //double rho = 1.0/fabs(rt);
      //y = sensorRadius * (rho * sensorRadius - cos(tphi)) / sin(tphi);

    } else {

      found = false;
      VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      BOOST_FOREACH(VxdID layer, geo.getLayers(SensorInfo::PXD)) {
        BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)) {
          BOOST_FOREACH(VxdID sensorID, geo.getSensors(ladder)) {
            const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
            transfMatrix = currentSensor->getTransformation();
            rotMatrix = transfMatrix.GetRotationMatrix();
            transMatrix = transfMatrix.GetTranslation();
            double width = currentSensor->getWidth();
            double length = currentSensor->getLength();
            //double shift = pxdShift[sensorID.getLayerNumber() - 1];
            double sphi = acos(rotMatrix[2]);
            double rs = sqrt((transMatrix[0] * transMatrix[0] + transMatrix[1] * transMatrix[1]));      // rs = radius of sensor layer

            /* Get z_shift for sensor */
            if (sensorID.getLayerNumber() == 1) {
              z_shift = pxdZ1Shift[sensorID.getSensorNumber() - 1];
            } else {
              z_shift = pxdZ2Shift[sensorID.getSensorNumber() - 1];
            }

            if (asin(rotMatrix[0]) > 0.01) {
              sphi = 2.0 * M_PI - sphi;
            }

            if (sphi > M_PI) {
              sphi = sphi - M_PI * 2.0;
            }

            angle = tphi - sphi;
            //       if (b >= 0.0) {
            if (tcurvsign == +1 /* =negativ charge */) {
              a = rs + fabs(rt) * sin(angle);
              b = rt * rt - a * a;
              cp = rt * cos(angle) + sqrt(b);                 // should be the wrong option from thinking about the geometry
              cm = rt * cos(angle) - sqrt(b);                 // should be the right option from thinking about the geometry
              //x = rs * cos(sphi) - sin(sphi) * cm;
              //y = rs * sin(sphi) + cos(sphi) * cm;
              x = rs;
              //y = +1.0*rt*cos(angle) - sqrt(b);
              y = cp;
            } else if (tcurvsign == -1 /* positive charge */) {
              //angle += M_PI/2.0;
              a = rs - fabs(rt) * sin(angle);
              b = rt * rt - a * a;
              //cp = -1.0*rt * cos(angle) + sqrt(b);            // should be the wrong option (+sqrt(b)) from thinking about the geometry, and -rt*cos(angle) should be right from calculation
              //cm = -1.0*rt * cos(angle) - sqrt(b);            // should be the right option (-sqrt(b)) from thinking about the geometry, and -rt*cos(angle) should be right from calculation
              cp = +1.0 * rt * cos(angle) + sqrt(
                     b);          // should be the wrong option (+sqrt(b)) from thinking about the geometry, and +rt*cos(angle) should be wrong from calculation
              cm = +1.0 * rt * cos(angle) - sqrt(
                     b);          // should be the right option (-sqrt(b)) from thinking about the geometry, and +rt*cos(angle) should be wrong from calculation
              //x = rs * cos(sphi) + sin(sphi) * cm;
              //y = rs * sin(sphi) - cos(sphi) * cm;
              x = rs;
              //y = -1.0*rt*cos(angle) + sqrt(b);
              y = cp;
            } else if (tcurvsign == 0 /* assume straight line */) {
              //x = rs * (cos(sphi) - sin(sphi) * tan(angle));
              //y = rs * (sin(sphi) + cos(sphi) * tan(angle));
              x = rs;
              y = rs * tan(angle);
            }

            // TODO: Have another look on this if-statement, maybe it is not correct
            if (y < maxY[sensorID.getLayerNumber() - 1] && y > minY[sensorID.getLayerNumber() - 1]) {
              mph_x = x * cos(sphi) - y * sin(sphi);
              mph_y = x * sin(sphi) + y * cos(sphi);
              //mph_x = x;
              //mph_y = y;
              //mph_z = mph_y / tan(ttheta);    // works for projection stuff in theta reco

              // should work for new algorithm with r in theta reco
              if (ttheta < (M_PI / 2.0)) {
                mph_z = sqrt(mph_x * mph_x + mph_y * mph_y) / tan(ttheta);
              } else if (ttheta > (M_PI / 2.0)) {
                mph_z = sqrt(mph_x * mph_x + mph_y * mph_y) / tan(ttheta);
              } else {
                mph_z = 0.0;
              }

              // TODO: Also look at this if-statement for correctnes
              if (mph_z > ((length / -2.0) + z_shift) && mph_z < ((length / 2.0) + z_shift)) {
                B2DEBUG(200, "    ->Found in PXD Sensor ID: " << sensorID << "\tPhi: "
                        << sphi / Unit::deg << " Radius: " << rs << " Shift: " << z_shift
                        << " Width: " << width << " Length: " << length);
                B2DEBUG(200, "        Candidate is: " << rs << ", " << y);
                B2DEBUG(200, "         -> Transformed: " << mph_x << ", " << mph_y << ", " << mph_z);
                mph_pos.SetXYZ(mph_x, mph_y, mph_z);

                static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
                const VXD::SensorInfoBase& info = geo.get(sensorID);
                TVector3 local_pos = info.pointToLocal(mph_pos);

                //extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(y - shift, mph_z - z_shift)));
                extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(local_pos.X(), local_pos.Y())));

                m_treeExtHitEventNo = evtNumber;
                m_treeExtHitHitNo   = ExtrapolatedHitNumber++;
                m_treeExtHitLayerNo = sensorID.getLayerNumber();
                m_treeExtHitLadderNo = sensorID.getLadderNumber();
                m_treeExtHitSensorNo = sensorID.getSensorNumber();
                m_treeExtHitTheta = ttheta;
                m_treeExtHitX = x;
                m_treeExtHitY = y;
                m_treeExtHitPosX    = mph_x;
                m_treeExtHitPosY    = mph_y;
                m_treeExtHitPosZ    = mph_z;
                /** Local coordinates u, v */
                double u = local_pos.X(), v = local_pos.Y();
                //m_treeExtHitPosU    = local_pos.X();
                //m_treeExtHitPosV    = local_pos.Y();
                m_treeExtHitPosU    = u;
                m_treeExtHitPosV    = v;
                m_treeExtHitUCell   = currentSensor->getUCellID(u, v, false);
                m_treeExtHitVCell   = currentSensor->getVCellID(v, false);

                if (sensorID.getLayerNumber() == 1) {
                  m_treeExtHitLadderNo_l1 = sensorID.getLadderNumber();
                  m_treeExtHitSensorNo_l1 = sensorID.getSensorNumber();
                  m_treeExtHitTheta_l1 = ttheta;
                  m_treeExtHitX_l1 = x;
                  m_treeExtHitY_l1 = y;
                  m_treeExtHitPosX_l1    = mph_x;
                  m_treeExtHitPosY_l1    = mph_y;
                  m_treeExtHitPosZ_l1    = mph_z;
                  m_treeExtHitPosU_l1    = u;
                  m_treeExtHitPosV_l1    = v;
                  m_treeExtHitUCell_l1   = currentSensor->getUCellID(u, v, false);
                  m_treeExtHitVCell_l1   = currentSensor->getVCellID(v, false);
                } if (sensorID.getLayerNumber() == 2) {
                  m_treeExtHitLadderNo_l2 = sensorID.getLadderNumber();
                  m_treeExtHitSensorNo_l2 = sensorID.getSensorNumber();
                  m_treeExtHitTheta_l2 = ttheta;
                  m_treeExtHitX_l2 = x;
                  m_treeExtHitY_l2 = y;
                  m_treeExtHitPosX_l2    = mph_x;
                  m_treeExtHitPosY_l2    = mph_y;
                  m_treeExtHitPosZ_l2    = mph_z;
                  m_treeExtHitPosU_l2    = u;
                  m_treeExtHitPosV_l2    = v;
                  m_treeExtHitUCell_l2   = currentSensor->getUCellID(u, v, false);
                  m_treeExtHitVCell_l2   = currentSensor->getVCellID(v, false);
                }

                // may be wrong here, reason: not all theta values are filled in the histograms, but this probably points to another error :/
                m_rootTreeExtrapolatedHits->Fill();

                storeExtrapolatedHits.appendNew(SVDHoughCluster(idx, mph_pos, sensorID, qOH));
                ++idx;
                found = true;
                break;
              } else {
                B2DEBUG(200, "    !Failed in PXD Sensor ID: " << sensorID << "\tPhi: "
                        << sphi / Unit::deg << " Radius: " << rs << " Shift: " << z_shift
                        << " Width: " << width << " Length: " << length);
                B2DEBUG(200, "      Candidate was: " << rs << ", " << y);
              }
            } else {
              B2DEBUG(200, "    !Failed at y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
            }

            //        } else {
            //          B2DEBUG(200, "    !Failed at a: " << a <<  " b: " << b << " angle: " << angle << " Sensor-Phi: "
            //                            << sphi  << " |tphi-sphi|: " << fabs((htrack->getTrackPhi() - sphi)));
            //        }

            /*
            if ((pow(fabs(a), 2.0) - pow(rs, 2.0)) >= 0.0 &&
                fabs((htrack->getTrackPhi() - sphi)) < (M_PI / 2.0)) {
              x = a + sqrt(pow(fabs(a), 2.0) - pow(rs, 2.0));
              b = -1 * pow(fabs(x), 2.0) - (2 * x * fabs(rt));

              if (b >= 0.0) {
                y = x * sin(angle) + sqrt(b) * cos(angle);

                //if (y < (shift + (width / 2.0)) && y > (shift + (width / -2.0))) {
                if (y < maxY[sensorID.getLayerNumber() - 1] && y > minY[sensorID.getLayerNumber() - 1]) {
                  mph_x = rs * cos(sphi) - y * sin(sphi);
                  mph_y = rs * sin(sphi) + y * cos(sphi);
                  mph_z = mph_y / tan(ttheta);

                  if (mph_z > ((length / -2.0) + z_shift) && mph_z < ((length / 2.0) + z_shift)) {
                    B2DEBUG(200, "    ->Found in PXD Sensor ID: " << sensorID << "\tPhi: "
                            << sphi / Unit::deg << " Radius: " << rs << " Shift: " << z_shift
                            << " Width: " << width << " Length: " << length);
                    B2DEBUG(200, "        Candidate is: " << rs << ", " << y);
                    B2DEBUG(200, "         -> Transformed: " << mph_x << ", " << mph_y << ", " << mph_z);
                    mph_pos.SetXYZ(mph_x, mph_y, mph_z);

                    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
                    const VXD::SensorInfoBase& info = geo.get(sensorID);
                    TVector3 local_pos = info.pointToLocal(mph_pos);

                    //extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(y - shift, mph_z - z_shift)));
                    extrapolatedHits.push_back(std::make_pair(sensorID, TVector2(local_pos.X(), local_pos.Y())));

                    storeExtrapolatedHits.appendNew(SVDHoughCluster(idx, mph_pos, sensorID, qOH));
                    ++idx;
                    found = true;
                    break;
                  } else {
                    B2DEBUG(200, "    !Failed in PXD Sensor ID: " << sensorID << "\tPhi: "
                            << sphi / Unit::deg << " Radius: " << rs << " Shift: " << z_shift
                            << " Width: " << width << " Length: " << length);
                    B2DEBUG(200, "      Candidate was: " << rs << ", " << y);
                  }
                } else {
                  B2DEBUG(200, "    !Failed at y = " << y << " a: " << a << " x: " << x << " b: " << b << " angle: " << angle);
                }
              } else {
                B2DEBUG(200, "    !Failed at x: " << x << " b: " << b << " a: " << a << " angle: " << angle
                        << " Sensor-Phi: " << sphi << " |tphi-sphi|: " << fabs((htrack->getTrackPhi() - sphi)));
              }
            } else {
              B2DEBUG(200, "    !Failed at a: " << a << " angle: " << angle << " Sensor-Phi: " << sphi
                      << " |tphi-sphi|: " << fabs((htrack->getTrackPhi() - sphi)));
            }
            */
          }

          if (found) {
            //break;
          }
        }
      }
    }
  }
}


/*
 * Analyzer tool for the extrapolated hits and check for its performance
 * in the pxd detector.
 */
void
SVDHoughTrackingModule::analyseExtrapolatedFull()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  PXDTrueHit* currHit;
  MCParticle* particle;
  TVector2 local_pos;
  TVector2 mph_pos;
  VxdID sensorID, mph_sensorID;
  int nPXD, nMCParticles;
  int nMcTrue;
  double distance, min_distance;
  double distance_u, distance_v;
  double min_distance_u, min_distance_v;
  double uPitch, vPitch;
  double half_width, half_length;

  RelationArray mcParticlesToTrueHits(storeMCParticles, storePXDTrueHits);

  nPXD          = storePXDTrueHits.getEntries();
  nMcTrue       = mcParticlesToTrueHits.getEntries();
  nMCParticles  = storeMCParticles.getEntries();

  B2DEBUG(200, "Analyze extrapolated hits: " << nPXD << " PXD TrueHits and "
          << extrapolatedHits.size() << " extrapolated Hits");

  if (!mcParticlesToTrueHits.isValid()) {
    B2WARNING("  Relation Array not valid!");
  }


  B2DEBUG(200, "  Number of MCParticles: " << nMCParticles);
  for (int i = 0; i < nMcTrue; ++i) {
    B2DEBUG(200, "  nMCTrueRelation: " << i);
    particle = storeMCParticles[i];

    //  if(/*particle->getMomentum().Perp() < 0.04 || */ particle->getMomentum().Theta()/Unit::deg < 16 || particle->getMomentum().Theta()/Unit::deg > 152  ) {
    //    continue;
    //  }

    //if (particle->getMother() == NULL) {
    /*if (particle->hasStatus(MCParticle::c_PrimaryParticle) && particle->getCharge() != 0) { */
    if (particle->hasStatus(MCParticle::c_PrimaryParticle) &&  particle->hasStatus(MCParticle::c_StableInGenerator) &&
        /* (particle->hasStatus(MCParticle::c_LeftDetector) || particle->hasStatus(MCParticle::c_StoppedInDetector)) && */
        particle->getCharge() != 0) {
      B2DEBUG(200, "  I'm a primary!");
    }
    for (unsigned int j = 0; j < mcParticlesToTrueHits[i].getToIndices().size(); j++) {
      B2DEBUG(200, "    Hit id: " << mcParticlesToTrueHits[i].getToIndex(j));
    }
  }


  if (nPXD == 0) {
    return;
  }

  for (auto it = extrapolatedHits.begin(); it != extrapolatedHits.end(); ++it) {

    m_treeExtHitAnaEventNo++;

    mph_sensorID = it->first;
    mph_pos = it->second;
    min_distance = 1e+99;
    B2DEBUG(200, "  Extrapolated hit SensorID: " << mph_sensorID << " Coord: "
            << mph_pos.X() << ", " << mph_pos.Y());

    for (int i = 0; i < nPXD; ++i) {
      currHit = storePXDTrueHits[i];
      sensorID = currHit->getSensorID();

      //static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      //const VXD::SensorInfoBase& info = geo.get(sensorID);
      const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
      half_width = currentSensor->getWidth() / 2.0;
      half_length = currentSensor->getLength() / 2.0;

      local_pos.Set(currHit->getU(), currHit->getV());
      B2DEBUG(200, "    PXD hit SensorID: " << sensorID << " Coord: " << local_pos.X() << ", " << local_pos.Y());

      /* so nicht gut, weil berücksichtigt nicht, wenn der extrapolierte oder
       * wahre Hit gerade auf der Grenze zum nächsten Sensor liegt */

      if (mph_sensorID != sensorID) {
        continue;
      }



      // distance = sqrt(pow(fabs(local_pos.X() - mph_pos.X()), 2.0) + pow(fabs(local_pos.Y() - mph_pos.Y()), 2.0));
      distance = sqrt(((local_pos.X() - mph_pos.X()) * (local_pos.X() - mph_pos.X()))
                      + ((local_pos.Y() - mph_pos.Y()) * (local_pos.Y() - mph_pos.Y())));

      if (distance < min_distance) {
        min_distance = distance;
        //min_distance_u = fabs(local_pos.X() - mph_pos.X());
        //min_distance_v = fabs(local_pos.Y() - mph_pos.Y());
        min_distance_u = local_pos.X() - mph_pos.X();
        min_distance_v = local_pos.Y() - mph_pos.Y();
      }

      /** New analysing algorithm (seems to be faulty, maybe!) */

      if (mph_sensorID.getLayerNumber() != sensorID.getLayerNumber()) {
        continue;
      }


      /** mph_LadderNumber for checking equal ladder numbers */
      int mph_LadderNumber = mph_sensorID.getLadderNumber();
      /** mph_SensorNumber for checking equal sensor numbers */
      int mph_SensorNumber = mph_sensorID.getSensorNumber();
      /** current hit sensor's LadderNumber for checking equal ladder numbers */
      int currLadderNumber = sensorID.getLadderNumber();
      /** current hit sensor's SensorNumber for checking equal sensor numbers */
      int currSensorNumber = sensorID.getSensorNumber();

      int deltaSensor = currSensorNumber - mph_SensorNumber;
      int deltaLadder = currLadderNumber - mph_LadderNumber;

      /** easiest case: sensorIDs of MPH and current sensor are equal */

      if (mph_sensorID == sensorID) {
        distance_u = local_pos.X() - mph_pos.X();
        distance_v = local_pos.Y() - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }
      } else if (deltaLadder == 1 && deltaSensor == 0) {

        distance_u = (half_width - local_pos.X()) - mph_pos.X();
        distance_v = local_pos.Y() - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == -1 && deltaSensor == 0) {

        distance_u = (-half_width + local_pos.X()) - mph_pos.X();
        distance_v = local_pos.Y() - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == 0 && deltaSensor == 1) {

        distance_u = local_pos.X() - mph_pos.X();
        distance_v = (half_length - local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == 0 && deltaSensor == -1) {

        distance_u = local_pos.X() - mph_pos.X();
        distance_v = (-half_length + local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == 1 && deltaSensor == 1) {

        distance_u = (half_width - local_pos.X()) - mph_pos.X();
        distance_v = (half_length - local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == 1 && deltaSensor == -1) {

        distance_u = (half_width - local_pos.X()) - mph_pos.X();
        distance_v = (-half_length + local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == -1 && deltaSensor == -1) {

        distance_u = (-half_width + local_pos.X()) - mph_pos.X();
        distance_v = (-half_length + local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      } else if (deltaLadder == -1 && deltaSensor == 1) {

        distance_u = (-half_width + local_pos.X()) - mph_pos.X();
        distance_v = (half_length - local_pos.Y()) - mph_pos.Y();
        if (fabs(distance_u) < fabs(min_distance_u)) {
          min_distance_u = distance_u;
        }
        if (fabs(distance_v) < fabs(min_distance_v)) {
          min_distance_v = distance_v;
        }

      }


      uPitch = currentSensor->getUPitch(local_pos.X());
      vPitch = currentSensor->getVPitch(local_pos.Y());


      /** End of new analysing algorithm */

    }

    static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& info = geo.get(sensorID);
    const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));

    uPitch = currentSensor->getUPitch(local_pos.X());
    vPitch = currentSensor->getVPitch(local_pos.Y());

    min_distance = sqrt(min_distance_u * min_distance_u + min_distance_v * min_distance_v);

    // (signed) deviation histograms
    m_histROIDiffPhi->Fill(min_distance_u);
    m_treeExtHitAnaDiffPhi = min_distance_u;
    m_histROIDiffTheta->Fill(min_distance_v);
    m_treeExtHitAnaDiffTheta = min_distance_v;
    m_histROIDiffPhiPx->Fill(min_distance_u / uPitch);
    m_treeExtHitAnaDiffPhiPx = min_distance_u / uPitch;
    m_histROIDiffThetaPx->Fill(min_distance_v / vPitch);
    m_treeExtHitAnaDiffThetaPx = min_distance_v / vPitch;
    m_histROIDiffPhiTheta->Fill(min_distance_u, min_distance_v);
    m_histROIDiffPhiThetaPx->Fill(min_distance_u / uPitch, min_distance_v / vPitch);

    if (mph_sensorID.getLayerNumber() == 1) {
      m_histROIDiffPhi_pxdl1->Fill(min_distance_u);
      m_treeExtHitAnaDiffPhi_pxdl1 = min_distance_u;
      m_histROIDiffTheta_pxdl1->Fill(min_distance_v);
      m_treeExtHitAnaDiffTheta_pxdl1 = min_distance_v;
      m_histROIDiffPhiPx_pxdl1->Fill(min_distance_u / uPitch);
      m_treeExtHitAnaDiffPhiPx_pxdl1 = min_distance_u / uPitch;
      m_histROIDiffThetaPx_pxdl1->Fill(min_distance_v / vPitch);
      m_treeExtHitAnaDiffThetaPx_pxdl1 = min_distance_v / vPitch;
      m_histROIDiffPhiTheta_pxdl1->Fill(min_distance_u, min_distance_v);
      m_histROIDiffPhiThetaPx_pxdl1->Fill(min_distance_u / uPitch, min_distance_v / vPitch);
    } else if (mph_sensorID.getLayerNumber() == 2) {
      m_histROIDiffPhi_pxdl2->Fill(min_distance_u);
      m_treeExtHitAnaDiffPhi_pxdl2 = min_distance_u;
      m_histROIDiffTheta_pxdl2->Fill(min_distance_v);
      m_treeExtHitAnaDiffTheta_pxdl2 = min_distance_v;
      m_histROIDiffPhiPx_pxdl2->Fill(min_distance_u / uPitch);
      m_treeExtHitAnaDiffPhiPx_pxdl2 = min_distance_u / uPitch;
      m_histROIDiffThetaPx_pxdl2->Fill(min_distance_v / vPitch);
      m_treeExtHitAnaDiffThetaPx_pxdl2 = min_distance_v / vPitch;
      m_histROIDiffPhiTheta_pxdl2->Fill(min_distance_u, min_distance_v);
      m_histROIDiffPhiThetaPx_pxdl2->Fill(min_distance_u / uPitch, min_distance_v / vPitch);

    }

    // unsigned / absolute deviation histograms
    //double min_dist_u_abs = fabs(min_distance_u);
    //double min_dist_v_abs = fabs(min_distance_v);
    //m_histROIDiffPhiabs->Fill(min_dist_u_abs);
    //m_histROIDiffThetaabs->Fill(min_dist_v_abs);
    //m_histROIDiffPhiPxabs->Fill(min_dist_u_abs / 0.005);
    //m_histROIDiffThetaPxabs->Fill(min_dist_v_abs / 0.0075);
    //m_histROIDiffPhiThetaabs->Fill(min_dist_u_abs, min_dist_v_abs);
    //m_histROIDiffPhiThetaPxabs->Fill(min_dist_u_abs / 0.005, min_dist_v_abs / 0.0075);

    for (int j = 0; j < nMcTrue; ++j) {
      TVector3 momentum = storeMCParticles[j]->getMomentum();
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.get(sensorID);
      TVector3 tmp = {local_pos.X(), local_pos.Y(), 0.0};
      TVector3 pos = info.pointToGlobal(tmp);

      //const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensorID));

      //uPitch = currentSensor->getUPitch(local_pos.X());
      //vPitch = currentSensor->getVPitch(local_pos.Y());

      // Test for small angular deviations (like in trackAnalyseMCParticle), but maybe not good to do so
      //if( ((momentum.Phi() - pos.Phi()) < 1.0) && ((momentum.Theta() == pos.Theta()) < 4) ){
      //if (((momentum.Phi() - pos.Phi()) < 1.0) && ((momentum.Theta() - pos.Theta()) < 4.0)) {
      // Put test of small angular deviations into comments since this might not be a good idea to do
      if ((fabs(momentum.Phi() / Unit::deg - pos.Phi() / Unit::deg) < 1.0)
          && (fabs(momentum.Theta() / Unit::deg - pos.Theta() / Unit::deg) < 4.0)) {
        // (signed) deviation histograms
        /** Total */
        m_histROIDiffPTPhi->Fill(momentum.Perp() , min_distance_u);
        m_histROIDiffPTPhiPx->Fill(momentum.Perp() , min_distance_u / uPitch);
        m_histROIDiffPTTheta->Fill(momentum.Perp() , min_distance_v);
        m_histROIDiffPTThetaPx->Fill(momentum.Perp() , min_distance_v / vPitch);
        m_treeExtHitAnaDiffPTPhi = min_distance_u;
        m_treeExtHitAnaDiffPTPhiPx = min_distance_u / uPitch;
        m_treeExtHitAnaDiffPTTheta = min_distance_v;
        m_treeExtHitAnaDiffPTThetaPx = min_distance_v / vPitch;
        m_treeExtHitAnaPTDist = momentum.Perp();

        /* PXD Layer 1 */
        if (mph_sensorID.getLayerNumber() == 1) {
          m_histROIDiffPTPhi_pxdl1->Fill(momentum.Perp() , min_distance_u);
          m_histROIDiffPTPhiPx_pxdl1->Fill(momentum.Perp() , min_distance_u / uPitch);
          m_histROIDiffPTTheta_pxdl1->Fill(momentum.Perp() , min_distance_v);
          m_histROIDiffPTThetaPx_pxdl1->Fill(momentum.Perp() , min_distance_v / vPitch);
          m_treeExtHitAnaDiffPTPhi_pxdl1 = min_distance_u;
          m_treeExtHitAnaDiffPTPhiPx_pxdl1 = min_distance_u / uPitch;
          m_treeExtHitAnaDiffPTTheta_pxdl1 = min_distance_v;
          m_treeExtHitAnaDiffPTThetaPx_pxdl1 = min_distance_v / vPitch;
          m_treeExtHitAnaPTDist_pxdl1 = momentum.Perp();
        } else if (mph_sensorID.getLayerNumber() == 2) {
          /* PXD Layer 2 */
          m_histROIDiffPTPhi_pxdl2->Fill(momentum.Perp() , min_distance_u);
          m_histROIDiffPTPhiPx_pxdl2->Fill(momentum.Perp() , min_distance_u / uPitch);
          m_histROIDiffPTTheta_pxdl2->Fill(momentum.Perp() , min_distance_v);
          m_histROIDiffPTThetaPx_pxdl2->Fill(momentum.Perp() , min_distance_v / vPitch);
          m_treeExtHitAnaDiffPTPhi_pxdl2 = min_distance_u;
          m_treeExtHitAnaDiffPTPhiPx_pxdl2 = min_distance_u / uPitch;
          m_treeExtHitAnaDiffPTTheta_pxdl2 = min_distance_v;
          m_treeExtHitAnaDiffPTThetaPx_pxdl2 = min_distance_v / vPitch;
          m_treeExtHitAnaPTDist_pxdl2 = momentum.Perp();
        }

        // unsigned / absolute deviation histograms
        //    m_histROIDiffPTabs->Fill(momentum.Perp() , min_distance);
        //    m_histROIDiffPTPhiabs->Fill(momentum.Perp() , min_dist_u_abs);
        //    m_histROIDiffPTPhiPxabs->Fill(momentum.Perp() , min_dist_u_abs / 0.005);
        //    m_histROIDiffPTThetaabs->Fill(momentum.Perp() , min_dist_v_abs);
        //    m_histROIDiffPTThetaPxabs->Fill(momentum.Perp() , min_dist_v_abs / 0.0075);
      }
    }

    m_rootTreeExtHitAna->Fill();

    B2DEBUG(100, "  => Min distance: " << min_distance << " u (x): " << min_distance_u << " v (y): "
            << min_distance_v);
  }
}



/*
 * Analyzer tool for the extrapolated hits and check for its performance
 * in the pxd detector.
 * ATTENTION: Probably obsolete and for the old SectorNeighbourFinder algorithm!
 * TODO: Thus will be deleted in (near) future
 */
void
SVDHoughTrackingModule::analyseExtrapolatedHits()
{
  StoreArray<SVDHoughCluster> storeExtrapolatedHits(m_storeExtrapolatedHitsName);
  std::map<int, TVector3>::iterator iter;
  TVector3 vec, closest_vec, clus;
  double dist, closest;

  unsigned int num_hits = storeExtrapolatedHits.getEntries();
  for (unsigned int i = 0; i < num_hits; ++i) {
    TVector3 hit = storeExtrapolatedHits[i]->getHitPos();
    VxdID sensor = storeExtrapolatedHits[i]->getSensorID();
    closest = 1.0e128;
    for (iter = pxd_cluster_map.begin(); iter != pxd_cluster_map.end(); ++iter) {
      clus = iter->second;
      vec = hit - clus;
      dist = vec.Mag();
      if ((fabs(tb_radius[sensor.getLayerNumber() - 1] - clus.X())) > 0.2) {
        continue;
      }

      if (dist < closest) {
        closest = dist;
        closest_vec = vec;
      }
    }
    //if (dist < 0.5) {
    const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(sensor));
    B2DEBUG(250, "Residuals for Sensor: " << sensor << " Pitch: " << currentSensor->getVPitch());
    B2DEBUG(250, "\tDx: " << fabs(closest_vec.x()) << " ("
            << fabs(closest_vec.x()) / currentSensor->getUPitch() << " px)"
            << " Dy: " <<  fabs(closest_vec.y())
            << " Dz: " << fabs(closest_vec.z()) << " (" <<  fabs(closest_vec.z()) / currentSensor->getVPitch()
            << " px)" << " Distance: " << closest);
    if (m_histROIy && m_histROIz) {
      m_histROIy->Fill(closest_vec.Y());
      m_histROIz->Fill(closest_vec.Z());
    }
    //}
  }
}
