/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDDigitSorterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/reconstruction/Sample.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <set>
#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitSorter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitSorterModule::SVDDigitSorterModule() : Module()
{
  //Set module properties
  setDescription("This module sorts and filters the existing SVDDigits collection and also "
                 "updates the corresponding Relation to MCParticles and TrueHits. This is "
                 "needed for unsorted strip data as the Clusterizer expects sorted input."
                 "The module is generally not required for simulation, as SVDDigitzer outputs"
                 "a properly sorted digit collection.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("mergeDuplicates", m_mergeDuplicates,
           "If true, merge Sample information if more than one digit exists for the same address", true);
  addParam("minSamples", m_minSamples,
           "Minimum number of consecutive samples to accept a strip signal. 1-6, default 3, 1 means filtering disabled.", 3);
  addParam("RejectionThreshold", m_rejectionThreshold,
           "Threshold that at least minSamples consecutive samples must exceed for a strip to pass", 3.0);

  addParam("ignoredStripsListName", m_ignoredStripsListName, "Name of the xml with ignored strips list", string(""));
  addParam("digits", m_storeDigitsName, "SVDDigit collection name", string(""));
  addParam("truehits", m_storeTrueHitsName, "SVDTrueHit collection name", string(""));
  addParam("particles", m_storeMCParticlesName, "MCParticle collection name", string(""));
  addParam("digitsToTrueHits", m_relDigitTrueHitName, "Digits to TrueHit relation name",
           string(""));
  addParam("digitsToMCParticles", m_relDigitMCParticleName, "Digits to MCParticles relation name",
           string(""));
}

