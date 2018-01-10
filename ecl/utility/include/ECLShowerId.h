/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERID_H
#define ECLSHOWERID_H


namespace Belle2 {
  namespace ECL {
    /**
     * Class to convert the shower id into CR, CS and Seed and vice versa
     */
    class ECLShowerId {

    public:

      /**
       * Constructor
       */
      ECLShowerId();

      /**
       * Returns connected region for a given shower ID
       */
      int getCRID(const int showerid) const
      {
        return showerid / m_CRMultiplier;
      }

      /**
       * Returns hypothesis for a given shower ID
       */
      int getHypothesis(const int showerid) const
      {
        return (showerid % m_CRMultiplier) / m_HypothesisMultiplier;
      }

      /**
       * Returns seed for a given shower ID
       */
      int getSeed(const int showerid) const
      {
        return ((showerid % m_CRMultiplier) % m_HypothesisMultiplier) / m_SeedMultiplier;
      }

      /**
       * Returns shower identifier for given CRID, hypothesis and seed
       */
      int getShowerId(const int crid, const int hypothesis, const int seed) const
      {
        return m_CRMultiplier * crid + m_HypothesisMultiplier * hypothesis + m_SeedMultiplier * seed;
      }


    private:
      /** Connected Region Multiplier */
      const int m_CRMultiplier = 100000;

      /** Hypothesis Multiplier     */
      const int m_HypothesisMultiplier = 1000;

      /** Seed Multiplier     */
      const int m_SeedMultiplier = 1;
    };
  }
} // Belle2 namespace

#endif



