/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

/** forward declared memory structure for Keccak Hash functions */
struct KeccakHashInstance;

namespace Belle2 {
  /** Simple interface to calculate SHA3 hash sum (FIPS 202 draft) with fixed
   * size from given data inputs */
  class SHA3Hash {
  public:
    /** Available hash modes according to FIPS 202 draft */
    enum EHashMode {
      c_SHA3_224 = 224 / 8, /** 224bit output size */
      c_SHA3_256 = 256 / 8, /** 256bit output size */
      c_SHA3_384 = 384 / 8, /** 384bit output size */
      c_SHA3_512 = 512 / 8 /** 512bit output size */
    };
    /** Constructor initializing the hash structure with a given output size */
    explicit SHA3Hash(EHashMode length);
    /** No copying */
    SHA3Hash(const SHA3Hash&) = delete;
    /** destructor freeing the memory */
    ~SHA3Hash();
    /** reinit the hash structure to create a new hash sum */
    void clear();
    /** update the internal state by adding n bytes of data from buff */
    void update(int n, unsigned char* buff);
    /** obtain the hash value into buff. buff must be large enough for the
     * chosen hash size. The values of EHashLength correspond to the number
     * of required bytes, e.g.  unsigned char hashValue[c_SHA3_512];
     */
    void getHash(unsigned char* buff);
    /** obtain the hash value as a vector of unsigned char */
    std::vector<unsigned char> getHash();
  private:
    /** memory structure to calculate the hash value */
    KeccakHashInstance* m_instance;
    /** chosen hash mode */
    EHashMode m_mode;
  };

  /** Simple interface to calculate SHAKE256 hash sum (FIPS 202 draft) with
   * variable size from given data inputs */
  class ShakeHash {
  public:
    /** Available hash modes according to FIPS 202 draft */
    enum EHashMode {
      c_SHAKE128, /**< variable hash size with up to 128 bit collision resistance */
      c_SHAKE256, /**< variable hash size with up to 256 bit collision resistance */
    };
    /** constructor initializing the hash structure */
    explicit ShakeHash(EHashMode mode);
    /** No copying */
    ShakeHash(const ShakeHash&) = delete;
    /** destructor freeing the memory */
    ~ShakeHash();
    /** reinit the hash structure to create a new hash sum */
    void clear();
    /** update the internal state by adding n bytes of data from buff */
    void update(int n, unsigned char* buff);
    /** obtain the hash value with a length of n bytes into buff */
    void getHash(int n, unsigned char* buff);
  private:
    /** memory structure to calculate the hash value */
    KeccakHashInstance* m_instance;
    /** chosen hash mode */
    EHashMode m_mode;
  };
} //Belle2 namespace
