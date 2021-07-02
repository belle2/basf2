/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <iostream>
#include <iomanip>
#include <map>
#include <TObject.h>
#include <TRandom.h>
#include <cdc/dbobjects/CDCCrossTalkClasses.h>

using std::vector;
using std::upper_bound;
using std::equal_range;
using std::pair;

namespace Belle2 {

  /**
   * Database object for ASIC crosstalk library
   */
  class CDCCrossTalkLibrary: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCCrossTalkLibrary() = default;

    /**
     * Add a new ASIC record to the library
     * @param channel  -- Channel number inside RO board (between 0 and 48)
     * @param ADC      -- ADC value
     * @param asicInfo -- TDC,ADC,TOT information for all channels in ASIC, -1 if no hit.
     */
    void addAsicRecord(const Short_t channel, const Short_t ADC, const asicChannels& asicInfo)
    {
      Short_t ch8 = channel % 8;
      adcAsicTuple entry{ADC, ch8 , asicInfo};
      // keep sorted
      auto place = upper_bound(m_library.begin(), m_library.end(), entry, adc_search());
      m_library.insert(place, entry);
    }


    /**
        Get cross talk record from the library. Output is a vector of pairs: channel number (from 0 to 48) and corresponding TDC,ADC,TOT values.
        Depending on the value of insertSignalToOutput, the output may contain the input signal hit.
         @param channel  readout board channel number, from 0 to 48
         @param TDCin  input channel TDC value
         @param ADCin  input channel ADC value
         @param TOTin  input channel TOT value
         @param entry  for multiple entries given (channel,ADC) value either return random (entry=0) or specific one (entry>0). If entry > max entries, entry % max entries is used
         @param insertSignalToOutput Add signal to the output  vector
    */
    const vector< pair<Short_t, asicChannel> > getLibraryCrossTalk(Short_t channel, Short_t TDCin, Short_t ADCin, Short_t TOTin,
        size_t entry = 0, bool insertSignalToOutput = false) const
    {
      // output
      vector< pair<Short_t, asicChannel> > outRec;
      auto prob = gRandom->Uniform();

      if (prob > pCrossTalk(ADCin)) {
        /// return signal-only, no x-talk
        if (insertSignalToOutput)
          outRec.emplace_back(std::make_pair(channel, asicChannel{TDCin, ADCin, TOTin}));
        return outRec;
      }

      // find a range of possible candidates
      Short_t ch8  = channel % 8;
      adcChannelPair query{ADCin, ch8};
      auto pADC = equal_range(m_library.begin(), m_library.end(), query, adc_search());

      size_t size = std::distance(pADC.first, pADC.second);
      size_t val = 0;


      if (entry != 0) {
        // select modulo
        val = entry % size;
      } else {
        // select random
        val = gRandom->Integer(size);
      }

      asicChannels rec;
      bool recNotSet = true;
      if (size == 0) {
        if (pADC.first == m_library.end()) {
          rec = m_library.back().record;
          recNotSet = false;
        } else {
          if (pADC.first->Channel == ch8) {
            rec = (pADC.first)->record;
            recNotSet = false;
          } else { // need to step back, if possible
            if ((--pADC.first)->Channel == ch8) {
              rec = (--pADC.first)->record;
              recNotSet = false;
            } else {
              B2WARNING("Could not find CDC Cross talk library entry for channel, ADC: " << channel << " " << ADCin);
              if (insertSignalToOutput)
                outRec.emplace_back(std::make_pair(channel, asicChannel{TDCin, ADCin, TOTin}));
              return outRec;
            }
          }
        }
      } else {
        size_t count = 0;
        for (auto p = pADC.first; p != pADC.second; ++p) {
          if (count == val) {
            rec = p->record;
            recNotSet = false;
          }
          count += 1;
        }
      }

      B2ASSERT("CDC cross talk record not set", !recNotSet);

      /// Determine Delta in TDC:
      Short_t DeltaTDC = TDCin - rec[ch8].TDC;

      if (abs(DeltaTDC) > 1000) {
        B2WARNING("Large xTalk DeltaTDC=" << DeltaTDC);
      }

      //      std::cout << TDCin << " HHH " << rec[ch8].TDC << " " << ch8 << "\n";


      for (int i = 0; i < 8; i += 1) {
        if (rec[i].TDC > 0) {
          if (i == ch8) {   // store input values
            B2ASSERT("CDC Cross talk entry for the selected channel cannot be empty " << ch8 << " " << rec[ch8].TDC , rec[ch8].TDC > -1);
            if (insertSignalToOutput)
              outRec.emplace_back(std::make_pair(channel, asicChannel{TDCin, ADCin, TOTin}));
          } else { // adjust TDC for the cross talk, keep ADC and TOT
            Short_t TDCout = rec[i].TDC + DeltaTDC;
            outRec.emplace_back(std::make_pair(i - ch8 + channel , asicChannel{TDCout, rec[i].ADC, rec[i].TOT}));
          }
        }
      }
      return outRec;
    }



    /**
     * Print out contents of the library
     */
    void dump(int verbosity) const
    {
      std::cout << "Content of CDCCrossTalkLibrary" << std::endl;
      std::cout << "Size = " <<  m_library.size() << "\n";
      if (verbosity < 1) return;

      for (size_t i = 0 ; i < m_library.size(); i += 1) {
        auto [adc, ch, record] = m_library[i];
        std::cout << "ADC: " << adc << " CH: " << ch << " count: " << i << "\n";
        std::cout << "  TDC    ADC  TOT \n";
        for (auto rec : record) {
          std::cout << "  " << rec.TDC << " " << rec.ADC << " " << rec.TOT << "\n";
        }
      }
      std::cout << "Probability of cross talk vs ADC: \n";
      for (size_t a = 0; a < 8196; a++) {
        std::cout << "P(" << a << ")=" << m_pCrossTalk[a] << "\n";
      }
    }

    /// Dump single entry, for a given channel
    void dumpEntry(size_t entry)
    {

      if (entry > m_library.size()) entry = m_library.size() - 1;
      auto [adc, ch, record] = m_library[entry];
      std::cout << "ADC:" << adc << " CH: " << ch << " Size: " <<  m_library.size() <<  "\n";
      std::cout << "  TDC    ADC  TOT \n";
      for (auto rec : record) {
        std::cout << "  " << rec.TDC << " " << rec.ADC << " " << rec.TOT << "\n";
      }

    }

    /**
     * Get probability of the cross talk
     */
    double pCrossTalk(const Short_t ADC) const
    {
      if (ADC < 0) return 0;
      if (ADC >= 8196) return 1;
      return m_pCrossTalk[ADC];
    }

    /// Store x-talk probability
    void setPCrossTalk(const double* probs)
    {
      for (size_t i = 0; i < m_pCrossTalk.size(); i += 1) {
        m_pCrossTalk[i] = probs[i];
      }
    }

  private:
    std::vector<adcAsicTuple>  m_library;   ///< Library
    array<float, 8196>   m_pCrossTalk;      ///< x-talk probability

    ClassDef(CDCCrossTalkLibrary, 2); /**< ClassDef */
  };

} // end namespace Belle2
