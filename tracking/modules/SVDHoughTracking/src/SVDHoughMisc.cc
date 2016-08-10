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


void
SVDHoughTrackingModule::saveHits()
{
  uint32_t cnt;
  bool createLUT = true;
  bool conformal = true;
  double scale = 1024.0 * 1024.0 * 1024.0;
  double rsqr;
  double x, y, z;
  int32_t x32, y32, z32;
  short layer;
  ofstream p_pos;

  if (m_tbMapping) {
    conformal = false;
    scale = 1;
  }

  /* Open file for output position writing */
  if (createLUT) {
    p_pos.open("p_clusters.v", ofstream::out);
    p_pos << "\t\t\tcase(cnt)" << endl;
    cnt = 0;

    p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
    p_pos << "\t\t\t\t\tx <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\ty <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\twr <= 0;" << endl;
    p_pos << "\t\t\t\t\tlayer <= 2'd" << 0 << ";" << endl;
    p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
    p_pos << "\t\t\t\tend" << endl;
    ++cnt;

    for (auto it = p_clusters.begin(); it != p_clusters.end(); ++it) {
      x = it->second.second.X() / Unit::um;
      y = it->second.second.Y() / Unit::um;
      z = it->second.second.Z() / Unit::um;
      layer = it->second.first.getLayerNumber() - 3; /* we start at layer 0 */

      if (conformal) {
        rsqr = ((x * x) + (y * y));
        x = x / rsqr;
        y = y / rsqr;
      }

      x32 = (int32_t)(x * scale);
      y32 = (int32_t)(y * scale);
      z32 = (int32_t)(z * scale);

      p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
      p_pos << "\t\t\t\t\tx <= " << convertInt(x32) << ";" << endl;
      p_pos << "\t\t\t\t\ty <= " << convertInt(y32) << ";" << endl;
      p_pos << "\t\t\t\t\twr <= 1;" << endl;
      p_pos << "\t\t\t\t\tlayer <= 2'd" << layer << ";" << endl;
      p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
      p_pos << "\t\t\t\tend" << endl;
      ++cnt;
    }
    p_pos << "\t\t\tendcase" << endl;
    p_pos.close();

    /* N clusters */
    p_pos.open("n_clusters.v", ofstream::out);
    p_pos << "\t\t\tcase(cnt)" << endl;
    cnt = 0;

    p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
    p_pos << "\t\t\t\t\tx <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\ty <= " << convertInt(0) << ";" << endl;
    p_pos << "\t\t\t\t\twr <= 0;" << endl;
    p_pos << "\t\t\t\t\tlayer <= 2'd" << 0 << ";" << endl;
    p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
    p_pos << "\t\t\t\tend" << endl;
    ++cnt;

    for (auto it = n_clusters.begin(); it != n_clusters.end(); ++it) {
      x = it->second.second.X() / Unit::um;
      y = it->second.second.Y() / Unit::um;
      z = it->second.second.Z() / Unit::um;
      layer = it->second.first.getLayerNumber() - 3; /* we start at layer 0 */

      if (conformal) {
        rsqr = ((x * x) + (y * y));
        x = x / rsqr;
        y = y / rsqr;
      }

      x32 = (int32_t)(x * scale);
      y32 = (int32_t)(y * scale);
      z32 = (int32_t)(z * scale);

      p_pos << "\t\t\t\t32'd" << cnt << ": begin" << endl;
      p_pos << "\t\t\t\t\tx <= " << convertInt(x32) << ";" << endl;
      p_pos << "\t\t\t\t\ty <= " << convertInt(z32) << ";" << endl;
      p_pos << "\t\t\t\t\twr <= 1;" << endl;
      p_pos << "\t\t\t\t\tlayer <= 2'd" << layer << ";" << endl;
      p_pos << "\t\t\t\t\tlast_out <= 0;" << endl;
      p_pos << "\t\t\t\tend" << endl;
      ++cnt;
    }
    p_pos << "\t\t\tendcase" << endl;
  } else {
    p_pos.open("p_clusters.dat", ofstream::out);

    for (auto it = p_clusters.begin(); it != p_clusters.end(); ++it) {
      p_pos << it->second.second.X() << " " << it->second.second.Y() << " "
            << it->second.second.Z() << " " << endl;
    }
  }

  p_pos.close();
}

