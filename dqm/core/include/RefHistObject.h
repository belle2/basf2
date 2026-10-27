/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TH1.h>

namespace Belle2 {

  /**
   * Class to keep track of reference histograms with the original
   * histograms copy (run type dependent, loaded on run start) and
   * possible scaled copies
   */
  class RefHistObject {
  public:
    std::string m_orghist_name; /**< online histogram name TODO this is not used */
    std::string m_refhist_name; /**< reference histogram name TODO this is not used */
    std::unique_ptr <TH1> m_refHist;/**< Pointer to reference histogram */
    std::unique_ptr <TH1> m_refCopy;/**< Pointer to scaled reference histogram */

  public:

    /** Constructor
     */
    RefHistObject(void) : m_orghist_name(""), m_refhist_name(""), m_refHist(nullptr), m_refCopy(nullptr) {};

    /** Move constructor
    */
    RefHistObject(RefHistObject&& other) noexcept
      : m_orghist_name(std::move(other.m_orghist_name)),
        m_refhist_name(std::move(other.m_refhist_name)),
        m_refHist(std::move(other.m_refHist)),
        m_refCopy(std::move(other.m_refCopy))
    {
      // Reset the moved-from object
      other.m_refHist = nullptr;
      other.m_refCopy = nullptr;
    }

    /** Move assignment operator
    */
    RefHistObject& operator=(RefHistObject&& other) noexcept
    {
      if (this != &other) {
        m_orghist_name = std::move(other.m_orghist_name);
        m_refhist_name = std::move(other.m_refhist_name);
        m_refHist = std::move(other.m_refHist);
        m_refCopy = std::move(other.m_refCopy);

        // Reset the moved-from object
        other.m_refHist = nullptr;
        other.m_refCopy = nullptr;
      }
      return *this;
    }

#if 0 // Unused, keep in case we need an explicit public accessor (e.g. testing). Code should use getReference below.
    /** Get reference histogram pointer
    * @return reference histogram pointer
    */
    TH1* getRefHist(void) { return m_refHist.get();};

    /** Get scaled reference histogram pointer
    * @return scaled reference histogram pointer
    */
    TH1* getRefCopy(void) { return m_refCopy.get();};
#endif

    /** set reference histogram pointer, takes ownership
    * @param refHist reference histogram pointer
    */
    void setRefHist(TH1* refHist)
    {
      m_refHist.reset(refHist);  // Assumes ownership of refHist
    }

    /** set scaled reference histogram pointer, takes ownership
    * @param refCopy scaled reference histogram pointer
    */
    void setRefCopy(TH1* refCopy)
    {
      m_refCopy.reset(refCopy);  // Assumes ownership of refCopy
    }

    /** Get reference histogram pointer
    * the pointer is to a freshly made copy, which can be modified by the caller (scaled)
    * without changing the original stored reference histogram. Ownership stays in this class.
    * @return reference histogram pointer
    */
    TH1* getReference(void);

  private:
    /** Make a copy of the underlying reference histogram which can be used for scaling.
    */
    void makeReferenceCopy(void);
  };
}
