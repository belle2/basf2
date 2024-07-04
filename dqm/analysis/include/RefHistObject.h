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
    TCanvas* m_canvas{}; /**< canvas where we draw the histogram*/
    TH1* m_refHist{};/**< Pointer to reference histogram */
    TH1* m_refCopy{};/**< Pointer to scaled reference histogram */
    bool m_updated = false; /**< flag if update since last event */

    /** Reference Node structure used in DQMHistReferenceModule*/
    typedef struct {
      /** online histogram name */
      std::string orghist_name;
      /** reference histogram name */
      std::string refhist_name;
      /** related canvas name name */
      TCanvas* canvas{nullptr};
      /** pointer to the org reference histogram */
      TH1* ref_org{nullptr};
      /** pointer to the cloned/scaled reference histogram */
      TH1* ref_clone{nullptr};
    } REFNODE;

    /** Unique object that could be used by ReferenceModule to access objects directly */
    REFNODE m_refNode{m_orghist_name, m_refhist_name, m_canvas, m_refHist, m_refCopy };

  public:

    /** Constructor
     */
    RefHistObject(void) : m_orghist_name(""), m_refhist_name(""), m_canvas(nullptr), m_refHist(nullptr), m_refCopy(nullptr),
      m_updated(false) {};

    /** Destructor
     */
    ~RefHistObject(void);

    /** Updating original reference, and scaled reference
     * @param ref pointer to reference
     * @param norm normalization from original histogram
     * @return histogram was updated flag (return m_updated)
     */
    bool update(TH1* ref, double norm);

    /** Reset histogram and update flag, not the entries
     */
    void resetBeforeEvent(void);

    /** Get hist pointer
    * @return hist ptr
    */
    TCanvas* getCanvas(void) { return m_canvas;};

    /** Get ref hist pointer
    * @return ref hist ptr
    */
    TH1* getRefHist(void) { return m_refHist;};

    /** Get scaled ref hist pointer
    * @return scaled ref hist ptr
    */
    TH1* getRefCopy(void) { return m_refCopy;};

    /** Get custom node structure output (for reference module)
    * @return refnode
    */
    REFNODE getRefNode(void) { return m_refNode;};

  };
}
