/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDigitMerging/SVDCheckDigitsModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <cassert>
#include <gtest/gtest.h>

#include <string>
#include <sstream>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


#define assert_float(A,B) if(!isClose((A),(B)))\
    B2FATAL("Assertion failed: " << #A << " (" << (A) << ") != " << #B << " (" << (B) << ")");
#define assert_eq(A,B) if((A)!=(B))\
    B2FATAL("Assertion failed: " << #A << " (" << (A) << ") != " << #B << " (" << (B) << ")");
namespace {
  /** Small helper function to compare two doubles with a given precision */
  bool isClose(double a, double b, double epsilon = 1e-6)
  {
    return a == b || fabs(a - b) < epsilon || (fabs(a / b) - 1.0) < epsilon;
  }

  /** Helper class to summarize and compare relations
   * The class is a map that add weights on insertion of an existing key.*/
  class WeightedSet {
  public:
    /** Add (index, weight) pair to the map.
     * @param index index of the object in the relation
     * @param weight weight of objecct in the relation
     */
    void add(size_t index, double weight)
    {
      auto findresult = m_data.find(index);
      if (findresult != m_data.end())
        findresult->second = weight;
      else
        m_data.insert(make_pair(index, weight));
    }
    /** Return map */
    const map<size_t, double>& get_data() const { return m_data; }
    /** Compare with another weighted set object, by size and element-by-element */
    void is_equal_to(const WeightedSet& other)
    {
      // The sets should be equal
      const auto& other_data = other.get_data();
      assert_eq(m_data.size(), other_data.size());
      for (decltype(other_data.begin()) i1 = m_data.begin(), i2 = other_data.begin();
           i1 != m_data.end() && i2 != m_data.end();
           ++i1, ++i2) {
        assert_eq(i1->first, i2->first);
        assert_float(i1->second, i2->second);
      }
    }
    /** Display data
     */
    string print() const
    {
      ostringstream os;
      os << "{";
      for (const auto& item : m_data)
        os << "(" << item.first << ":" << item.second << "),";
      os << "}" << endl;
      return os.str();
    }
  private:
    std::map<size_t, double> m_data;
  }; // class WeightedSet
} // end anonymous namespace

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCheckDigits);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCheckDigitsModule::SVDCheckDigitsModule() : Module()
{
  //Set module properties
  setDescription(
    "This modules compares sets of SVDShaperDigits and SVDDigits to make sure "
    "they are eqivalent. Intended to check conversion of SVDDigits to "
    "SVDShaperDigits or back, including relations."
  );
  setPropertyFlags(c_ParallelProcessingCertified);

  // Collections
  addParam("Digits", m_storeDigitsName,
           "SVDDigits collection name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name", string(""));
}

void SVDCheckDigitsModule::initialize()
{
  //Mark all StoreArrays as required
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  StoreArray<MCParticle> storeMCParticles;
  StoreArray<SVDTrueHit> storeTrueHits;
  storeMCParticles.isRequired();
  storeDigits.isRequired();
  storeShaperDigits.isRequired();
  storeTrueHits.isRequired();

  //And also all relations
  RelationArray relDigitsMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitsTrueHits(storeDigits, storeTrueHits);
  relDigitsMCParticles.isRequired();
  relDigitsTrueHits.isRequired();

  RelationArray relShaperDigitsMCParticles(storeShaperDigits, storeMCParticles);
  RelationArray relShaperDigitsTrueHits(storeShaperDigits, storeTrueHits);
  RelationArray relShaperDigitsDigits(storeShaperDigits, storeDigits);
  relShaperDigitsMCParticles.isRequired();
  relShaperDigitsTrueHits.isRequired();
  relShaperDigitsDigits.isRequired();

  m_storeDigitsName = storeDigits.getName();
  m_storeShaperDigitsName = storeShaperDigits.getName();
}

void SVDCheckDigitsModule::event()
{
  //Obtain all StoreArrays
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  const StoreArray<MCParticle> storeMCParticles;
  const StoreArray<SVDTrueHit> storeTrueHits;

  //Number of 6-Digits
  const size_t nShaperDigits = storeShaperDigits.getEntries();

  for (unsigned int iShaperDigit = 0; iShaperDigit < nShaperDigits; ++iShaperDigit) {
    // Loop through 6-digits, get the original 1-digits and compre digit data
    // and relations. We don't care about small deviations, so we use loose comparison
    // rather than ==.
    const SVDShaperDigit& shaperDigit = *storeShaperDigits[iShaperDigit];
    B2DEBUG(99, shaperDigit.toString());
    const RelationVector<SVDDigit>& onedigits =
      shaperDigit.getRelationsTo<SVDDigit>(storeDigits.getName());
    // Number of related 1-digits should equal the number of non-zero samples in the 6-digit.
    const SVDShaperDigit::APVFloatSamples& sixSamples = shaperDigit.getSamples();
    size_t nSixSamples = count_if(sixSamples.begin(), sixSamples.end(),
                                  [](float x)->bool {return x > -1000; });
    assert_eq(nSixSamples, onedigits.size());
    for (const SVDDigit& digit : onedigits) {
      B2DEBUG(99, digit.toString());
      assert_eq(digit.getSensorID(), shaperDigit.getSensorID().getID());
      assert_eq(digit.isUStrip(), shaperDigit.isUStrip());
      assert_eq(digit.getCellID(), shaperDigit.getCellID());
      assert_float(digit.getCharge(), sixSamples[digit.getIndex()]);
    }
    // 6-to-1 relation weights are equal to digit signals, modulo rounding error
    for (size_t i = 0; i < onedigits.size(); ++i)
      assert_float(onedigits[i]->getCharge(), onedigits.weight(i));

    // Digit-to-SVDTrueHits relations
    const RelationVector<SVDTrueHit>& sixTrueHitRelVec =
      shaperDigit.getRelationsTo<SVDTrueHit>(storeTrueHits.getName());
    WeightedSet sixTrueHitRels;
    for (size_t i = 0; i < sixTrueHitRelVec.size(); ++i)
      sixTrueHitRels.add(sixTrueHitRelVec[i]->getArrayIndex(), sixTrueHitRelVec.weight(i));
    B2DEBUG(85, "6-digits to TrueHits: " << sixTrueHitRels.print());

    WeightedSet oneTrueHitRels;
    for (const SVDDigit& digit : onedigits) {
      const RelationVector<SVDTrueHit>& oneTrueHitRelVec =
        digit.getRelationsTo<SVDTrueHit>(storeTrueHits.getName());
      for (size_t i = 0; i < oneTrueHitRelVec.size(); ++i)
        oneTrueHitRels.add(oneTrueHitRelVec[i]->getArrayIndex(), oneTrueHitRelVec.weight(i));
    }
    B2DEBUG(85, "1-digits to TrueHits: " << oneTrueHitRels.print());
    oneTrueHitRels.is_equal_to(sixTrueHitRels);

    // Digit-to-MCParticles relations
    const RelationVector<MCParticle>& sixMCParticleRelVec =
      shaperDigit.getRelationsTo<MCParticle>(storeMCParticles.getName());
    WeightedSet sixMCParticleRels;
    for (size_t i = 0; i < sixMCParticleRelVec.size(); ++i)
      sixMCParticleRels.add(sixMCParticleRelVec[i]->getArrayIndex(), sixMCParticleRelVec.weight(i));
    B2DEBUG(85, "6-digits to MCParticles: " << sixMCParticleRels.print());

    WeightedSet oneMCParticleRels;
    for (const SVDDigit& digit : onedigits) {
      const RelationVector<MCParticle>& oneMCParticleRelVec =
        digit.getRelationsTo<MCParticle>(storeMCParticles.getName());
      for (size_t i = 0; i < oneMCParticleRelVec.size(); ++i)
        oneMCParticleRels.add(
          oneMCParticleRelVec[i]->getArrayIndex(), oneMCParticleRelVec.weight(i)
        );
    }
    B2DEBUG(85, "1-digits to MCParticles: " << oneMCParticleRels.print());
    oneMCParticleRels.is_equal_to(sixMCParticleRels);

  } // for 6-digits

} // event()
