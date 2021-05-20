/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018, 2021 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan strube, Marko Staric, Connor Hainje                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPPDFCollection.h>
#include <top/dataobjects/TOPAssociatedPDF.h>
#include <top/dataobjects/TOPPixelLikelihood.h>
#include <framework/gearbox/Const.h>
#include <string>

namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPPDFDebuggerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPPDFDebuggerModule();

    /**
     * Destructor
     */
    virtual ~TOPPDFDebuggerModule()
    {}

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     *
     */
    virtual void event() override;

  private:

    /**
     * Associate PDF peaks with photons using S-plot technique
     * @param pdfConstructor reconstruction object for given track and hypothesis
     */
    void associatePDFPeaks(const TOP::PDFConstructor& pdfConstructor);

    // Module steering parameters

    double m_minTime = 0;      /**< optional time limit for photons */
    double m_maxTime = 0;      /**< optional time limit for photons */
    std::string m_pdfOption;   /**< PDF option name */
    std::vector<int> m_pdgCodes;   /**< particle codes */

    // others
    TOP::PDFConstructor::EPDFOption m_PDFOption = TOP::PDFConstructor::c_Rough; /**< PDF option */
    std::vector<Const::ChargedStable> m_chargedStables;  /**< particle hypotheses */

    // collections
    StoreArray<Track> m_tracks;  /**< collection of tracks */
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<TOPPDFCollection> m_pdfCollection; /**< collection of analytic PDF's */
    StoreArray<TOPAssociatedPDF> m_associatedPDFs; /**< collection of associated PDF's */
    StoreArray<TOPPixelLikelihood> m_pixelData; /**< collection of per-pixel data */
  };

} // Belle2 namespace

