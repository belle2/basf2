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

#include <stdint.h>
#include <TRandom.h>
#include <vector>

namespace Belle2 {
  /** Fast Random number Generator using on xorshift1024* [arXiv:1402.6246].
   * It has a period of 2^{1024-1} and passes the Big Crush test while being
   * very small and fast with an internal state of just 1024bit. We seed the
   * generator at the beginning of each event with a combination of a seed
   * value and the experiment/run/event number information of that event to
   * ensure maximum reproducibility even in multiprocessing environments.
   *
   * The seeding is done using the Keccak secure hash algorithm (SHA-3, FIPS
   * 202 Draft) with arbitrary hash length (set to 1024bit). This ensures
   * evenly distributed generator states even with only small changes in the
   * event information and large numbers of zeros in the seed information. We
   * generate the hash from the (optional) arbitrary seed information followed
   * by the event meta data in big endian representation.
   *
   * See BELLE2-NOTE-TE-2015-031: Belle II UNICORN – UNIfied Calculation Of Random Numbers
   * for details. Available at: https://docs.belle2.org/record/292?ln=en
   */
  class RandomGenerator: public TRandom {
  public:
    /** Generator mode: determines which information is used to generate the
     * internal state */
    enum EGeneratorMode {
      /** Don't use event info to generate state */
      c_independent,
      /** Use experiment and run number to generate state */
      c_runDependent,
      /** Use experiment, run and event number to generate state */
      c_eventDependent
    };

    /** Default constructor, does not initialize the generator. */
    RandomGenerator();

    /** Destructor to free the seed information */
    virtual ~RandomGenerator() {}

    /** Set the seed information.
     * This can be an arbitrary sequence of bytes: strings, values, etc.
     * It should be less or equal to 128 bytes as this is the size of
     * the random generator state but it can be longer. The seed is
     * copied when calling this function so the original buffer can be
     * freed afterwards
     * @param seed pointer to seed data
     * @param n of the seed data in bytes
     */
    void setSeed(const unsigned char* seed, unsigned int n);

    /** Set the generator mode */
    void setMode(EGeneratorMode mode) { m_mode = mode; }

    /** return the seed object */
    const std::vector<unsigned char>& getSeed() const { return m_seed; }

    /** set the State from event meta information like experiment, run, and
     * event number. The event number is only used if the generator is set to
     * event dependent on construction.
     */
    void initialize() { setState(0); }

    /** increase the barrier index. This will reseed the generator with the
     * current event information and and a new barrier index to make random
     * number state independent between different parts of the steering file.
     */
    void barrier() { setState(m_barrier + 1); }

    /** manually set the barrier index to a fixed value */
    void setBarrier(int barrierIndex) { setState(barrierIndex); }

    /** obtain the currently active barrier id */
    int getBarrier() const { return m_barrier; }

    /** Generate one 64bit unsigned integer between 0 and
     * UINT64_MAX (both inclusive).
     * @return random value in [0, UINT64_MAX]
     */
    uint64_t random64();

    /** Generate one 32bit unsigned integer between 0 and
     * UINT32_MAX (both inclusive)
     * @return random value in [0, UINT32_MAX]
     */
    uint32_t random32() { return random64() >> 32; }

    /** Generate a random double value between 0 and 1, both limits excluded.
     * @return value in (0,1)
     */
    double random01();

    /** Generate a random value in (0,1), both limits excluded. */
    Double_t Rndm() { return random01(); }
    /** Generate a random value in (0,1), both limits excluded (backward compatibility with root < 6.08). */
    Double_t Rndm(Int_t) { return Rndm(); }
    /** Fill an array of floats with random values in (0,1), both limits excluded.
     * @param n number of floats to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, Float_t* array);
    /** Fill an array of doubles with random values in (0,1), both limits excluded.
     * @param n number of doubles to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, Double_t* array);
    /** Fill an array of unsigned 64bit integers with random values in
     * [0, UINT64_MAX], both limits included.
     * @param n number of ints to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, ULong64_t* array);

    /** Fill an array of unsigned integers with random values in
     * [0, UINT32_MAX], both limits included.
     * @param n number of ints to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, UInt_t* array);

    /** Fill an array of 32bit integers with random values in
     * [INT32_MIN, INT32_MAX], both limits included.
     * @param n number of ints to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, Int_t* array) { RndmArray(n, (UInt_t*) array); }

    /** Fill an array of 64bit integers with random values in
     * [INT64_MIN, INT64_MAX], both limits included.
     * @param n number of ints to generate
     * @param array pointer to an array where the numbers should be stored
     */
    void RndmArray(Int_t n, Long64_t* array) { RndmArray(n, (ULong64_t*) array); }

