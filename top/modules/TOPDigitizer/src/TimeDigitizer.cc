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

    TimeDigitizer::TimeDigitizer(int moduleID, int pixelID):
      m_moduleID(moduleID), m_pixelID(pixelID)
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

      m_window = int(gRandom->Rndm() * 512);
      m_valid = true;

    }


    typedef std::multimap<double, const TOPSimHit*>::iterator Iterator;

    void TimeDigitizer::digitize(StoreArray<TOPDigit>& digits, double sigma)
    {

      if (m_times.empty()) return;

      // get parameters of the model
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& tdc = geo->getNominalTDC();
      double hitResolveTime = tdc.getDoubleHitResolution();
      double pileupTime = tdc.getPileupTime();
      int overflow = tdc.getOverflowValue();

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
          double pulseHeight = 50.0 + gRandom->Exp(70.0); // TODO: get it from ...
          weights.push_back(pulseHeight);
          simHits.push_back(it->second);
          prevTime = it->first;
        }

        // determine pulse height and set weights
        double pulseHeight = 0;
        for (const auto& weight : weights) pulseHeight += weight;
        if (pulseHeight > 0) {
          for (auto& weight : weights) weight /= pulseHeight;
        } else {
          B2ERROR("Generated pulse height is not positive - no digitization done");
          return;
        }

        // determine detection time
        double time = 0;
        for (unsigned j = 0; j < times.size(); j++) time += times[j] * weights[j];

        // add additional time jitter
        if (sigma > 0) time += gRandom->Gaus(0., sigma);

        // digitize detection time
        int TDCcount = tdc.getTDCcount(time);
        if (TDCcount == overflow) continue;

        if (pulseHeight > 2000) pulseHeight = 2000;
        double pulseWidth = gRandom->Gaus(2.3, 0.4); // TODO: get it from ...
        if (pulseWidth < 1.0) pulseWidth = 1.0;
        double integral = pulseHeight * pulseWidth * 2.9; // TODO: get it from ...

        // append new digit
        TOPDigit* digit = digits.appendNew(m_moduleID, m_pixelID, TDCcount);
        digit->setTime(time);
        digit->setADC(int(pulseHeight));
        digit->setIntegral(int(integral));
        digit->setPulseWidth(pulseWidth);
        digit->setChannel(m_channel);

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



    void TimeDigitizer::digitize(StoreArray<TOPRawDigit>& rawDigits,
                                 StoreArray<TOPDigit>& digits,
                                 double sigma)
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
          double pulseHeight = 50.0 + gRandom->Exp(70.0); // TODO: get it from ...
          weights.push_back(pulseHeight);
          simHits.push_back(it->second);
          prevTime = it->first;
        }

        // determine pulse height and set weights
        double height = 0;
        for (const auto& weight : weights) height += weight;
        if (height > 0) {
          for (auto& weight : weights) weight /= height;
        } else {
          B2ERROR("Generated pulse height is not positive - no digitization done");
          return;
        }
        if (height > 2000) height = 2000; // TODO: get cut value from ...

        // generate pulse width and determine integral
        double width = gRandom->Gaus(2.3, 0.4); // TODO: get it from ...
        if (width < 1.0) width = 1.0;
        int integral = height * width * 2.9; // TODO: get it from ...

        // determine detection time
        double time = 0;
        for (unsigned j = 0; j < times.size(); j++) time += times[j] * weights[j];

        // add additional time jitter
        if (sigma > 0) time += gRandom->Gaus(0., sigma);

        // emulate feature extraction data

        double mean = time + width / 2;
        double sigma = width / 2.35482;

        int sampleRise = tdc.getSample(time);
        if (!tdc.isSampleValid(sampleRise)) continue;
        short vRise0 = height * gauss(tdc.getSampleTime(sampleRise), mean, sigma);
        short vRise1 = height * gauss(tdc.getSampleTime(sampleRise + 1), mean, sigma);

        int sampleFall = tdc.getSample(time + width);
        if (!tdc.isSampleValid(sampleFall + 1)) continue;
        short vFall0 = height * gauss(tdc.getSampleTime(sampleFall), mean, sigma);
        short vFall1 = height * gauss(tdc.getSampleTime(sampleFall + 1), mean, sigma);

        int samplePeak = tdc.getSample(mean);
        short vPeak = height * gauss(tdc.getSampleTime(samplePeak), mean, sigma);
        short vPeak1 = height * gauss(tdc.getSampleTime(samplePeak + 1), mean, sigma);
        if (vPeak1 > vPeak) {
          vPeak = vPeak1;
          samplePeak++;
        }

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
