/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKINGEVALUATIONMODULE_H_
#define TRACKINGEVALUATIONMODULE_H_

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <TFile.h>
#include <TTree.h>

#include <genfit/TrackCand.h>

namespace Belle2 {


  class TrackingEvaluationModule : public Module {

  public:

    TrackingEvaluationModule();

    virtual ~TrackingEvaluationModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();



  protected:

  private:
    // Functions to fill the variables to be flushed to the TTrees
    void assignEventVars();
    void assignPRTrackVars(const genfit::TrackCand& mcTrackCand);
    void assignMCTrackVars(const genfit::TrackCand& prTrackCand);


    enum class MCToPRMatchInfo { MATCHED, MERGED, MISSING, INVALID };
    enum class PRToMCMatchInfo { MATCHED, CLONE, BACKGROUND, GHOST, INVALID };

    const genfit::TrackCand* getRelatedMCTrackCand(const genfit::TrackCand&, float& purity);
    const genfit::TrackCand* getRelatedPRTrackCand(const genfit::TrackCand&, float& efficiency);

    MCToPRMatchInfo getMCToPRMatchInfo(const genfit::TrackCand* prTrackCand, const float& efficiency);
    PRToMCMatchInfo getPRToMCMatchInfo(const genfit::TrackCand& prTrackCand, const genfit::TrackCand* mcTrackCand, const float& purity);

    //Parameters
    std::string m_param_prGFTrackCandsColName;                        /**< TrackCandidates collection name from the patter recognition*/
    std::string m_param_mcGFTrackCandsColName;                        /**< TrackCandidates collection name from the mc recognition*/

    std::string m_param_outputFileName;                               /**< Name of the ROOT output file for the various Trees*/

    // TTrees
    TFile* m_outputFile;                                              /**< ROOT output file */
    TTree* m_eventTree;                                               /**< ROOT tree with output data for each event */
    TTree* m_prTrackTree;                                             /**< ROOT tree with output data for each pattern recognition track */
    TTree* m_mcTrackTree;                                             /**< ROOT tree with output data for each pattern recognition track */

    // TTreeBranch variables

    /* To remember :  ROOT leaf types characters
       Leaves are specified with

          isName/O

       where isName is the leave name and O is the type character
       (Bool_t in this case)

      - C : a character string terminated by the 0 character
      - B : an 8 bit signed integer (Char_t)
      - b : an 8 bit unsigned integer (UChar_t)
      - S : a 16 bit signed integer (Short_t)
      - s : a 16 bit unsigned integer (UShort_t)
      - I : a 32 bit signed integer (Int_t)
      - i : a 32 bit unsigned integer (UInt_t)
      - F : a 32 bit floating point (Float_t)
      - D : a 64 bit floating point (Double_t)
      - L : a 64 bit signed integer (Long64_t)
      - l : a 64 bit unsigned integer (ULong64_t)
      - O : [the letter 'o', not a zero] a boolean (Bool_t)
    */

    Int_t m_eventNumber;

    // Variables general to the event
    struct EventVars {

      Int_t nMCParticles;
      Int_t nMCTracks;
      Int_t nPRTracks;

      Int_t nMatched;

      // MC Classes
      Int_t nMissing;
      Int_t nMerged;

      // PR Classes
      Int_t nGhost;
      Int_t nBackground;
      Int_t nClone;

    } m_eventVars;

