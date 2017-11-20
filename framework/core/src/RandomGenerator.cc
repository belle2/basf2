/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/core/RandomGenerator.h"
#include "framework/utilities/sha3hash/Hash.h"
#include "framework/logging/Logger.h"
#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"
#include "framework/dataobjects/FileMetaData.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace Belle2;

namespace {
  /** Convert integral type to unsigned char byte representation in big endian format.
   * @param buffer where to put the byte representation
   * @param value value to add to buffer
   */
  template<class T> void addValueToBuffer(std::vector<unsigned char>& buffer, const T& value)
  {
    static_assert(std::is_integral<T>::value, "integral type required");
    constexpr int bytes = sizeof(T);
    for (int i = bytes - 1; i >= 0; --i) {
      buffer.push_back((value >> (i * 8)) & 0xFF);
    }
  }
};

RandomGenerator::RandomGenerator(): TRandom(), m_state{0}, m_index(0), m_barrier(0),
  m_mode(c_independent)
{
  SetName("B2Random");
  SetTitle("Belle2 Random Generator");
}

void RandomGenerator::setSeed(const unsigned char* seed, unsigned int n)
{
  //Copy the seed information and set the seed length
  m_seed.resize(n);
  std::copy_n(seed, n, m_seed.data());
  //reinit the state but ignore missing EventMetaInfo
  setState(0);
}

void RandomGenerator::setState(int barrier)
{
  //Reset the internal state position
  m_index = 0;
  //Set the barrier to the requested value
  m_barrier = barrier;
  //Create a SHA-3 hash structrure
  ShakeHash hash(ShakeHash::c_SHAKE256);
  //If we have a seed info, add it to the hash
  if (m_seed.size() > 0) hash.update(m_seed.size(), m_seed.data());
  //Create a byte buffer to store event dependent data to feed to the hash
  std::vector<unsigned char> buffer;
  //estimated size of event dependent info in bytes. vector will make sure it
  //works even if this changes but we like to avoid unnecessary relocations
  buffer.reserve(28);
  //add the barrier to the buffer
  addValueToBuffer(buffer, m_barrier);
  //do we want to use EventMetaData at all?
  if (m_mode != c_independent) {
    //check if we have event data
    StoreObjPtr<EventMetaData> evt;
    if (!evt) {
      //no event data, this should not be
      B2ERROR("No EventMetaData, cannot set state of RandomGenerator from event data");
    } else {
      //ok, add event data to buffer
      addValueToBuffer(buffer, evt->getExperiment());
      addValueToBuffer(buffer, evt->getRun());
      // and if we are in run dependent mode add also event number to the hash
      if (m_mode == c_eventDependent) {
        addValueToBuffer(buffer, evt->getEvent());
      }
    }
  }

  hash.update(buffer.size(), buffer.data());
  //Extract 1024bit hash from the hash structure and write it into the
  //internal state
  hash.getHash(sizeof(m_state), (unsigned char*) m_state);

  //Only prepare debugoutput if we actually want to show it. This is almost
  //equivalent to B2DEBUG(200, ...); but we need to loop over states for
  //printing so we could not put it in a normal B2DEBUG statement easily.
#ifndef LOG_NO_B2DEBUG
  if (Belle2::LogSystem::Instance().isLevelEnabled(Belle2::LogConfig::c_Debug, 200, PACKAGENAME())) {
    std::stringstream info;
    info << "Random Generator State info:\n";
    info << "  seed (" << std::dec << m_seed.size() << "):\n    ";
    for (auto c : m_seed) { info << std::setw(2) << std::setfill('0') << std::hex << (int)c << " "; }
    info << "\n  event info (mode=" << m_mode << "): \n";
    info << "    barrier:" << std::dec << m_barrier;
    if (m_mode != c_independent) {
      StoreObjPtr<EventMetaData> evt;
      info << " EXP:" << evt->getExperiment() << " RUN:" << evt->getRun();
      if (m_mode == c_eventDependent) {
        info << " EVT:" << evt->getEvent();
      }
    }
    info << "\n  event bytes (" << std::dec << buffer.size() << "):\n    ";
    for (auto c : buffer) { info << std::setw(2) << std::setfill('0') << std::hex << (int)c << " "; }
    info << "\n  state (index=" << m_index << "): ";
    for (int i = 0; i < 16; ++i) {
      info << ((i % 4 == 0) ? "\n    " : " ") << std::setw(16) << std::setfill('0') << std::hex << m_state[i];
    }
    _B2LOGMESSAGE(Belle2::LogConfig::c_Debug, 200, info.str(), PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__);
  }
#endif
}

void RandomGenerator::RndmArray(Int_t n, unsigned char* array)
{
  //First we fill the array using 64bit blocks
  RndmArray(n / sizeof(ULong64_t), (ULong64_t*)array);
  const Int_t remainder = n % sizeof(ULong64_t);
  //If the size is not divisible by 8 we fill the remainder from one additional
  //random value
  if (remainder) {
    const ULong64_t r = random64();
    std::copy_n((unsigned char*)&r, remainder, array + (n - remainder - 1));
  }
}
