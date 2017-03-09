/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPDigitizer/TimeDigitizer.h>
#include <top/geometry/TOPGeometryPar.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TimeDigitizer::TimeDigitizer(int moduleID, int pixelID, unsigned window,
                                 const TOP::PulseHeightGenerator& generator):
      m_moduleID(moduleID), m_pixelID(pixelID), m_window(window),
      m_pulseHeightGenerator(generator)
    {
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
      if (!channelMapper.isValid()) return;

      m_channel = channelMapper.getChannel(pixelID);
      if (!channelMapper.isChannelValid(m_channel)) return;

      unsigned bs = 0;
      channelMapper.splitChannelNumber(m_channel, bs, m_carrier, m_asic, m_chan);

      const auto& frontEndMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
      if (!frontEndMapper.isValid()) return;

      const auto* map = frontEndMapper.getMap(m_moduleID, bs);
      if (!map) return;
      m_scrodID = map->getScrodID();

      m_valid = true;

    }


    typedef std::multimap<double, const TOPSimHit*>::iterator Iterator;

    void TimeDigitizer::digitize(StoreArray<TOPRawDigit>& rawDigits,
                                 StoreArray<TOPDigit>& digits,
                                 short threshold,
                                 short thresholdCount,
                                 double timeJitter)
    {

      if (m_times.empty()) return;

      // get parameters of the model
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& tdc = geo->getNominalTDC();
      double hitResolveTime = tdc.getDoubleHitResolution();
      double pileupTime = tdc.getPileupTime();

      // split time pattern into multiple hits (according to double-hit resolution)
      std::vector<Iterator> ranges;
      ranges.push_back(m_times.begin());
      double prevTime = m_times.begin()->first;
      for (Iterator it = m_times.begin(); it != m_times.end(); ++it) {
        if (it->first - prevTime > hitResolveTime) ranges.push_back(it);
        prevTime = it->first;
      }
      ranges.push_back(m_times.end());

      // loop over splitted regions
      for (unsigned k = 0; k < ranges.size() - 1; k++) {

        // temporary containers
        std::vector<double> times;
        std::vector<double> weights;
        std::vector<const TOPSimHit*> simHits;

        // take only hits within pileup time and discard others
        prevTime = ranges[k]->first;
        for (Iterator it = ranges[k]; it != ranges[k + 1]; ++it) {
          if (it->first - prevTime > pileupTime) break;
          times.push_back(it->first);
          double pulseHeight = m_pulseHeightGenerator.generateWithNoise();
          weights.push_back(pulseHeight);
          simHits.push_back(it->second);
          prevTime = it->first;
        }

        // determine pulse height
        double height = 0;
        for (const auto& weight : weights) height += weight;

        // set weights
        for (auto& weight : weights) weight /= height;

        // generate pulse width
        double width = gRandom->Gaus(2.3, 0.4); // TODO: get it from ...
        if (width < 0.5) width = 0.5;

        // determine detection time
        double time = 0;
        for (unsigned j = 0; j < times.size(); j++) time += times[j] * weights[j];

        // add additional time jitter
        if (timeJitter > 0) time += gRandom->Gaus(0., timeJitter);

        // emulate feature extraction data

        double mean = time + width / 2;
        double sigma = width / 2.35482;

        int samplePeak = tdc.getSample(mean);
        if (mean - tdc.getSampleTime(samplePeak) > tdc.getSampleWidth() / 2) samplePeak++;
        short vPeak = height * gauss(tdc.getSampleTime(samplePeak), mean, sigma);
        if (vPeak < threshold) continue;
        if (vPeak > 3000) vPeak = 3000; // saturation - roughly (e.g. 4096 - pedestal)

        double halfWid = sigma * sqrt(-2 * log(0.5 * vPeak / height));
        int sampleRise = tdc.getSample(mean - halfWid);
        if (!tdc.isSampleValid(sampleRise)) continue;
        short vRise0 = height * gauss(tdc.getSampleTime(sampleRise), mean, sigma);
        short vRise1 = height * gauss(tdc.getSampleTime(sampleRise + 1), mean, sigma);

        int sampleFall = tdc.getSample(mean + halfWid);
        if (!tdc.isSampleValid(sampleFall + 1)) continue;
        short vFall0 = height * gauss(tdc.getSampleTime(sampleFall), mean, sigma);
        short vFall1 = height * gauss(tdc.getSampleTime(sampleFall + 1), mean, sigma);

        if (threshold > 0) {
          double halfWid = sigma * sqrt(-2 * log(threshold / height));
          int overThr = tdc.getSample(mean + halfWid) - tdc.getSample(mean - halfWid);
          if (overThr < thresholdCount) continue;
        }

        // determine integral
        int numSamples = (sampleFall - sampleRise) * 4; // according to topcaf
        double integral = m_pulseHeightGenerator.getIntegral(height, numSamples);

        // append new raw digit
        auto* rawDigit = rawDigits.appendNew(m_scrodID);
        rawDigit->setCarrierNumber(m_carrier);
        rawDigit->setASICNumber(m_asic);
        rawDigit->setASICChannel(m_chan);
        rawDigit->setASICWindow(m_window);
        rawDigit->setSampleRise(sampleRise);
        rawDigit->setDeltaSamplePeak(samplePeak - sampleRise);
        rawDigit->setDeltaSampleFall(sampleFall - sampleRise);
        rawDigit->setValueRise0(vRise0);
        rawDigit->setValueRise1(vRise1);
        rawDigit->setValueFall0(vFall0);
        rawDigit->setValueFall1(vFall1);
        rawDigit->setValuePeak(vPeak);
        rawDigit->setIntegral(integral);

        double cfdTime = rawDigit->getCFDLeadingTime() * tdc.getSampleWidth()
                         - tdc.getOffset();
        double cfdWidth = rawDigit->getFWHM() * tdc.getSampleWidth();
        int TDCcount = tdc.getTDCcount(cfdTime);
        unsigned tfine = TDCcount % (0x1 << tdc.getSubBits());
        rawDigit->setTFine(tfine);

        // append new digit
        auto* digit = digits.appendNew(m_moduleID, m_pixelID, TDCcount);
        digit->setTime(cfdTime);
        // digit->setTimeError(timeError); TODO!
        digit->setADC(rawDigit->getValuePeak());
        digit->setIntegral(rawDigit->getIntegral());
        digit->setPulseWidth(cfdWidth);
        digit->setChannel(m_channel);
        digit->setFirstWindow(rawDigit->getASICWindow());
        digit->addRelationTo(rawDigit);

        // set relations to simulated hits and MC particles
        for (unsigned j = 0; j < simHits.size(); j++) {
          const auto* simHit = simHits[j];
          const double& weight = weights[j];
          if (simHit) {
            digit->addRelationTo(simHit, weight);
            RelationVector<MCParticle> particles = simHit->getRelationsFrom<MCParticle>();
            for (unsigned i = 0; i < particles.size(); ++i) {
              digit->addRelationTo(particles[i], particles.weight(i) * weight);
            }
          }
        }

      } // end loop over splitted regions
    }



  } // TOP namespace
} // Belle2 namespace
