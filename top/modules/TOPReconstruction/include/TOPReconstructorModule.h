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
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPPull.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <framework/gearbox/Const.h>
#include <vector>
#include <string>
#include <limits>

namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPReconstructorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPReconstructorModule();

    /**
     * Destructor
     */
    virtual ~TOPReconstructorModule()
    {}

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    /**
     * A collection of PDF's of charged stable particles for a given track
     */
    struct PDFCollection {

      const TOP::TOPTrack* topTrack = 0; /**< track */
      std::vector<TOP::PDFConstructor*> PDFs; /**< PDF's of charged stable particles */
      const TOP::PDFConstructor* mostProbable = 0; /**< PDF of most probable particle hypothesis */
      bool isValid = false; /**< true if all PDF's are provided */

      /**
       * Constructor
       * @param trk given track
       * @param deltaRayModeling include or exclude delta-ray modeling in log likelihood calculation
       */
      PDFCollection(const TOP::TOPTrack& trk, bool deltaRayModeling)
      {
        topTrack = &trk;
        for (const auto& chargedStable : Const::chargedStableSet) {
          auto* pdf = new TOP::PDFConstructor(trk, chargedStable);
          if (not pdf->isValid()) {
            delete pdf;
            for (auto PDF : PDFs) delete PDF;
            PDFs.clear();
            return;
          }
          pdf->switchDeltaRayPDF(deltaRayModeling);
          PDFs.push_back(pdf);
        }
        isValid = true;
      }

      /**
       * Release memory allocated in the constructor
       */
      void deletePDFs()
      {
        for (auto pdf : PDFs) delete pdf;
        PDFs.clear();
        isValid = false;
      }

      /**
       * Sets most probable hypothesis
       */
      void setMostProbable()
      {
        if (not isValid) return;
        double maxLogL = -std::numeric_limits<double>::infinity();
        for (const auto pdf : PDFs) {
          double logL = pdf->getLogL().logL;
          if (logL > maxLogL) {
            mostProbable = pdf;
            maxLogL = logL;
          }
        }
      }

      /**
       * Append most probable PDF of other track in the same module
       * @param pdfOther PDF of other track
       */
      void appendPDFOther(const TOP::PDFConstructor* pdfOther)
      {
        if (not isValid) return;
        for (const auto pdf : PDFs) pdf->appendPDFOther(pdfOther);
      }

      /**
       * Clear the containers of PDF's of other tracks
       */
      void clearPDFOther()
      {
        if (not isValid) return;
        for (const auto pdf : PDFs) pdf->clearPDFOther();
      }

    };

    // Module steering parameters

    double m_minTime = 0;      /**< optional lower time limit for photons */
    double m_maxTime = 0;      /**< optional upper time limit for photons */
    int m_PDGCode = 0;   /**< PDG code of hypothesis to construct pulls */
    bool m_deltaRayModeling;  /**< include or exclude delta-ray modeling in log likelihood calculation */
    double m_pTCut;  /**< pT cut to suppress badly extrapolated tracks that cannot reach TOP counter */
    std::string m_topDigitCollectionName; /**< name of the collection of TOPDigits */
    std::string m_topLikelihoodCollectionName; /**< name of the collection of created TOPLikelihoods */
    std::string m_topPullCollectionName; /**< name of the collection of created TOPPulls */

    // datastore objects

    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreArray<ExtHit> m_extHits; /**< collection of extrapolated hits */
    StoreArray<TOPBarHit> m_barHits; /**< collection of MCParticle hits at TOP */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<TOPLikelihood> m_likelihoods; /**< collection of likelihoods */
    StoreArray<TOPPull> m_topPulls; /**< collection of pulls */

  };

} // Belle2 namespace

