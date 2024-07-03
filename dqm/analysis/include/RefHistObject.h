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
   */
  class RefHistObject {


  public:
    std::string orghist_name; /** online histogram name */
    std::string refhist_name; /** online histogram name */
    TH1* m_hist{};/**< Pointer to original histogram */
    TH1* m_refHist{};/**< Pointer to reference histogram */
    TH1* m_refCopy{};/**< Pointer to scaled reference histogram */

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

    REFNODE m_refNode{};

  public:

    /** Constructor
     */
    RefHistObject(void) : orghist_name(""), refhist_name(""), m_hist(nullptr), m_refHist(nullptr), m_refCopy(nullptr) {};

    /** Get hist pointer
    * @return hist ptr
    */
    TH1* getHist(void) { return m_hist;};

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