void
SVDHoughTrackingModule::save_hits()
{
  StoreArray<SVDTrueHit> storeSVDTrueHits(m_storeSVDTrueHitsName);
  StoreArray<SVDDigit>   storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<PXDTrueHit> storePXDTrueHits(m_storePXDTrueHitsName);
  StoreArray<PXDDigit>   storePXDDigits(m_storePXDDigitsName);
  RelationArray relDigitTrueHit(storeSVDDigits, storeSVDTrueHits, m_relDigitTrueHitName);

  /* Write out PXD data */
  unsigned int nPXDTruehits = storePXDTrueHits.getEntries();
  B2DEBUG(250, "Start hits processing: Number " << nPXDTruehits);
  for (unsigned int i = 0; i < nPXDTruehits; ++i) {
    of_pos << "# Sensor: " << storePXDTrueHits[i]->getSensorID() << endl;
    of_pos << storePXDTrueHits[i]->getU() / Unit::mm << " "
           <<  storePXDTrueHits[i]->getV() / Unit::mm << endl;
  }

  /* Write out SVD data */
  unsigned int nSVDTruehits = storeSVDTrueHits.getEntries();
  B2DEBUG(250, "Start hits processing: Number " << nSVDTruehits);
  for (unsigned int i = 0; i < nSVDTruehits; ++i) {
    of_pos << "# Sensor: " << storeSVDTrueHits[i]->getSensorID() << endl;
    of_pos << storeSVDTrueHits[i]->getU() / Unit::mm << " "
           <<  storeSVDTrueHits[i]->getV() / Unit::mm << endl;
  }
}


void SVDHoughTrackingModule::print_num()
{
  StoreArray<SVDHoughCluster> storeHoughCluster(m_storeHoughCluster);

  /* Number of found Hough Clusters */
  int num = storeHoughCluster.getEntries();
  B2DEBUG(200, "Number of in print_num: " << num << " ID: " << storeHoughCluster[num - 1]->getClusterID());
}


/*
 * Print Tracks
 */
void
SVDHoughTrackingModule::printTracks()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  int nTracks;
  double r, phi, theta;
  double m1, m2, a1, a2;

  nTracks = storeHoughTrack.getEntries();

  if (m_printTrackInfo) {
    B2INFO("Found tracks: ");
  } else {
    B2DEBUG(1, "Found tracks: ");
  }
  for (int i = 0; i < nTracks; ++i) {
    if (storeHoughTrack[i]->getTrackType()) {
      m1 = storeHoughTrack[i]->getTrackM1();
      a1 = storeHoughTrack[i]->getTrackA1();
      m2 = storeHoughTrack[i]->getTrackM2();
      a2 = storeHoughTrack[i]->getTrackA2();
      if (m_printTrackInfo) {
        cout << "  Straight Track [ " << i << " ] M1: " << m1 << " A1: " << a1 << " M2: " << m2
             << " A2: " << a2 << endl;
      } else {
        B2DEBUG(1, "  Straight Track [ " << i << " ] M1: " << m1 << " A1: " << a1 << " M2: " << m2
                << " A2: " << a2);
      }
    } else {
      r = storeHoughTrack[i]->getTrackR();
      phi = storeHoughTrack[i]->getTrackPhi();
      theta = storeHoughTrack[i]->getTrackTheta();
      if (m_printTrackInfo) {
        if (m_usePhiOnly) {
          cout << "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")" << endl;
        } else {
          cout << "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")"
               << " Theta: " << theta << " (" << theta / Unit::deg << ")" << endl;
        }
//        cout << "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")"
//             << " Theta: " << theta << " (" << theta / Unit::deg << ")" << endl;
      } else {
        B2DEBUG(1, "  Track [ " << i << " ] Radius: " << r << " Phi: " << phi << " (" << phi / Unit::deg << ")"
                << " Theta: " << theta << " (" << theta / Unit::deg << ")");
      }
    }
    ++allTracks;
  }
}



/*
 * Save digits.
 */