    // Setup routine for the Branches of the eventTree
    // Keep next to the variable defintion for easier change and extension
    void setupEventTree() {

      if (m_eventTree == nullptr) {
        B2FATAL("m_eventTree was not created.");
      }

      // Seperate event number
      m_eventTree->Branch("eventNumber", &m_eventNumber, "eventNumber/I");

      m_eventTree->Branch("nMCParticles", &m_eventVars.nMCParticles, "nMCParticles/I");
      m_eventTree->Branch("nMCTracks", &m_eventVars.nMCTracks, "nMCTracks/I");
      m_eventTree->Branch("nPRTracks", &m_eventVars.nPRTracks, "nPRTracks/I");

      m_eventTree->Branch("nMatched", &m_eventVars.nMatched, "nMatched/I");

      m_eventTree->Branch("nMissing", &m_eventVars.nMissing, "nMissing/I");
      m_eventTree->Branch("nMerged", &m_eventVars.nMerged, "nMerged/I");

      m_eventTree->Branch("nGhost", &m_eventVars.nGhost, "nGhost/I");
      m_eventTree->Branch("nBackground", &m_eventVars.nBackground, "nBackground/I");
      m_eventTree->Branch("nClone", &m_eventVars.nClone, "nClone/I");

    }

    struct MCTrackVars {

      Int_t mcTrackCandId;

      Int_t nPXDHits;
      Int_t nSVDHits;
      Int_t nCDCHits;
      Int_t ndf;

      Int_t relatedPRTrackCandId;
      Float_t efficiency;
      Float_t purity;

      Bool_t isMatched;
      Bool_t isMerged;
      Bool_t isMissing;

      Int_t truePDGCode;
      Float_t trueCharge;

      Float_t truePx;
      Float_t truePy;
      Float_t truePz;

    } m_mcTrackVars;


    // Setup routine for the Branches of the mcTrackTree
    // Keep next to the variable defintion for easier change and extension
    void setupMCTrackTree() {

      if (m_mcTrackTree == nullptr) {
        B2FATAL("m_mcTrackTree was not created.");
      }

      // Seperate event number
      m_mcTrackTree->Branch("eventNumber", &m_eventNumber, "eventNumber/I");

      // Copied general event variables
      m_mcTrackTree->Branch("nMCParticles", &m_eventVars.nMCParticles, "nMCParticles/I");
      m_mcTrackTree->Branch("nMCTracks", &m_eventVars.nMCTracks, "nMCTracks/I");
      m_mcTrackTree->Branch("nPRTracks", &m_eventVars.nPRTracks, "nPRTracks/I");

      // Variables for  the individual pattern recogntion track
      m_mcTrackTree->Branch("mcTrackCandId", &m_mcTrackVars.mcTrackCandId, "mcTrackCandId/I");

      m_mcTrackTree->Branch("nPXDHits", &m_mcTrackVars.nPXDHits, "nPXDHits/I");
      m_mcTrackTree->Branch("nSVDHits", &m_mcTrackVars.nSVDHits, "nSVDHits/I");
      m_mcTrackTree->Branch("nCDCHits", &m_mcTrackVars.nCDCHits, "nCDCHits/I");

      m_mcTrackTree->Branch("ndf", &m_mcTrackVars.ndf, "ndf/I");

      m_mcTrackTree->Branch("isMatched", &m_mcTrackVars.isMatched, "isMatched/O");
      m_mcTrackTree->Branch("isMerged", &m_mcTrackVars.isMerged, "isMerged/O");
      m_mcTrackTree->Branch("isMissing", &m_mcTrackVars.isMissing, "isMissing/O");

      m_mcTrackTree->Branch("relatedPRTrackId", &m_mcTrackVars.relatedPRTrackCandId, "relatedPRTrackId/I");

      m_mcTrackTree->Branch("efficiency", &m_mcTrackVars.efficiency, "efficiency/F");
      m_mcTrackTree->Branch("purity", &m_mcTrackVars.purity, "purity/F");

      m_mcTrackTree->Branch("truePDGCode", &m_mcTrackVars.truePDGCode, "truePDGCode/I");
      m_mcTrackTree->Branch("trueCharge,", &m_mcTrackVars.trueCharge, "trueCharge/F");

      m_mcTrackTree->Branch("truePx", &m_mcTrackVars.truePx, "truePx/F");
      m_mcTrackTree->Branch("truePy", &m_mcTrackVars.truePy, "truePy/F");
      m_mcTrackTree->Branch("truePz", &m_mcTrackVars.truePz, "truePz/F");

    }

