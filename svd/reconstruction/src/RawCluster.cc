/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/RawCluster.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/core/Environment.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    RawCluster::RawCluster(VxdID vxdID, bool isUside, double cutSeed, double cutAdjacent)
      : m_vxdID(vxdID)
      , m_isUside(isUside)
      , m_cutSeed(cutSeed)
      , m_cutAdjacent(cutAdjacent)
      , m_seedMaxSample(-1)
      , m_seedSNR(-1)
      , m_seedIndex(-1)
      , m_seedInternalIndex(-1)
      , m_storeShaperDigitsName("SVDShaperDigits")
    {m_strips.clear();};

    RawCluster::RawCluster(VxdID vxdID, bool isUside, double cutSeed, double cutAdjacent, const std::string& storeShaperDigitsName)
      : m_vxdID(vxdID)
      , m_isUside(isUside)
      , m_cutSeed(cutSeed)
      , m_cutAdjacent(cutAdjacent)
      , m_seedMaxSample(-1)
      , m_seedSNR(-1)
      , m_seedIndex(-1)
      , m_seedInternalIndex(-1)
      , m_storeShaperDigitsName(storeShaperDigitsName)
    {m_strips.clear();};

    bool RawCluster::add(VxdID vxdID, bool isUside, struct StripInRawCluster& aStrip)
    {

      bool added = false;

      //do not add if you are on the wrong sensor or side
      if ((m_vxdID != vxdID) || (m_isUside != isUside))
        return false;

      //do not add if its SNR is below the minimum SNR for adjacents strips
      if ((float)aStrip.maxSample / aStrip.noise < m_cutAdjacent)
        return false;

      //add if it's the first strip
      if (m_strips.size() == 0)
        added = true;

      //add if it is adjacent to the last strip added
      //(we assume that SVDShaperDigits are ordered)
      if ((m_strips.size() > 0 && (aStrip.cellID == m_strips.at(m_strips.size() - 1).cellID + 1)))
        added  = true;

      //add it to the vector of strips, update the seed max sample and index:
      if (added) {
        m_strips.push_back(aStrip);

        if (aStrip.maxSample > m_seedMaxSample) {
          m_seedMaxSample = aStrip.maxSample;
          m_seedSNR = (float)aStrip.maxSample / (float)aStrip.noise;
          m_seedInternalIndex = m_strips.size() - 1;
          m_seedIndex = aStrip.shaperDigitIndex;
        }
      }
      return added;

    };


    bool RawCluster::isGoodRawCluster()
    {

      bool isGood = false;

      if (m_seedMaxSample > 0 && m_seedSNR >= m_cutSeed)
        isGood = true;

      return isGood;
    };


    Belle2::SVDShaperDigit::APVFloatSamples RawCluster::getClsSamples(bool inElectrons) const
    {

      if (m_strips.size() == 0)
        B2ERROR("oopps ... you are asking for the cluster samples of a cluster candidate with no strips");

      //steps:
      //1.loop on m_strips
      //2. access the index of the shaperdigit from the element of m_strip
      //3. sum each sample for each strip accessed in the loop
      //4. you are done

      Belle2::SVDShaperDigit::APVFloatSamples returnSamples = {0, 0, 0, 0, 0, 0};

      const StoreArray<SVDShaperDigit> m_storeShaperDigits(m_storeShaperDigitsName.c_str());

      SVDPulseShapeCalibrations pulseShapeCal;

      int startPos = -1;
      int endPos   = -1;
      int clusterSize = static_cast<int>(m_strips.size());
      if (clusterSize > 3) {
        int maxDigitPos;
        float maxDigit = 0;
        int count = 0;
        for (auto istrip : m_strips) {
          const SVDShaperDigit* shaperdigit = m_storeShaperDigits[istrip.shaperDigitIndex];
          if (!shaperdigit) B2ERROR("No SVDShaperDigit for this strip! Are you sure you set the correct SVDShaperDigit StoreArray name?");
          Belle2::SVDShaperDigit::APVFloatSamples APVsamples = shaperdigit->getSamples();
          float fraction = 0;
          for (int iSample = 0; iSample < static_cast<int>(APVsamples.size()); ++iSample)
            fraction += APVsamples.at(iSample);
          // std::cout<<" count "<<count<<" fraction "<<fraction<<std::endl;
          if (fraction > maxDigit) {
            maxDigit = fraction;
            maxDigitPos = count;
          }
          count++;
        }
        if (maxDigitPos == 0) maxDigitPos = 1;
        if (maxDigitPos == clusterSize - 1) maxDigitPos = clusterSize - 2;
        startPos = maxDigitPos - 1;
        endPos   = maxDigitPos + 1;
        // std::cout<<" startPos "<<startPos<<" endPos "<<endPos<<std::endl;
      } // if (static_cast<int>(APVsamples.size()) > 3) {

      int count = 0;
      for (auto istrip : m_strips) {
        count++;
        if (startPos >= 0 && (count - 1 < startPos || count - 1 > endPos)) continue;
        // std::cout<<" count - 1 : "<<(count - 1)<<std::endl;
        const SVDShaperDigit* shaperdigit = m_storeShaperDigits[istrip.shaperDigitIndex];
        if (!shaperdigit) B2ERROR("No SVDShaperDigit for this strip! Are you sure you set the correct SVDShaperDigit StoreArray name?");
        Belle2::SVDShaperDigit::APVFloatSamples APVsamples = shaperdigit->getSamples();
        for (int iSample = 0; iSample < static_cast<int>(APVsamples.size()); ++iSample)
          if (inElectrons)
            returnSamples.at(iSample) += pulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, shaperdigit->getCellID(), APVsamples.at(iSample));
          else
            returnSamples.at(iSample) += APVsamples.at(iSample);
      }


      return returnSamples;
    }

    std::pair<int, std::vector<float>> RawCluster::getMaxSum3Samples(bool inElectrons) const
    {

      //take the cluster samples
      Belle2::SVDShaperDigit::APVFloatSamples clsSamples = getClsSamples(inElectrons);

      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(clsSamples);
      return std::make_pair(maxSum.getFirstFrame(), maxSum.getSelectedSamples());

    }

  }  //SVD namespace
} //Belle2 namespace