void SVDDigitSorterModule::initialize()
{
  //Register collections
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeDigits.isRequired();
  storeMCParticles.isOptional();
  storeTrueHits.isOptional();

  RelationArray relDigitMCParticles(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  relDigitMCParticles.isOptional();
  relDigitTrueHits.isOptional();

  m_storeDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relDigitTrueHitName = relDigitTrueHits.getName();
  m_relDigitMCParticleName = relDigitMCParticles.getName();

  m_ignoredStripsList = unique_ptr<SVDIgnoredStripsMap>(new SVDIgnoredStripsMap(m_ignoredStripsListName));
}

void SVDDigitSorterModule::event()
{
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  // If not digits, nothing to do
  if (!storeDigits || !storeDigits.getEntries()) return;
  B2DEBUG(89, "Initial size of StoreDigits array: " << storeDigits.getEntries());

  RelationArray relDigitMCParticle(m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(m_relDigitTrueHitName);

  //List of mappings from old indices to new indices
  std::vector<RelationArray::consolidation_type> relationIndices(storeDigits.getEntries());
  //Mapping of Sample information to sort according to VxdID, row, column
  std::map<VxdID, std::multiset<Sample>> sensors;
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // Fill sensor information to get sorted SVDDigit indices
  const int nSamples = storeDigits.getEntries();
  for (int i = 0; i < nSamples; i++) {
    const SVDDigit* const digit = storeDigits[i];
    Sample sample(digit, i);
    VxdID sensorID = digit->getSensorID();
    if (!geo.validSensorID(sensorID)) {
      B2WARNING("Malformed VXDID in SVDDigit: " << sensorID.getLayerNumber() << "/" << sensorID.getLadderNumber() << "/" <<
                sensorID.getSensorNumber() << "/" << sensorID.getSegmentNumber() << ". Discarding.");
      continue;
    }
    // Re-use segment part of VXDID to distinguish u and v strips.
    // u-strps will have segment number 1
    if (digit->isUStrip()) sensorID.setSegmentNumber(1);
    // filter out digits from ignored strips
    if (m_ignoredStripsList->stripOK(sensorID, digit->getCellID())) {
      sensors[sensorID].insert(sample);
    }
  }


  if (m_minSamples > 1)
    for (auto& vxd_samples : sensors) {
      // vxd_samples.second is a multiset, remove all samples whose strip number occurs less than m_minSamples times
      // Fist geometry data for the sensor
      VxdID sensorID = vxd_samples.first;
      // If malformed object, drop it. Only here, for sorted digits the check will be done in sorter module.
      if (!geo.validSensorID(sensorID)) {
        B2WARNING("Sensor with malformed VxdID " << sensorID.getID() << ", dropping.");
        continue;
      }
      const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
      bool isU = (sensorID.getSegmentNumber() == 0);
      double ePerADU = isU ? info.getAduEquivalentU() : info.getAduEquivalentV();
      double elNoise = (sensorID.getSegmentNumber() == 0) ? info.getElectronicNoiseU() : info.getElectronicNoiseV();
      double threshold = m_rejectionThreshold * elNoise / ePerADU;

      decltype(vxd_samples.second)& samples = vxd_samples.second;
      unsigned int currentStrip = 5000;
      int currentSampleIndex = 0;
      int currentCount = 0;
      auto currentStart = samples.begin();
      auto firstSample = samples.cbegin();
      bool pass = false;
      B2DEBUG(99, "Initial size: " << vxd_samples.second.size());
      for (auto sensor_it = firstSample; sensor_it != samples.end(); sensor_it++) {
        if (sensor_it->getCellID() != currentStrip) {
          // Count samples and erase if small, reset counters.
          if (!pass) {
            samples.erase(currentStart, sensor_it);
            if (sensor_it != firstSample) B2DEBUG(89, "ERASED.");
          }
          currentStart = sensor_it;;
          currentCount = 0;
          currentStrip = sensor_it->getCellID();
          currentSampleIndex = 0;
          pass = false;
        }
        // We need consecutive samples, so reset index if a sample is missing or a sample is small
        if (
          (sensor_it->getSampleIndex() == currentSampleIndex)
          &&
          (sensor_it->getCharge() > threshold)
        ) currentCount++;
        else
          currentCount = 0;
        currentSampleIndex = sensor_it->getSampleIndex() + 1;
        if (currentCount >= m_minSamples) pass = true;
        B2DEBUG(99,
                "VXD ID: " << vxd_samples.first <<
                " cut: " << threshold <<
                " strip: " << currentStrip <<
                " sample: " << currentSampleIndex <<
                " count: " << currentCount <<
                " charge: " << sensor_it->getCharge() <<
                " pass: " << pass << endl;
               );
      }
      // Finish the last strip
      if (!pass) {
        samples.erase(currentStart, samples.end());
        B2DEBUG(89, "ERASED.");
      }
      B2DEBUG(99, "Final size: " << vxd_samples.second.size());
    } // for vxd_samples

  // Now we loop over sensors and reorder the digits list
  // To do this, we create a copy of the existing digits
  m_digitcopy.clear();
  m_digitcopy.insert(end(m_digitcopy), begin(storeDigits), end(storeDigits));
  // and a vector to remember which index changed into what
  unsigned int index(0);
  // And just loop over the sensors and assign the digits at the correct position
  for (const auto& sensor : sensors) {
    B2DEBUG(99, "Sample size: " << sensor.second.size());
    const SVD::Sample* lastsample(0);
    for (const SVD::Sample& sample : sensor.second) {
      //Normal case: strip has different address
      if (!lastsample || sample > *lastsample) {
        //Overwrite the digit
        *storeDigits[index] = m_digitcopy[sample.getArrayIndex()];
        //Remember old and new index
        relationIndices[sample.getArrayIndex()] = std::make_pair(index, false);
        //increment next index
        ++index;
      } else {
        //We already have a pixel at this address, see if we merge or drop the new one
        if (m_mergeDuplicates) {
          //Merge the two pixels. As the SVDDigit does not have setters we have to create a new object.
          const SVDDigit& old = *storeDigits[index - 1];
          *storeDigits[index - 1] = SVDDigit(
                                      old.getSensorID(),
                                      old.isUStrip(),
                                      old.getCellID(),
                                      old.getCellPosition(),
                                      old.getCharge() + m_digitcopy[sample.getArrayIndex()].getCharge(), old.getIndex()
                                    );
          relationIndices[sample.getArrayIndex()] = std::make_pair(index - 1, false);
        } else {
          //Otherwise delete the second pixel by omitting it here and removing relation elements on consolidation
          B2INFO("Dropped duplicate digit: " << sensor.first << ", strip " << sample.getCellID() << " sample " << sample.getSampleIndex());
          relationIndices[sample.getArrayIndex()] = std::make_pair(0, true);
        }
      }
      lastsample = &sample;
    }
  }

  //Resize if we omitted/merged one or more elements
  storeDigits.getPtr()->ExpandCreate(index);

  // Finally we just need to reorder the RelationArrays and delete elements
  // referencing pixels which were ignored because that adress was set more
  // than once
  RelationArray::ReplaceVec<> from(relationIndices);
  RelationArray::Identity to;
  if (relDigitMCParticle) relDigitMCParticle.consolidate(from, to, RelationArray::c_deleteElement);
  if (relDigitTrueHit) relDigitTrueHit.consolidate(from, to, RelationArray::c_deleteElement);
  B2DEBUG(89, "Final size of StoreDigits store array: " << storeDigits.getEntries());

}