    struct PRTrackVars {

      Int_t prTrackCandId;

      Int_t nPXDHits;
      Int_t nSVDHits;
      Int_t nCDCHits;

      Int_t ndf;

      Bool_t isMatched;
      Bool_t isClone;
      Bool_t isBackground;
      Bool_t isGhost;

      Int_t relatedMCTrackCandId;
      Float_t purity;
      Float_t efficiency;

      Int_t truePDGCode;
      Float_t trueCharge;

      Float_t truePx;
      Float_t truePy;
      Float_t truePz;

    } m_prTrackVars;

    // Setup routine for the Branches of the prTrackTree
    // Keep next to the variable defintion for easier change and extension
    void setupPRTrackTree() {

      if (m_prTrackTree == nullptr) {
        B2FATAL("m_ptTrackTree was not created.");
      }

      // Seperate event number
      m_prTrackTree->Branch("eventNumber", &m_eventNumber, "eventNumber/I");

      // Copied general event variables
      m_prTrackTree->Branch("nMCParticles", &m_eventVars.nMCParticles, "nMCParticles/I");
      m_prTrackTree->Branch("nMCTracks", &m_eventVars.nMCTracks, "nMCTracks/I");
      m_prTrackTree->Branch("nPRTracks", &m_eventVars.nPRTracks, "nPRTracks/I");

      // Variables for  the individual pattern recogntion track
      m_prTrackTree->Branch("prTrackCandId", &m_prTrackVars.prTrackCandId, "prTrackCandId/I");

      m_prTrackTree->Branch("nCDCHits", &m_prTrackVars.nCDCHits, "nCDCHits/I");
      m_prTrackTree->Branch("nSVDHits", &m_prTrackVars.nSVDHits, "nSVDHits/I");
      m_prTrackTree->Branch("nPXDHits", &m_prTrackVars.nPXDHits, "nPXDHits/I");
      m_prTrackTree->Branch("ndf", &m_prTrackVars.ndf, "ndf/I");

      m_prTrackTree->Branch("isMatched", &m_prTrackVars.isMatched, "isMatched/O");
      m_prTrackTree->Branch("isClone", &m_prTrackVars.isClone, "isClone/O");
      m_prTrackTree->Branch("isGhost", &m_prTrackVars.isGhost, "isGhost/O");
      m_prTrackTree->Branch("isBackground", &m_prTrackVars.isBackground, "isBackground/O");

      m_prTrackTree->Branch("relatedMCTrackId", &m_prTrackVars.relatedMCTrackCandId, "relatedMCTrackId/I");

      m_prTrackTree->Branch("purity", &m_prTrackVars.purity, "purity/F");
      m_prTrackTree->Branch("efficiency", &m_prTrackVars.efficiency, "efficiency/F");


      m_prTrackTree->Branch("truePDGCode", &m_prTrackVars.truePDGCode, "truePDGCode/I");
      m_prTrackTree->Branch("trueCharge", &m_prTrackVars.trueCharge, "trueCharge/F");

      m_prTrackTree->Branch("truePx", &m_prTrackVars.truePx, "truePx/F");
      m_prTrackTree->Branch("truePy", &m_prTrackVars.truePy, "truePy/F");
      m_prTrackTree->Branch("truePz", &m_prTrackVars.truePz, "truePz/F");

    }




    //Other variables

    typedef int NDF;
    std::map<int, NDF> m_ndf_by_detId {
      std::make_pair(Const::PXD, 2),
      std::make_pair(Const::SVD, 2),
      std::make_pair(Const::CDC, 1)
    };                                 /**< Map storing the standard number degrees of freedom for a single hit by detector */


  }; // end class
} // end namespace Belle2

#endif /* TRACKINGEVALUATIONMODULE_H_ */

