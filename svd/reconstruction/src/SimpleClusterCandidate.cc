/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SimpleClusterCandidate.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/core/Environment.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDRecoDigit.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    SimpleClusterCandidate::SimpleClusterCandidate(VxdID vxdID, bool isUside, int sizeHeadTail, double cutSeed, double cutAdjacent,
                                                   double cutCluster, int timeAlgorithm)
      : m_vxdID(vxdID)
      , m_isUside(isUside)
      , m_sizeHeadTail(sizeHeadTail)
      , m_cutSeed(cutSeed)
      , m_cutAdjacent(cutAdjacent)
      , m_cutCluster(cutCluster)
      , m_timeAlgorithm(timeAlgorithm)
      , m_charge(0)
      , m_chargeError(0)
      , m_seedCharge(0)
      , m_6SampleTime(0)
      , m_6SampleTimeError(0)
      , m_position(0)
      , m_positionError(0)
      , m_SNR(0)
      , m_seedSNR(0)
      , m_seedIndex(-1)
      , m_strips(4)
      , m_storeShaperDigitsName("SVDShaperDigits")
      , m_storeRecoDigitsName("SVDRecoDigits")
    {m_strips.clear();};

    SimpleClusterCandidate::SimpleClusterCandidate(VxdID vxdID, bool isUside, int sizeHeadTail, double cutSeed, double cutAdjacent,
                                                   double cutCluster, int timeAlgorithm, const std::string& storeShaperDigitsName, const std::string& storeRecoDigitsName)
      : m_vxdID(vxdID)
      , m_isUside(isUside)
      , m_sizeHeadTail(sizeHeadTail)
      , m_cutSeed(cutSeed)
      , m_cutAdjacent(cutAdjacent)
      , m_cutCluster(cutCluster)
      , m_timeAlgorithm(timeAlgorithm)
      , m_charge(0)
      , m_chargeError(0)
      , m_seedCharge(0)
      , m_6SampleTime(0)
      , m_6SampleTimeError(0)
      , m_position(0)
      , m_positionError(0)
      , m_SNR(0)
      , m_seedSNR(0)
      , m_seedIndex(-1)
      , m_strips(4)
      , m_storeShaperDigitsName(storeShaperDigitsName)
      , m_storeRecoDigitsName(storeRecoDigitsName)
    {m_strips.clear();};

    bool SimpleClusterCandidate::add(VxdID vxdID, bool isUside, struct  stripInCluster& aStrip)
    {

      bool added = false;

      //do not add if you are on the wrong sensor or side
      if ((m_vxdID != vxdID) || (m_isUside != isUside))
        return false;

      //add if it's the first strip
      if (m_strips.size() == 0)
        added = true;

      //add if it adjacent to the last strip added
      //(we assume that SVDRecoDigits are ordered)
      if ((m_strips.size() > 0 && (aStrip.cellID == m_strips.at(m_strips.size() - 1).cellID + 1)))
        added  = true;

      //add it to the vector od strips, update the seed charge and index:
      if (added) {
        m_strips.push_back(aStrip);

        if (aStrip.charge > m_seedCharge) {
          m_seedCharge = aStrip.charge;
          m_seedSNR = aStrip.charge / aStrip.noise;
          m_seedIndex = m_strips.size() - 1;
        }
      }
      return added;

    };

    void SimpleClusterCandidate::finalizeCluster()
    {

      m_stopCreationCluster = false;

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      int maxStripCellID = m_strips.at(m_strips.size() - 1).cellID;
      double maxStripCharge = m_strips.at(m_strips.size() - 1).charge;
      int minStripCellID = m_strips.at(0).cellID;
      double minStripCharge = m_strips.at(0).charge;

      int clusterSize = m_strips.size();

      double weightSum = 0;
      double noise = 0;
      for (auto aStrip : m_strips) {
        double stripPos = m_isUside ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);
        m_position += stripPos * aStrip.charge;
        m_charge += aStrip.charge;
        m_6SampleTime += aStrip.time * aStrip.charge;

        float tmp_sigmaSquared = aStrip.timeError * aStrip.timeError;
        weightSum +=  tmp_sigmaSquared;
        //FIXME: use error to weight the time of each strip in the cluster
        // it seems to yield a worst resolution vs EventT0 and an additional 1 ns bias
        //  m_6SampleTime += aStrip.time / tmp_sigmaSquared;
        // additional change also below: m_6SampleTime /= weightSum instead of m_6SampleTime/=m_charge
        noise += aStrip.noise * aStrip.noise;
      }

      if ((m_charge == 0) || (noise == 0)) {
        m_stopCreationCluster = true;
        return;
      }

      noise = sqrt(noise);
      m_6SampleTime /= m_charge;
      //      m_6SampleTime /= weightSum;
      m_6SampleTimeError = 1. / TMath::Sqrt(weightSum);
      m_SNR = m_charge / noise;


      if (clusterSize < m_sizeHeadTail) { // COG, size = 1 or 2
        m_position /= m_charge;
        // Compute position error
        if (clusterSize == 1) {
          // Add a strip charge equal to the zero-suppression threshold to compute the error
          double phantomCharge = m_cutAdjacent * m_strips.at(0).noise;
          m_positionError = pitch * phantomCharge / (m_charge + phantomCharge);
        } else {
          double a = m_cutAdjacent;
          if (m_strips.at(0).noise == 0) {
            m_stopCreationCluster = true;
            return;
          }
          double sn = m_charge / m_strips.at(0).noise;
          if (sn == 0) {
            m_stopCreationCluster = true;
            return;
          }
          m_positionError = a * pitch / sn;
        }
      } else { // Head-tail
        double centreCharge = (m_charge - minStripCharge - maxStripCharge) / (clusterSize - 2);
        if (centreCharge == 0) {
          m_stopCreationCluster = true;
          return;
        }

        minStripCharge = (minStripCharge < centreCharge) ? minStripCharge : centreCharge;
        maxStripCharge = (maxStripCharge < centreCharge) ? maxStripCharge : centreCharge;
        double minPos = m_isUside ? info.getUCellPosition(minStripCellID) : info.getVCellPosition(minStripCellID);
        double maxPos = m_isUside ? info.getUCellPosition(maxStripCellID) : info.getVCellPosition(maxStripCellID);
        m_position = 0.5 * (minPos + maxPos + (maxStripCharge - minStripCharge) / centreCharge * pitch);

        double sn = centreCharge / m_cutAdjacent / noise;
        // Rough estimates of Landau noise
        double landauHead = minStripCharge / centreCharge;
        double landauTail = maxStripCharge / centreCharge;
        m_positionError = 0.5 * pitch * sqrt(1.0 / sn / sn +
                                             0.5 * landauHead * landauHead +
                                             0.5 * landauTail * landauTail);
      }

      //Lorentz shift correction - PATCHED
      //NOTE: layer 3 is upside down with respect to L4,5,6 in the real data (real SVD), but _not_ in the simulation. We need to change the sign of the Lorentz correction on L3 only if reconstructing data, i.e. if Environment::Instance().isMC() is FALSE.
      const SensorInfo& sensorInfo = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(m_vxdID));

      bool isMC = Environment::Instance().isMC();

      if ((m_vxdID.getLayerNumber() == 3) && ! isMC)
        m_position += sensorInfo.getLorentzShift(m_isUside, m_position);
      else
        m_position -= sensorInfo.getLorentzShift(m_isUside, m_position);

    };

    bool SimpleClusterCandidate::isGoodCluster()
    {

      bool isGood = false;

      if (m_stopCreationCluster) {
        B2WARNING("Something is wrong in the cluster creation, this cluster will not be created!");
        return false;
      }

      if (m_seedCharge > 0 && m_seedSNR >= m_cutSeed && m_SNR >= m_cutCluster)
        isGood = true;

      return isGood;
    };


    float SimpleClusterCandidate::getTime() const
    {

      if (m_timeAlgorithm == 0)
        return get6SampleCoGTime();
      else if (m_timeAlgorithm == 1)
        return get3SampleCoGRawTime();
      else if (m_timeAlgorithm == 2)
        return get3SampleELSRawTime();
      else {
        B2FATAL("unrecognized timeAlgorithm, please check the input parameter and select a value among {0 (6-sample CoG), 1 (3-sample CoG), 2 (3-sample ELS)} to select the algorithm that you want to reconstruct the cluster time");
        return 0;
      }
    }

    float SimpleClusterCandidate::getTimeError() const
    {

      if (m_timeAlgorithm == 0)
        return get6SampleCoGTimeError();
      else if (m_timeAlgorithm == 1)
        return get3SampleCoGTimeError();
      else if (m_timeAlgorithm == 2)
        return get3SampleELSTimeError();
      else {
        B2FATAL("unrecognized timeAlgorithm, please check the input parameter and select a value among {0 (6-sample CoG), 1 (3-sample CoG), 2 (3-sample ELS)} to select the algorithm that you want to reconstruct the cluster time");
        return 0;
      }
    }

    float SimpleClusterCandidate::get3SampleCoGRawTime() const
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = getMaxSum3Samples().second;
      auto begin = clustered3s.begin();
      const auto end = clustered3s.end();

      //calculate 'raw' CoG3 hit-time
      constexpr auto stepSize = 16000. / 509; //APV25 clock period = 31.4 ns
      auto retval = 0., norm = 0.;
      for (auto step = 0.; begin != end; ++begin, step += stepSize) {
        norm += static_cast<double>(*begin);
        retval += static_cast<double>(*begin) * step;
      }
      float rawtime = retval / norm;

      return rawtime;
    }

    float SimpleClusterCandidate::get3SampleELSRawTime() const
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = getMaxSum3Samples().second;
      const auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-time
      constexpr auto stepSize = 16000. / 509; //APV25 clock period = 31.4 ns
      constexpr auto tau = 55;//ELS time constant, default55
      auto num = 2 * stepSize * std::exp(-4 * stepSize / tau) + std::exp(-2 * stepSize / tau) * stepSize / 2 * (*begin - std::exp(
                   -2 * stepSize / tau) * (*(begin + 2))) / (*begin + std::exp(-stepSize / tau) * (*(begin + 1)) / 2);
      auto denom = 1 - std::exp(-4 * stepSize / tau) - (1 + std::exp(-2 * stepSize / tau) / 2) * (*begin - std::exp(
                     -2 * stepSize / tau) * (*(begin + 2))) / (*begin + std::exp(-stepSize / tau) * (*(begin + 1)) / 2);
      float rawtime = - num / denom;

      return rawtime;

    }
    float SimpleClusterCandidate::get3SampleCoGTimeError() const
    {

      //no obvious way to compute the error yet
      return 6;
    }

    float SimpleClusterCandidate::get3SampleELSTimeError() const
    {

      //no obvious way to compute the error yet
      return 6;
    }


    Belle2::SVDShaperDigit::APVFloatSamples SimpleClusterCandidate::getClsSamples() const
    {

      if (m_strips.size() == 0)
        B2ERROR(" you are asking fo the cluster samples for a cluster candidate with no strips, it make no sense to ask for the cluster time!");

      //steps:
      //1.loop on m_strips
      //2. access the index of the recodigit from the element of m_strip
      //3. get the shaperdigit related to the recodigit of index recoDigitIndex
      //4. sum each sample for each strip accessed in the loop
      //5. you are done

      Belle2::SVDShaperDigit::APVFloatSamples returnSamples = {0, 0, 0, 0, 0, 0};
      //FIXME: the name of the StoreArray of RecoDigits and ShaperDigits
      // must be taken from the SimpleClusterizer.
      const StoreArray<SVDRecoDigit> m_storeRecoDigits(m_storeRecoDigitsName.c_str());
      for (auto istrip : m_strips) {
        const SVDShaperDigit* shaperdigit = m_storeRecoDigits[istrip.recoDigitIndex]->getRelatedTo<SVDShaperDigit>
                                            (m_storeShaperDigitsName.c_str());
        if (!shaperdigit) B2ERROR("No shaperdigit for strip!?");
        Belle2::SVDShaperDigit::APVFloatSamples APVsamples = shaperdigit->getSamples();
        for (int iSample = 0; iSample < static_cast<int>(APVsamples.size()); ++iSample)
          returnSamples.at(iSample) += APVsamples.at(iSample);
      }

      return returnSamples;
    }

    std::pair<int, std::vector<float>> SimpleClusterCandidate::getMaxSum3Samples() const
    {

      //take the cluster samples
      Belle2::SVDShaperDigit::APVFloatSamples clsSamples = getClsSamples();

      //Max Sum selection
      if (clsSamples.size() < 3) B2ERROR("APV25 samples less than 3!?");
      std::vector<float> Sum2bin(clsSamples.size() - 1, 0);
      for (int iBin = 0; iBin < static_cast<int>(clsSamples.size()) - 1; ++iBin)
        Sum2bin.at(iBin) = clsSamples.at(iBin) + clsSamples.at(iBin + 1);
      auto itSum = std::max_element(std::begin(Sum2bin), std::end(Sum2bin));
      int ctrFrame = std::distance(std::begin(Sum2bin), itSum);
      if (ctrFrame == 0) ctrFrame = 1;
      std::vector<float> clustered3s = {clsSamples.at(ctrFrame - 1), clsSamples.at(ctrFrame), clsSamples.at(ctrFrame + 1)};

      return std::make_pair(ctrFrame - 1, clustered3s);

    }


  }  //SVD namespace
} //Belle2 namespace
