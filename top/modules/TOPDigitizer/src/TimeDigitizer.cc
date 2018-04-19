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
                                 unsigned storageDepth, double rmsNoise,
                                 const TOPSampleTimes& sampleTimes):
      m_moduleID(moduleID), m_pixelID(pixelID), m_window(window),
      m_storageDepth(storageDepth), m_rmsNoise(rmsNoise), m_sampleTimes(&sampleTimes)
    {
      const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();
      if (!channelMapper.isValid()) {
        B2ERROR("TimeDigitizer::TimeDigitizer: no valid channel mapper found");
        return;
      }

      m_channel = channelMapper.getChannel(pixelID);
      if (!channelMapper.isChannelValid(m_channel)) {
        B2ERROR("TimeDigitizer::TimeDigitizer: invalid channel");
        return;
      }

      unsigned bs = 0;
      channelMapper.splitChannelNumber(m_channel, bs, m_carrier, m_asic, m_chan);

      const auto& frontEndMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
      if (!frontEndMapper.isValid()) {
        B2ERROR("TimeDigitizer::TimeDigitizer: no valid frontend mapper found");
        return;
      }

      const auto* map = frontEndMapper.getMap(m_moduleID, bs);
      if (!map) {
        B2ERROR("TimeDigitizer::TimeDigitizer: no valid frontend map found");
        return;
      }

      m_scrodID = map->getScrodID();
      m_valid = true;

    }

    void TimeDigitizer::addTimeOfHit(double t, double pulseHeight, EType type,
                                     const TOPSimHit* simHit)
    {
      Hit hit;
      hit.pulseHeight = pulseHeight;
      hit.type = type;
      hit.simHit = simHit;
      switch (type) {
        case static_cast<int>(c_Hit):
          hit.shape = &(TOPGeometryPar::Instance()->getGeometry()->getSignalShape());
          break;
        case static_cast<int>(c_ChargeShare):
          hit.shape = &(TOPGeometryPar::Instance()->getGeometry()->getSignalShape());
          break;
        case static_cast<int>(c_CrossTalk):
          hit.shape = 0;
          B2ERROR("TOP::TimeDigitizer: waveform shape of cross-talk not yet available");
          break;
        case static_cast<int>(c_CalPulse):
          hit.shape = &(TOPGeometryPar::Instance()->getGeometry()->getCalPulseShape());
          break;
        default:
          hit.shape = 0;
      }
      m_times.insert(std::pair<double, const Hit>(t, hit));
    }

    //-------- simplified pile-up and double-hit-resolution model ------- //
    // this function will probably be removed in the future, therefore I don't
    // care about some hard-coded values

    typedef std::multimap<double, const TimeDigitizer::Hit>::const_iterator Iterator;

    void TimeDigitizer::digitize(StoreArray<TOPRawDigit>& rawDigits,
                                 StoreArray<TOPDigit>& digits,
                                 int threshold,
                                 int thresholdCount,
                                 double timeJitter) const
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
          double pulseHeight = gRandom->Gaus(it->second.pulseHeight, m_rmsNoise);
          weights.push_back(pulseHeight);
          simHits.push_back(it->second.simHit);
          prevTime = it->first;
        }

        // determine pulse height
        double height = 0;
        for (const auto& weight : weights) height += weight;

        // set weights
        for (auto& weight : weights) weight /= height;

        // generate pulse width
        double width = gRandom->Gaus(2.3, 0.4);
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
          double halfDt = sigma * sqrt(-2 * log(threshold / height));
          int overThr = tdc.getSample(mean + halfDt) - tdc.getSample(mean - halfDt);
          if (overThr < thresholdCount) continue;
        }

        // determine integral
        int numSamples = (sampleFall - sampleRise) * 4; // according to topcaf
        double sigmaIntegral = 0;
        if (numSamples > 1) sigmaIntegral = m_rmsNoise * sqrt(numSamples - 1);
        double integral = gRandom->Gaus(height * 7.0, sigmaIntegral);

        // append new raw digit
        auto* rawDigit = rawDigits.appendNew(m_scrodID, TOPRawDigit::c_MC);
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

        double rawTime = rawDigit->getCFDLeadingTime();
        double cfdTime = rawTime * tdc.getSampleWidth() - tdc.getOffset();
        double cfdWidth = rawDigit->getFWHM() * tdc.getSampleWidth();
        int sampleDivisions = 0x1 << tdc.getSubBits();
        unsigned tfine = int(rawTime * sampleDivisions) % sampleDivisions;
        rawDigit->setTFine(tfine);

        // append new digit

        auto* digit = digits.appendNew(m_moduleID, m_pixelID, rawTime);
        digit->setTime(cfdTime);
        digit->setTimeError(timeJitter);
        digit->setPulseHeight(rawDigit->getValuePeak());
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


    // -------- full waveform digitization ------- //

    void TimeDigitizer::digitize(StoreArray<TOPRawWaveform>& waveforms,
                                 StoreArray<TOPRawDigit>& rawDigits,
                                 StoreArray<TOPDigit>& digits,
                                 int threshold,
                                 int hysteresis,
                                 int thresholdCount) const
    {

      // get parameters of the model

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& tdc = geo->getNominalTDC();

      // generate waveform

      std::vector<unsigned short> windowNumbers;
      windowNumbers.push_back(m_window);
      for (unsigned i = 1; i < tdc.getNumWindows(); i++) {
        windowNumbers.push_back((windowNumbers.back() + 1) % m_storageDepth);
      }
      std::vector<double> baselines(windowNumbers.size(), 0);
      std::vector<double> rmsNoises(windowNumbers.size(), m_rmsNoise);
      double averagePedestal = tdc.getAveragePedestal();
      std::vector<double> pedestals(windowNumbers.size(), averagePedestal);
      int adcRange = tdc.getADCRange();

      auto wfData = generateWaveform(baselines, rmsNoises, pedestals, adcRange);

      // store waveform

      auto* waveform = waveforms.appendNew(m_moduleID, m_pixelID, m_channel, m_scrodID,
                                           m_window, 0, wfData);
      waveform->setStorageWindows(windowNumbers);
      waveform->setPedestalSubtractedFlag(true);

      // do feature extraction

      waveform->featureExtraction(threshold, hysteresis, thresholdCount);

      // digits

      for (const auto& feature : waveform->getFeatureExtractionData()) {

        // append new raw digit and set it
        auto* rawDigit = rawDigits.appendNew(m_scrodID, TOPRawDigit::c_MC);
        rawDigit->setCarrierNumber(m_carrier);
        rawDigit->setASICNumber(m_asic);
        rawDigit->setASICChannel(m_chan);
        rawDigit->setASICWindow(m_window);
        rawDigit->setStorageWindows(windowNumbers);
        rawDigit->setSampleRise(feature.sampleRise);
        rawDigit->setDeltaSamplePeak(feature.samplePeak - feature.sampleRise);
        rawDigit->setDeltaSampleFall(feature.sampleFall - feature.sampleRise);
        rawDigit->setValueRise0(feature.vRise0);
        rawDigit->setValueRise1(feature.vRise1);
        rawDigit->setValueFall0(feature.vFall0);
        rawDigit->setValueFall1(feature.vFall1);
        rawDigit->setValuePeak(feature.vPeak);
        rawDigit->setIntegral(feature.integral);
        double rawTime = rawDigit->getCFDLeadingTime(); // time in [samples]
        double rawTimeErr = rawDigit->getCFDLeadingTimeError(m_rmsNoise); // in [samples]
        int sampleDivisions = 0x1 << tdc.getSubBits();
        unsigned tfine = int(rawTime * sampleDivisions) % sampleDivisions; // TODO rawTime<0 ?
        rawDigit->setTFine(tfine);
        rawDigit->addRelationTo(waveform);

        // convert to [ns] using time base calibration
        double cfdTime = m_sampleTimes->getTime(m_window, rawTime) - tdc.getOffset();
        double width = m_sampleTimes->getDeltaTime(m_window,
                                                   rawDigit->getCFDFallingTime(),
                                                   rawDigit->getCFDLeadingTime());
        double timeError = rawTimeErr * m_sampleTimes->getTimeBin(m_window,
                                                                  feature.sampleRise);

        // append new digit and set it
        auto* digit = digits.appendNew(m_moduleID, m_pixelID, rawTime);
        digit->setTime(cfdTime);
        digit->setTimeError(timeError);
        digit->setPulseHeight(rawDigit->getValuePeak());
        digit->setIntegral(rawDigit->getIntegral());
        digit->setPulseWidth(width);
        digit->setChannel(m_channel);
        digit->setFirstWindow(rawDigit->getASICWindow());
        digit->addRelationTo(rawDigit);

        // set relations to simulated hits and MC particles, largest weight first

        std::multimap<double, const Hit*, std::greater<double>> weights;
        for (const auto& hit : m_times) {
          double hitTime = hit.first;
          double weight = 0;
          const auto* pulseShape = hit.second.shape;
          if (pulseShape) {
            weight = fabs(pulseShape->getValue(cfdTime - hitTime));
          }
          if (weight > 0.01) {
            weight *= hit.second.pulseHeight;
            weights.insert(std::pair<double, const Hit*>(weight, &hit.second));
          }
        }
        double sum = 0;
        for (const auto& w : weights) sum += w.first;
        if (sum == 0) continue; // noisy hit
        for (const auto& w : weights) {
          auto weight = w.first / sum;
          const auto* simHit = w.second->simHit;
          if (simHit and weight > 0) {
            digit->addRelationTo(simHit, weight);
            RelationVector<MCParticle> particles = simHit->getRelationsFrom<MCParticle>();
            for (unsigned i = 0; i < particles.size(); ++i) {
              digit->addRelationTo(particles[i], particles.weight(i) * weight);
            }
          } else if (w.second->type == c_CalPulse and weight > 0.90) {
            digit->setHitQuality(TOPDigit::c_CalPulse);
          }
        }
      }

    }


    vector<short> TimeDigitizer::generateWaveform(const vector<double>& baselines,
                                                  const vector<double>& rmsNoises,
                                                  const vector<double>& pedestals,
                                                  int adcRange) const
    {

      if (baselines.empty() or baselines.size() != rmsNoises.size() or
          baselines.size() != pedestals.size())
        B2FATAL("TOP::TimeDigitizer: inconsistent vector sizes");

      // model parameters

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& tdc = geo->getNominalTDC();
      const auto& signalShape = geo->getSignalShape();

      double dt = tdc.getSampleWidth();
      double fmax = 1 / (2 * dt); // Nyquist frequency
      double tau1 = 1 / (2 * M_PI * signalShape.getPole1());
      double tau2 = 1 / (2 * M_PI * signalShape.getPole2());
      double bw1 = 1 / (4 * tau1);        // bandwidth of first order filter
      double bw2 = 1 / (4 * (tau1 + tau2)); // bandwidth of second order filter

      // construct waveform vector containing white noise

      vector<double> waveform;
      for (auto rms : rmsNoises) {
        double rms0 = rms * sqrt(fmax / bw2);
        for (unsigned i = 0; i < TOPRawWaveform::c_WindowSize; i++) {
          waveform.push_back(gRandom->Gaus(0, rms0));
        }
      }

      // noise smoothing according to second order low pass filter

      double a1 = exp(-dt / tau1);
      double v0 = gRandom->Gaus(0, rmsNoises[0] * sqrt(bw1 / bw2));
      double prevValue = v0;
      for (auto& value : waveform) { // first order filter (pole1)
        value = a1 * prevValue + (1 - a1) * value;
        prevValue = value;
      }
      double a2 = exp(-dt / tau2);
      prevValue = v0;
      for (auto& value : waveform) { // first order filter again (pole2)
        value = a2 * prevValue + (1 - a2) * value;
        prevValue = value;
      }

      // add possible baseline shifts

      int k = 0;
      for (auto baseline : baselines) {
        for (unsigned i = 0; i < TOPRawWaveform::c_WindowSize; i++) {
          waveform[k] += baseline;
          k++;
        }
      }

      // add signal

      for (const auto& hit : m_times) {
        double hitTime = hit.first;
        double pulseHeight = hit.second.pulseHeight;
        const auto* pulseShape = hit.second.shape;
        if (!pulseShape) continue;
        for (unsigned sample = 0; sample < waveform.size(); sample++) {
          double t = m_sampleTimes->getTime(m_window, sample) - tdc.getOffset();
          waveform[sample] += pulseHeight * pulseShape->getValue(t - hitTime);
        }
      }

      // model saturation effects and convert to short
      // (exact modelling would require pedestals for each sample)

      vector<short> wf;
      k = 0;
      for (auto pedestal : pedestals) {
        for (unsigned i = 0; i < TOPRawWaveform::c_WindowSize; i++) {
          int adc = waveform[k] + pedestal;
          if (adc < 0) adc = 0;
          if (adc > adcRange) adc = adcRange;
          adc -= pedestal;
          wf.push_back(adc);
          k++;
        }
      }

      return wf;
    }



  } // TOP namespace
} // Belle2 namespace