void
SVDHoughTrackingModule::saveStrips()
{
  StoreArray<SVDDigit> storeDigits(m_storeSVDDigitsName);
  int nDigits, sample_cnt;
  SVDDigit* digit;
  VxdID sensorID, last_sensorID;
  bool new_strip;
  bool p_side, last_side;
  short strip, last_strip;
  float sample;
  float samples[6];
  ofstream of_pstrip, of_nstrip;

  /* Open file for output position writing */
  of_pstrip.open("p_strips.dat", ofstream::out);
  of_nstrip.open("n_strips.dat", ofstream::out);
  of_pstrip << "Event " << runNumber << endl;
  of_nstrip << "Event " << runNumber << endl;

  of_pstrip << "FADC 1" << endl;
  of_nstrip << "FADC 129" << endl;

  last_sensorID = VxdID(0);
  last_side = false;
  new_strip = true;
  last_strip = 0;
  sample_cnt = 0;

  nDigits = storeDigits.getEntries();
  B2DEBUG(250, "Save strips: " << nDigits);
  if (nDigits == 0) {
    return;
  }

  /* Loop over all Digits and aggregate first all samples from a
   * single strip.
   * We assume they are sorted!
   */
  for (int i = 0; i < nDigits; i++) {
    digit = storeDigits[i];
    sensorID = digit->getSensorID();
    p_side = digit->isUStrip();
    strip = digit->getCellID();
    sample = digit->getCharge();

    const SensorInfo* sensorInfo = dynamic_cast<const SensorInfo*>(&VXD::GeoCache::get(sensorID));
    B2DEBUG(350, "  ** Width: " << (sensorInfo->getWidth() / 2.0) << " Length: "
            << (sensorInfo->getLength() / 2.0));

    /* Same strip, add up all samples */
    if (sample_cnt == 5) {
      samples[sample_cnt] = sample;
      //writeStrip(sensorID, n_side ? false : true, strip, samples, n_side ? of_pstrip : of_nstrip);
      writeStrip(sensorID, p_side ? true : false, strip, samples, p_side ? of_pstrip : of_nstrip);
      sample_cnt = 0;
      new_strip = true;
    } else {
      //if (!new_strip && (last_side != n_side || last_sensorID != sensorID || last_strip != strip)) {
      if (!new_strip && (last_side != p_side || last_sensorID != sensorID || last_strip != strip)) {
        B2WARNING("Strips not sorted!");
      }

      new_strip = false;
      samples[sample_cnt] = sample;
      ++sample_cnt;
    }

    last_strip = strip;
    last_side = p_side;
    last_sensorID = sensorID;
  }

  of_pstrip.close();
  of_nstrip.close();
}

/*
 * Write strips to file.
 */
void
SVDHoughTrackingModule::writeStrip(VxdID sensorID, bool p_side, short strip, float* samples, ofstream& of)
{
  short layer = sensorID.getLayerNumber();
  short strips_per_apv = 128;
  short apv, max_strips, rest_strip;
  short apv_offset = 0;

  if (m_tbMapping) {
    if (layer == 3 || p_side) {
      max_strips = 768;
    } else {
      max_strips = 512;
    }
    B2DEBUG(3500, "\tMaximum number of strips: " << max_strips);

    switch (layer) {
      case 3:
        apv_offset = 0;
        break;
      case 4:
        if (!p_side) {
          apv_offset = 7;
        } else {
          apv_offset = 6;
        }
        break;
      case 5:
        if (!p_side) {
          apv_offset = 25;
        } else {
          apv_offset = 24;
        }
        break;
      case 6:
        apv_offset = 30;
        break;
    }

    apv = (strip / strips_per_apv);
    rest_strip = strip - (apv * strips_per_apv);

    /* For testbeam we need to remap layer 4 and 5 n-side */
    if (layer == 5 && !p_side) {
      switch (apv) {
        case 0:
          apv = 25;
          break;
        case 1:
          apv = 24;
          break;
        case 2:
          apv = 27;
          break;
        case 3:
          apv = 26;
          break;
      }
    } else if (layer == 4 && !p_side) {
      switch (apv) {
        case 0:
          apv = 7;
          break;
        case 1:
          apv = 6;
          break;
        case 2:
          apv = 9;
          break;
        case 3:
          apv = 8;
          break;
      }
    } else {
      apv += apv_offset;
    }

    /* Write to stream, format:
     * apv strip sample1 sample2 sample3 sample4 sample5 sample6
     */
    of << apv << " " << rest_strip << " " << samples[0] << " " << samples[1] << " " << samples[2] << " "
       << samples[3] << " " << samples[4] << " " << samples[5] << endl;

    B2DEBUG(250, "  Sensor: " << sensorID << " Side: " << p_side << " Strip: " << strip
            << " Charge: " << samples[0] << " " << samples[1] << " " << samples[2] << " " << samples[3]
            << " " << samples[4] << " " << samples[5] << " APV: " << apv << " Rest strip: " << rest_strip);
  } else {
    B2WARNING("Only testbeam mapping implemented so far for write strips!");
  }
}


/*
 * Run the noise filter over the given numbers of samples. If
 * it fulfills the requirements true is returned.
 */
bool
SVDHoughTrackingModule::noiseFilter(float* samples, int size, float* peak_sample)
{
  int i, peak_pos;
  *peak_sample = 0.0;
  peak_pos = 0;

  if (m_disableNoiseFilter) {
    return true;
  }

  /* Find maximum sample and save position */
  for (i = 0; i < size; ++i) {
    if (*peak_sample < samples[i]) {
      *peak_sample = samples[i];
      peak_pos = i;
    }
  }

  if (peak_pos > 0 && peak_pos < (size - 1)) {
    if (samples[peak_pos - 1] > (*peak_sample / m_noiseFactor)) {
      return (true);
    }
  }

  return (false);
}


