/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TH1.h>
#include <TCanvas.h>

namespace Belle2 {

  /**
   * Class to keep track of reference histograms with the original
   */
  class RefHistObject {
  public:
    std::string m_orghist_name; /**< online histogram name */
    std::string m_refhist_name; /**< reference histogram name */
    std::unique_ptr <TCanvas> m_canvas; /**< canvas where we draw the histogram*/
    std::unique_ptr <TH1> m_refHist;/**< Pointer to reference histogram */
    std::unique_ptr <TH1> m_refCopy;/**< Pointer to scaled reference histogram */

  public:

    /** Constructor
     */
    RefHistObject(void) : m_orghist_name(""), m_refhist_name(""), m_canvas(nullptr), m_refHist(nullptr), m_refCopy(nullptr) {};

    /** Move constructor
    */
    RefHistObject(RefHistObject&& other) noexcept
      : m_orghist_name(std::move(other.m_orghist_name)),
        m_refhist_name(std::move(other.m_refhist_name)),
        m_canvas(std::move(other.m_canvas)),
        m_refHist(std::move(other.m_refHist)),
        m_refCopy(std::move(other.m_refCopy))
    {
      // Reset the moved-from object
      other.m_canvas = nullptr;
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
        m_canvas = std::move(other.m_canvas);
        m_refHist = std::move(other.m_refHist);
        m_refCopy = std::move(other.m_refCopy);

        // Reset the moved-from object
        other.m_canvas = nullptr;
        other.m_refHist = nullptr;
        other.m_refCopy = nullptr;
      }
      return *this;
    }

    /** Reset histogram and update flag, not the entries
     */
    void resetBeforeEvent(void);

    /** Get canvas pointer
    * @return canvas ptr
    */
    TCanvas* getCanvas(void) { return m_canvas.get();};

    /** Get ref hist pointer
    * @return ref hist ptr
    */
    TH1* getRefHist(void) { return m_refHist.get();};

    /** Get scaled ref hist pointer
    * @return scaled ref hist ptr
    */
    TH1* getRefCopy(void) { return m_refCopy.get();};

    /** Set canvas pointer
    * @param canvas input TCanvas pointer
    */
    void setCanvas(TCanvas* canvas)
    {
      m_canvas.reset(canvas);  // Assumes ownership of canvas
    }

    /** set ref hist pointer
    * @param refHist reference TH1 pointer
    */
    void setRefHist(TH1* refHist)
    {
      m_refHist.reset(refHist);  // Assumes ownership of refHist
    }

    /** set scaled ref hist pointer
    * @param refCopy scaled reference TH1 pointer
    */
    void setRefCopy(TH1* refCopy)
    {
      m_refCopy.reset(refCopy);  // Assumes ownership of refCopy
    }

    /** Get reference pointer for copy
    * @return reference histogram pointer
    */
    TH1* getReference(void);

  private:
    /** Make a reference copy
    */
    void makeReferenceCopy(void);
  };
}