    /** Fill an array with random data
     * @param n number of bytes to generate
     * @param array pointer to an array where the data should be stored
     */
    void RndmArray(Int_t n, unsigned char* array);
  private:
    /** override base class SetSeed to do nothing, we don't need it but it gets
     * called by parent constructor */
    void SetSeed(UInt_t) {}
    /** argument type was changed in root 6.08. */
    void SetSeed(ULong_t) {}

    /** Set the state of the random number generator.
     * To achieve maximum reproducibility the generator is reset at the begin
     * of every event using a combination of the seed data and the
     * experiment/run/event number and possible some extra information.
     *
     * To achieve a good separation of the state for similar event information
     * we use the Keccak hash algorithm (SHAKE, FIPS202 draft) where we select
     * the hashsize to be equal to the generator state. This generates evenly
     * distributed generator states even for input data which contains many
     * zeros.
     *
     * @param barrier index to use when setting the state. m_barrier will be
     *    set to his value
     */
    void setState(int barrier);

    /** Internal state of the random number generator */
    uint64_t m_state[16];
    /** currently active index in the internal state */
    unsigned int m_index;
    /** current barrier index. This is used to separate random numbers in
     * different modules by resetting the state between modules using a
     * different barrier index. */
    int m_barrier;
    /** seed information */
    std::vector<unsigned char> m_seed;
    /** Current generator mode */
    EGeneratorMode m_mode;
    /** and the root dictionary macro needs to be documented as well :)
     * Version 2: merge m_eventDependent and m_useEventData into a general
     * generator mode m_mode
     */
    ClassDef(RandomGenerator, 2);
  };

  inline uint64_t RandomGenerator::random64()
  {
    //Generate random number using magic, taken from [arXiv:1402.6246] and only
    //changed to conform to naming scheme
    uint64_t s0 = m_state[ m_index ];
    uint64_t s1 = m_state[ m_index = (m_index + 1) & 15 ];
    s1 ^= s1 << 31;
    s1 ^= s1 >> 11;
    s0 ^= s0 >> 30;
    return (m_state[ m_index ] = s0 ^ s1) * 1181783497276652981LL;
  }

  inline void RandomGenerator::RndmArray(Int_t n, Float_t* array)
  {
    //We could optimize this more thoroughly since one 64bit random int is
    //enough to generate two floats but this would probably be rather academic
    for (int i = 0; i < n; ++i) array[i] = random01();
  }

  inline void RandomGenerator::RndmArray(Int_t n, Double_t* array)
  {
    //Fill the array, no optimization whatsoever necessary
    for (int i = 0; i < n; ++i) array[i] = random01();
  }

  inline void RandomGenerator::RndmArray(Int_t n, ULong64_t* array)
  {
    //Fill the array, no optimization whatsoever necessary
    for (int i = 0; i < n; ++i) array[i] = random64();
  }

  inline void RandomGenerator::RndmArray(Int_t n, UInt_t* array)
  {
    //Fill the most part of the array using 64bit numbers
    RndmArray(n / 2, (ULong64_t*)array);
    //Only for uneven number of elements we need to fill the last one using a
    //32bit number
    if (n % 2) array[n - 1] = random32();
  }

  inline double RandomGenerator::random01()
  {
    // There are two possibilities to generate a uniform double between 0 and
    // 1: multiply the integer by a constant or exploit the double
    // representation and use some bit shift magic. We have both implementation
    // here and they seem to produce the exact same output so we stick with the
    // more readable one but leave the bitshift solution just in case
#ifdef RANDOM_IEEE754
    //Generate a double in (0,1) using magic bit hackery with doubles: The
    //memory layout of a IEEE754 double precision floating point variable
    //is [sign(1)|exponent(11)|fraction(52)] with values in parentheses
    //being the number of bits. The actual value is then
    //-1^{sign} * (1.fraction) * 2^{exponent-1023}. Setting sign to 0 the
    //exponent to 1023 will thus return a value between 1 (inclusive) and 2
    //(exclusive). So we shift the 64bit integer to the right by 12 bits
    //(which gives as zeros for sign and exponent) and logical or this with
    //the correct binary representation of the exponent

    //To do this we use a union to modify the binary representation using
    //an integer and then return the double value
    union { uint64_t i; double d; } x;
    x.i = random64() >> 12;
    //This is a bit academic but we want (0,1) so if we happen to get
    //exactly zero we try again. Chance is 1 in 2^52
    if (x.i == 0) return random01();
    x.i |= 0x3FF0000000000000ULL;
    return x.d - 1.0;
#else
    //Generate a double (0,1) the traditional way by multiplying it with a
    //constant. As doubles only have a precision of 52 bits we need to
    //remove the 12 leading bits from our random int value
    const uint64_t x = random64() >> 12;
    //This is a bit academic but we want (0,1) so if we happen to get
    //exactly zero we try again. Chance is 1 in 2^52
    if (!x) return random01();
    //return x / 2^{52};
    return x * 2.220446049250313080847263336181640625e-16;
#endif
  }
}
