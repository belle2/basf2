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
    std::shared_ptr <TCanvas> m_canvas; /**< canvas where we draw the histogram*/
    std::unique_ptr <TH1> m_refHist;/**< Pointer to reference histogram */
    std::unique_ptr <TH1> m_refCopy;/**< Pointer to scaled reference histogram */
    bool m_updated = false; /**< flag if update since last event */

  public:

    /** Constructor
     */
    RefHistObject(void) : m_orghist_name(""), m_refhist_name(""), m_canvas(nullptr), m_refHist(nullptr), m_refCopy(nullptr),
      m_updated(false) {};

    // Move constructor
    RefHistObject(RefHistObject&& other) noexcept
      : m_orghist_name(std::move(other.m_orghist_name)),
        m_refhist_name(std::move(other.m_refhist_name)),
        m_canvas(std::move(other.m_canvas)),
        m_refHist(std::move(other.m_refHist)),
        m_refCopy(std::move(other.m_refCopy)),
        m_updated(other.m_updated)
    {
      // Reset the moved-from object
      other.m_canvas = nullptr;
      other.m_refHist = nullptr;
      other.m_refCopy = nullptr;
      other.m_updated = false;
    }

    // Move assignment operator
    RefHistObject& operator=(RefHistObject&& other) noexcept
    {
      if (this != &other) {
        m_orghist_name = std::move(other.m_orghist_name);
        m_refhist_name = std::move(other.m_refhist_name);
        m_canvas = std::move(other.m_canvas);
        m_refHist = std::move(other.m_refHist);
        m_refCopy = std::move(other.m_refCopy);
        m_updated = other.m_updated;

        // Reset the moved-from object
        other.m_canvas = nullptr;
        other.m_refHist = nullptr;
        other.m_refCopy = nullptr;
        other.m_updated = false;
      }
      return *this;
    }

    /** Destructor
     */
    ~RefHistObject(void);

    /** Updating original reference, and scaled reference
     * @param ref pointer to reference
     * @param refCopy pointer to pre-scaled reference
     * @param canvas pointer to canvas
     * @return histogram was updated flag (return m_updated)
     */
    void update(TH1* ref, TH1* refCopy, TCanvas* canvas);

    /** Reset histogram and update flag, not the entries
     */
    void resetBeforeEvent(void);

    /** Get hist pointer
    * @return hist ptr
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

    // Setter for m_canvas
    void setCanvas(TCanvas* canvas)
    {
      m_canvas.reset(canvas);  // Assumes ownership of canvas
    }

    // Setter for m_refHist
    void setRefHist(TH1* refHist)
    {
      m_refHist.reset(refHist);  // Assumes ownership of refHist
    }

    // Setter for m_refCopy
    void setRefCopy(TH1* refCopy)
    {
      m_refCopy.reset(refCopy);  // Assumes ownership of refCopy
    }

  };
}
