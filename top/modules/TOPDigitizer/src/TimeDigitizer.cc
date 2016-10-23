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

      // get channel mapper
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();

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
        TOPDigit* digit = digits.appendNew(m_barID, m_pixelID, TDCcount);
        digit->setTime(time);
        digit->setADC(int(pulseHeight));
        digit->setIntegral(int(integral));
        digit->setPulseWidth(pulseWidth);
        digit->setChannel(channelMapper.getChannel(m_pixelID));

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
