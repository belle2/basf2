/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/DigitBase.h>
#include <string>
#include <map>
#include <vector>

namespace Belle2 {

  /**
   * Overlay of measured background with simulated data (Digits or Clusters)
   */
  class BGOverlayExecutorModule : public Module {

  public:

    /**
     * Constructor
     */
    BGOverlayExecutorModule();

    /**
     * Destructor
     */
    virtual ~BGOverlayExecutorModule()
    {}

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_PXDDigitsName; /**< name of PXD collection to overlay with BG */
    std::string m_SVDShaperDigitsName; /**< name of SVD collection to overlay with BG */
    std::string m_CDCHitsName;   /**< name of CDC collection to overlay with BG */
    std::string m_TOPDigitsName; /**< name of TOP collection to overlay with BG */
    std::string m_ARICHDigitsName; /**< name of ARICH collection to overlay with BG */
    std::string m_KLMDigitsName; /**< name of KLM collection to overlay with BG */

    std::string m_extensionName; /**< name added to default branch names */
    std::string m_BackgroundInfoInstanceName = ""; /**< name BackgroundInfo name */

    std::vector<std::string> m_components; /**< detector components included in overlay */

    bool m_addPXD = false; /**< if true add BG digits of PXD */
    bool m_addSVD = false; /**< if true add BG digits of SVD */
    bool m_addCDC = false; /**< if true add BG digits of CDC */
    bool m_addTOP = false; /**< if true add BG digits of TOP */
    bool m_addARICH = false; /**< if true add BG digits of ARICH */
    bool m_addKLM = false; /**< if true add BG digits of KLM */

    /**
     * Register simulated and BG digits (both as optional input)
     */
    template <class Digit>
    void registerDigits(const std::string& name)
    {
      StoreArray<Digit> digits(name);
      digits.isOptional();
      StoreArray<Digit> tmp; // just to get the default name
      std::string nameBG = tmp.getName() + m_extensionName;
      StoreArray<Digit> bgDigits(nameBG);
      bgDigits.isOptional();
      B2DEBUG(20, "optional input: " << digits.getName() << " " << bgDigits.getName());
    }

    /**
     * Add BG digits to simulated ones. Class must inherit from DigitBase.
     * BG digit can be either merged with existing one in a channel or appended to array.
     */
    template <class Digit>
    void addBGDigits(const std::string& name)
    {
      // simulated digits
      StoreArray<Digit> digits(name);
      if (!digits.isValid()) {
        B2DEBUG(20, digits.getName() << " are not valid");
        return;
      }

      // background digits
      StoreArray<Digit> tmp; // just to get the default name
      std::string nameBG = tmp.getName() + m_extensionName;
      StoreArray<Digit> bgDigits(nameBG);
      if (!bgDigits.isValid()) {
        B2DEBUG(20, bgDigits.getName() << " are not valid");
        return;
      }

      // map unique channel ID with simulated digits
      std::multimap<unsigned, Digit*> multimap;
      for (auto& digit : digits) {
        unsigned id = digit.getUniqueChannelID();
        // multimap.emplace(id, &digit); /* Clang doesn't know this member! */
        multimap.insert(std::pair<unsigned, Digit*>(id, &digit));
      }

      int size = digits.getEntries(); // for debug print

      // add BG digits to simulated ones
      typedef typename std::multimap<unsigned, Digit*>::iterator Iterator;
      for (const auto& bgDigit : bgDigits) {
        bool pileup = false;
        unsigned id = bgDigit.getUniqueChannelID();
        std::pair<Iterator, Iterator> range = multimap.equal_range(id);
        for (Iterator it = range.first; it != range.second; ++it) {
          auto* digit = it->second;
          pileup = digit->addBGDigit(&bgDigit) == DigitBase::c_DontAppend;
          if (pileup) break; // BG digit merged with simulated one
        }
        if (!pileup) digits.appendNew(bgDigit)->adjustAppendedBGDigit(); // BG digit not merged, therefore append and possibly modify
      }

      // debug printout
      int diff = size + bgDigits.getEntries() - digits.getEntries();
      B2DEBUG(20, digits.getName() << ": before " << size
              << " + " << bgDigits.getEntries() << "(BG)"
              << ", after " << digits.getEntries()
              << ", merged " << diff);

    }

  };

} // Belle2 namespace

