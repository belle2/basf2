/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKINGOUTPUT_H_
#define TRACKINGOUTPUT_H_

#include "TObject.h"



namespace Belle2 {

  /** Class to store information after the fit. The idea is to have for each primary MCParticle an TrackingOutputClass, where all the information concerning this particle can be stored (MCtruth together with fit results). */
  class TrackingOutput : public TObject {
  public:

    /** Constructor, that initializes everything to 0.
     */
    TrackingOutput():
      m_mcMomentumX(0.), m_mcMomentumY(0.), m_mcMomentumZ(0.), m_mcMomentum(0.), m_mcCurv(0.),
      m_mcPhi(0.), m_mcCotTheta(0.), m_mcTheta(0.), m_mcPDG(0),
      m_mcPositionX(0.), m_mcPositionY(0.), m_mcPositionZ(0.) {
    }

    /** Destructor; nothing was newed in this class.
     */
    ~TrackingOutput() {

    }
    /** Setter for momentum of MCParticle (x-component).
     */
    void setMCMomentumX(float mcMomentumX) {m_mcMomentumX = mcMomentumX ;}

    /** Setter for momentum of MCParticle (y-component).
     */
    void setMCMomentumY(float mcMomentumY) {m_mcMomentumY = mcMomentumY ;}

    /** Setter for momentum of MCParticle (z-component).
     */
    void setMCMomentumZ(float mcMomentumZ) {m_mcMomentumZ = mcMomentumZ ;}

    /** Setter for momentum of MCParticle (absolute value).
     */
    void setMCMomentum(float mcMomentum) {m_mcMomentum = mcMomentum ;}

    /** Setter for curvature of MCParticle (inverse transverse momentum).
      */
    void setMCCurv(float mcCurv) {m_mcCurv = mcCurv ;}

    /** Setter for the phi angle of MCParticle (track angle in xy plane).
      */
    void setMCPhi(float mcPhi) {m_mcPhi = mcPhi;}

    /** Setter for Cotangens theta of the MCParticle (pz/pt).
     */
    void setMCCotTheta(float mcCotTheta) {m_mcCotTheta = mcCotTheta ;}

    /** Setter for theta of the MCParticle .
     */
    void setMCTheta(float mcTheta) {m_mcTheta = mcTheta ;}

    /** Setter for the pdg value of the MCParticle.
     */
    void setMCPDG(int mcPDG) {m_mcPDG = mcPDG ;}

    /** Setter for vertex of MCParticle (x-component).
     */
    void setMCPositionX(float mcPositionX) {m_mcPositionX = mcPositionX ;}

    /** Setter for vertex of MCParticle (y-component).
     */
    void setMCPositionY(float mcPositionY) {m_mcPositionY = mcPositionY ;}

    /** Setter for vertex of MCParticle (z-component).
     */
    void setMCPositionZ(float mcPositionZ) {m_mcPositionZ = mcPositionZ ;}


    /** Getter for momentum of MCParticle (x-component).
     */
    float getMCMomentumX() {return m_mcMomentumX; }

    /** Getter for momentum of MCParticle (y-component).
     */
    float getMCMomentumY() {return m_mcMomentumY; }

    /** Getter for momentum of MCParticle (z-component).
     */
    float getMCMomentumZ() {return m_mcMomentumZ; }

    /** Getter for momentum of MCParticle (absolute value).
     */
    float getMCMomentum() {return m_mcMomentum; }

    /** Getter for curvature of MCParticle (inverse transverse momentum).
     */
    float getMCCurv() {return m_mcCurv; }

    /** Getter for phi of MCParticle.
     */
    float getMCPhi() {return m_mcPhi; }

    /** Getter for cotangens theta of MCParticle (pz/pt).
     */
    float getMCCotTheta() {return m_mcCotTheta; }

    /** Getter for theta of MCParticle.
     */
    float getMCTheta() {return m_mcTheta; }


    //------------------------------------------------------------------------------------------------

    /** Setter for start value for the MCTrack fit: momentum (x-component).
     */
    void setMCStartMomentumX(float mcStartMomentumX) {m_mcStartMomentumX.push_back(mcStartMomentumX) ;}

    /** Setter for start value for the MCTrack fit: momentum (y-component).
     */
    void setMCStartMomentumY(float mcStartMomentumY) {m_mcStartMomentumY.push_back(mcStartMomentumY) ;}

    /** Setter for start value for the MCTrack fit: momentum (z-component).
     */
    void setMCStartMomentumZ(float mcStartMomentumZ) {m_mcStartMomentumZ.push_back(mcStartMomentumZ) ;}

    /** Setter for start value for the MCTrack fit: momentum (absolute value).
     */
    void setMCStartMomentum(float mcStartMomentum) {m_mcStartMomentum.push_back(mcStartMomentum) ;}

    /** Setter for start value for the MCTrack fit: curvature (inverse transverse momentum).
     */
    void setMCStartCurv(float mcStartCurv) {m_mcStartCurv.push_back(mcStartCurv) ;}

    /** Setter for the start value for the MCTrack fit: phi angle (track angle in xy plane).
      */
    void setMCStartPhi(float mcStartPhi) {m_mcStartPhi.push_back(mcStartPhi);}

    /** Setter for start value for the MCTrack fit: cotangens theta (pz/pt).
     */
    void setMCStartCotTheta(float mcStartCotTheta) {m_mcStartCotTheta.push_back(mcStartCotTheta) ;}

    /** Setter for start value for the MCTrack fit: theta .
     */
    void setMCStartTheta(float mcStartTheta) {m_mcStartTheta.push_back(mcStartTheta) ;}

    /** Setter for start value for the MCTrack fit: vertex position (x-component).
     */
    void setMCStartPositionX(float mcStartPositionX) {m_mcStartPositionX.push_back(mcStartPositionX) ;}

    /** Setter for start value for the MCTrack fit: vertex position (y-component).
     */
    void setMCStartPositionY(float mcStartPositionY) {m_mcStartPositionY.push_back(mcStartPositionY) ;}

    /** Setter for start value for the MCTrack fit: vertex position (z-component).
     */
    void setMCStartPositionZ(float mcStartPositionZ) {m_mcStartPositionZ.push_back(mcStartPositionZ) ;}

    /** Setter for the fit result of the MCTrack fit: momentum (x-component).
      */
    void setMCFitMomentumX(float mcFitMomentumX) {m_mcFitMomentumX.push_back(mcFitMomentumX) ;}

    /** Setter for the fit result of the MCTrack fit: momentum (y-component).
      */
    void setMCFitMomentumY(float mcFitMomentumY) {m_mcFitMomentumY.push_back(mcFitMomentumY) ;}

    /** Setter for the fit result of the MCTrack fit: momentum (z-component).
     */
    void setMCFitMomentumZ(float mcFitMomentumZ) {m_mcFitMomentumZ.push_back(mcFitMomentumZ) ;}

    /** Setter for the fit result of the MCTrack fit: momentum (absolute value).
     */
    void setMCFitMomentum(float mcFitMomentum) {m_mcFitMomentum.push_back(mcFitMomentum) ;}

    /** Setter for the fit result of the MCTrack fit: curvature (inverse transverse momentum).
     */
    void setMCFitCurv(float mcFitCurv) {m_mcFitCurv.push_back(mcFitCurv) ;}

    /** Setter for the fit result the MCTrack fit: phi angle (track angle in xy plane).
      */
    void setMCFitPhi(float mcFitPhi) {m_mcFitPhi.push_back(mcFitPhi);}

    /** Setter for the fit result of the MCTrack fit: cotangens theta (pz/pt).
     */
    void setMCFitCotTheta(float mcFitCotTheta) {m_mcFitCotTheta.push_back(mcFitCotTheta) ;}

    /** Setter for the fit result of the MCTrack fit: theta .
     */
    void setMCFitTheta(float mcFitTheta) {m_mcFitTheta.push_back(mcFitTheta) ;}

    /** Setter for the fit result of the MCTrack fit: vertex position (x-component).
     */
    void setMCFitPositionX(float mcFitPositionX) {m_mcFitPositionX.push_back(mcFitPositionX) ;}

    /** Setter for the fit result of the MCTrack fit: vertex position (y-component).
     */
    void setMCFitPositionY(float mcFitPositionY) {m_mcFitPositionY.push_back(mcFitPositionY) ;}

    /** Setter for the fit result of the MCTrack fit: vertex position (z-component).
     */
    void setMCFitPositionZ(float mcFitPositionZ) {m_mcFitPositionZ.push_back(mcFitPositionZ) ;}

    /** Setter for the number of hits used in the MCTrack fit.
      */
    void setNMCHits(int nMCHits) {m_nMCHits.push_back(nMCHits) ;}

    /** Setter for the pdg hypothesis of MCTrack fit.
      */
    void setMCFitPDG(int mcFitPDG) {m_mcFitPDG.push_back(mcFitPDG) ;}

    /** Setter for the Chi2 of the MCTrack fit.
      */
    void setMCFitChi2(float mcFitChi2) {m_mcFitChi2.push_back(mcFitChi2) ;}

    /** Setter for the probability value of the MCTrack fit.
      */
    void setMCFitPValue(float mcFitPValue) {m_mcFitPValue.push_back(mcFitPValue) ;}

    /** Setter for the successful fit of the MCTrack.
      */
    void setMCSuccessFit(int mcSuccessFit) {m_mcSuccessFit.push_back(mcSuccessFit) ;}

    /** Setter for the successful extrapolation of the MCTrack.
      */
    void setMCSuccessExtrap(int mcSuccessExtrap) {m_mcSuccessExtrap.push_back(mcSuccessExtrap) ;}

    /** Setter for the fit result of the MCTrack fit: momentum error (x-component).
        */
    void setMCFitMomentumXErr(float mcFitMomentumXErr) {m_mcFitMomentumXErr.push_back(mcFitMomentumXErr) ;}

    /** Setter for the fit result of the MCTrack fit: momentum error (y-component).
      */
    void setMCFitMomentumYErr(float mcFitMomentumYErr) {m_mcFitMomentumYErr.push_back(mcFitMomentumYErr) ;}

    /** Setter for the fit result of the MCTrack fit: momentum error (z-component).
     */
    void setMCFitMomentumZErr(float mcFitMomentumZErr) {m_mcFitMomentumZErr.push_back(mcFitMomentumZErr) ;}

    /** Setter for the fit result of the MCTrack fit: position error (x-component).
        */
    void setMCFitPositionXErr(float mcFitPositionXErr) {m_mcFitPositionXErr.push_back(mcFitPositionXErr) ;}

    /** Setter for the fit result of the MCTrack fit: position error (y-component).
      */
    void setMCFitPositionYErr(float mcFitPositionYErr) {m_mcFitPositionYErr.push_back(mcFitPositionYErr) ;}

    /** Setter for the fit result of the MCTrack fit: position error (z-component).
     */
    void setMCFitPositionZErr(float mcFitPositionZErr) {m_mcFitPositionZErr.push_back(mcFitPositionZErr) ;}

    /** Setter for the MCTrack Ids for the given MCParticle.
     */
    void setMCFitTracks(std::vector<int> mcFitTracks) {m_mcFitTracks = mcFitTracks;}

    /** Setter for the number of MCTracks.
     */
    void setNMCFitTracks(int nMCFitTracks) {m_nMCFitTracks.push_back(nMCFitTracks);}

    /** Getter for the number of <CTracks.
     */
    std::vector<int> getNMCFitTracks() {return m_nMCFitTracks; }

    /** Getter for the MCTrack Ids for the given MCParticle.
     */
    std::vector<int> getMCFitTracks() { return m_mcFitTracks; }


    /** Getter for the start value of MCTrack fit: momentum (x-component).
     */
    std::vector<float> getMCStartMomentumX() {return m_mcStartMomentumX; }

    /** Getter for the start value of MCTrack fit: momentum (y-component).
     */
    std::vector<float> getMCStartMomentumY() {return m_mcStartMomentumY; }

    /** Getter for the start value of MCTrack fit: momentum (z-component).
     */
    std::vector<float> getMCStartMomentumZ() {return m_mcStartMomentumZ; }

    /** Getter for the start value of MCTrack fit: momentum (absolute value).
     */
    std::vector<float> getMCStartMomentum() {return m_mcStartMomentum; }

    /** Getter for the start value of MCTrack fit: curvature (inverse transverse momentum).
     */
    std::vector<float> getMCStartCurv() {return m_mcStartCurv; }

    /** Getter for the start value of MCTrack fit: phi .
     */
    std::vector<float> getMCStartPhi() {return m_mcStartPhi; }

    /** Getter for the start value of MCTrack fit: cotangens theta (pz/pt).
     */
    std::vector<float> getMCStartCotTheta() {return m_mcStartCotTheta; }

    /** Getter for the start value of MCTrack fit: theta .
     */
    std::vector<float> getMCStartTheta() {return m_mcStartTheta; }

    /** Getter for the fit result of MCTrack fit: momentum (x-component).
     */
    std::vector<float> getMCFitMomentumX() {return m_mcFitMomentumX; }

    /** Getter for the fit result of MCTrack fit: momentum (y-component).
     */
    std::vector<float> getMCFitMomentumY() {return m_mcFitMomentumY; }

    /** Getter for the fit result of MCTrack fit: momentum (z-component).
     */
    std::vector<float> getMCFitMomentumZ() {return m_mcFitMomentumZ; }

    /** Getter for the fit result of MCTrack fit: momentum (absolute value).
     */
    std::vector<float> getMCFitMomentum() {return m_mcFitMomentum; }

    /** Getter for the fit result of MCTrack fit: curvature (inverse transverse momentum).
     */
    std::vector<float> getMCFitCurv() {return m_mcFitCurv; }

    /** Getter for the fit result of MCTrack fit: phi.
     */
    std::vector<float> getMCFitPhi() {return m_mcFitPhi; }

    /** Getter for the fit result of MCTrack fit: cotangens theta (pz/pt).
     */
    std::vector<float> getMCFitCotTheta() {return m_mcFitCotTheta; }

    /** Getter for the fit result of MCTrack fit: theta.
     */
    std::vector<float> getMCFitTheta() {return m_mcFitTheta; }


    //------------------------------------------------------------------------------------------------------------

    /** Setter for start value for the PRTrack fit: momentum (x-component).
     */
    void setPRStartMomentumX(float prStartMomentumX) {m_prStartMomentumX.push_back(prStartMomentumX) ;}

    /** Setter for start value for the PRTrack fit: momentum (y-component).
     */
    void setPRStartMomentumY(float prStartMomentumY) {m_prStartMomentumY.push_back(prStartMomentumY) ;}

    /** Setter for start value for the PRTrack fit: momentum (z-component).
     */
    void setPRStartMomentumZ(float prStartMomentumZ) {m_prStartMomentumZ.push_back(prStartMomentumZ) ;}

    /** Setter for start value for the PRTrack fit: momentum (absolute value).
     */
    void setPRStartMomentum(float prStartMomentum) {m_prStartMomentum.push_back(prStartMomentum) ;}

    /** Setter for start value for the PRTrack fit: curvature (inverse transverse momentum).
     */
    void setPRStartCurv(float prStartCurv) {m_prStartCurv.push_back(prStartCurv) ;}

    /** Setter for the start value for the PRTrack fit: phi angle (track angle in xy plane).
      */
    void setPRStartPhi(float prStartPhi) {m_prStartPhi.push_back(prStartPhi);}

    /** Setter for start value for the PRTrack fit: cotangens theta (pz/pt).
     */
    void setPRStartCotTheta(float prStartCotTheta) {m_prStartCotTheta.push_back(prStartCotTheta) ;}

    /** Setter for start value for the PRTrack fit: theta .
     */
    void setPRStartTheta(float prStartTheta) {m_prStartTheta.push_back(prStartTheta) ;}

    /** Setter for start value for the PRTrack fit: vertex position (x-component).
     */
    void setPRStartPositionX(float prStartPositionX) {m_prStartPositionX.push_back(prStartPositionX) ;}

    /** Setter for start value for the PRTrack fit: vertex position (y-component).
     */
    void setPRStartPositionY(float prStartPositionY) {m_prStartPositionY.push_back(prStartPositionY) ;}

    /** Setter for start value for the PRTrack fit: vertex position (z-component).
     */
    void setPRStartPositionZ(float prStartPositionZ) {m_prStartPositionZ.push_back(prStartPositionZ) ;}

    /** Setter for the fit result of the PRTrack fit: momentum (x-component).
      */
    void setPRFitMomentumX(float prFitMomentumX) {m_prFitMomentumX.push_back(prFitMomentumX) ;}

    /** Setter for the fit result of the PRTrack fit: momentum (y-component).
      */
    void setPRFitMomentumY(float prFitMomentumY) {m_prFitMomentumY.push_back(prFitMomentumY) ;}

    /** Setter for the fit result of the PRTrack fit: momentum (z-component).
     */
    void setPRFitMomentumZ(float prFitMomentumZ) {m_prFitMomentumZ.push_back(prFitMomentumZ) ;}

    /** Setter for the fit result of the PRTrack fit: momentum (absolute value).
     */
    void setPRFitMomentum(float prFitMomentum) {m_prFitMomentum.push_back(prFitMomentum) ;}

    /** Setter for the fit result of the PRTrack fit: curvature (inverse transverse momentum).
     */
    void setPRFitCurv(float prFitCurv) {m_prFitCurv.push_back(prFitCurv) ;}

    /** Setter for the fit result the PRTrack fit: phi angle (track angle in xy plane).
      */
    void setPRFitPhi(float prFitPhi) {m_prFitPhi.push_back(prFitPhi);}

    /** Setter for the fit result of the PRTrack fit: cotangens theta (pz/pt).
     */
    void setPRFitCotTheta(float prFitCotTheta) {m_prFitCotTheta.push_back(prFitCotTheta) ;}

    /** Setter for the fit result of the PRTrack fit: theta .
     */
    void setPRFitTheta(float prFitTheta) {m_prFitTheta.push_back(prFitTheta) ;}

    /** Setter for the fit result of the PRTrack fit: vertex position (x-component).
     */
    void setPRFitPositionX(float prFitPositionX) {m_prFitPositionX.push_back(prFitPositionX) ;}

    /** Setter for the fit result of the PRTrack fit: vertex position (y-component).
     */
    void setPRFitPositionY(float prFitPositionY) {m_prFitPositionY.push_back(prFitPositionY) ;}

    /** Setter for the fit result of the PRTrack fit: vertex position (z-component).
     */
    void setPRFitPositionZ(float prFitPositionZ) {m_prFitPositionZ.push_back(prFitPositionZ) ;}

    /** Setter for the number of hits used in the PRTrack fit.
      */
    void setNPRHits(int nPRHits) {m_nPRHits.push_back(nPRHits) ;}

    /** Setter for the pdg hypothesis of PRTrack fit.
      */
    void setPRFitPDG(int prFitPDG) {m_prFitPDG.push_back(prFitPDG) ;}

    /** Setter for the Chi2 of the PRTrack fit.
      */
    void setPRFitChi2(float prFitChi2) {m_prFitChi2.push_back(prFitChi2) ;}

    /** Setter for the probability value of the PRTrack fit.
      */
    void setPRFitPValue(float prFitPValue) {m_prFitPValue.push_back(prFitPValue) ;}

    /** Setter for the successful fit of the PRTrack.
      */
    void setPRSuccessFit(int prSuccessFit) {m_prSuccessFit.push_back(prSuccessFit) ;}

    /** Setter for the successful extrapolation of the PRTrack.
      */
    void setPRSuccessExtrap(int prSuccessExtrap) {m_prSuccessExtrap.push_back(prSuccessExtrap) ;}

    /** Setter for purity of the PRTracks.
     */
    void setPRPurity(double prPurity) {m_prPurity.push_back(prPurity); }

    /** Setter for the fit result of the PRTrack fit: momentum error (x-component).
        */
    void setPRFitMomentumXErr(float prFitMomentumXErr) {m_prFitMomentumXErr.push_back(prFitMomentumXErr) ;}

    /** Setter for the fit result of the PRTrack fit: momentum error (y-component).
      */
    void setPRFitMomentumYErr(float prFitMomentumYErr) {m_prFitMomentumYErr.push_back(prFitMomentumYErr) ;}

    /** Setter for the fit result of the PRTrack fit: momentum error (z-component).
     */
    void setPRFitMomentumZErr(float prFitMomentumZErr) {m_prFitMomentumZErr.push_back(prFitMomentumZErr) ;}

    /** Setter for the fit result of the PRTrack fit: position error (x-component).
        */
    void setPRFitPositionXErr(float prFitPositionXErr) {m_prFitPositionXErr.push_back(prFitPositionXErr) ;}

    /** Setter for the fit result of the PRTrack fit: position error (y-component).
      */
    void setPRFitPositionYErr(float prFitPositionYErr) {m_prFitPositionYErr.push_back(prFitPositionYErr) ;}

    /** Setter for the fit result of the PRTrack fit: position error (z-component).
     */
    void setPRFitPositionZErr(float prFitPositionZErr) {m_prFitPositionZErr.push_back(prFitPositionZErr) ;}

    /** Setter for the PRTrack Ids for the given PRParticle.
     */
    void setPRFitTracks(std::vector<int> prFitTracks) {m_prFitTracks = prFitTracks;}

    /** Setter for the number of PRTracks.
      */
    void setNPRFitTracks(int nPRFitTracks) {m_nPRFitTracks.push_back(nPRFitTracks);}

    /**Getter for the number of PRTracks.
     */
    std::vector<int> getNPRFitTracks() {return m_nPRFitTracks; }

    /** Getter for the PRTrack Ids for the given PRParticle.
     */
    std::vector<int> getPRFitTracks() { return m_prFitTracks; }


    /** Getter for the start value of PRTrack fit: momentum (x-component).
     */
    std::vector<float> getPRStartMomentumX() {return m_prStartMomentumX; }

    /** Getter for the start value of PRTrack fit: momentum (y-component).
     */
    std::vector<float> getPRStartMomentumY() {return m_prStartMomentumY; }

    /** Getter for the start value of PRTrack fit: momentum (z-component).
     */
    std::vector<float> getPRStartMomentumZ() {return m_prStartMomentumZ; }

    /** Getter for the start value of PRTrack fit: momentum (absolute value).
     */
    std::vector<float> getPRStartMomentum() {return m_prStartMomentum; }

    /** Getter for the start value of PRTrack fit: curvature (inverse transverse momentum).
     */
    std::vector<float> getPRStartCurv() {return m_prStartCurv; }

    /** Getter for the start value of PRTrack fit: phi .
     */
    std::vector<float> getPRStartPhi() {return m_prStartPhi; }

    /** Getter for the start value of PRTrack fit: cotangens theta (pz/pt).
     */
    std::vector<float> getPRStartCotTheta() {return m_prStartCotTheta; }

    /** Getter for the start value of PRTrack fit: theta .
     */
    std::vector<float> getPRStartTheta() {return m_prStartTheta; }


    /** Getter for the fit result of PRTrack fit: momentum (x-component).
     */
    std::vector<float> getPRFitMomentumX() {return m_prFitMomentumX; }

    /** Getter for the fit result of PRTrack fit: momentum (y-component).
     */
    std::vector<float> getPRFitMomentumY() {return m_prFitMomentumY; }

    /** Getter for the fit result of PRTrack fit: momentum (z-component).
     */
    std::vector<float> getPRFitMomentumZ() {return m_prFitMomentumZ; }

    /** Getter for the fit result of PRTrack fit: momentum (absolute value).
     */
    std::vector<float> getPRFitMomentum() {return m_prFitMomentum; }

    /** Getter for the fit result of PRTrack fit: curvature (inverse transverse momentum).
     */
    std::vector<float> getPRFitCurv() {return m_prFitCurv; }

    /** Getter for the fit result of PRTrack fit: phi.
     */
    std::vector<float> getPRFitPhi() {return m_prFitPhi; }

    /** Getter for the fit result of PRTrack fit: cotangens theta (pz/pt).
     */
    std::vector<float> getPRFitCotTheta() {return m_prFitCotTheta; }

    /** Getter for the fit result of PRTrack fit: theta.
     */
    std::vector<float> getPRFitTheta() {return m_prFitTheta; }


  private:

    //-----------------------------------------------------------------------
    float m_mcMomentumX;                         /**< Momentum of the MCParticle (x-component) */
    float m_mcMomentumY;                         /**< Momentum of the MCParticle (y-component) */
    float m_mcMomentumZ;                         /**< Momentum of the MCParticle (z-component) */
    float m_mcMomentum;                          /**< Momentum of the MCParticle (absolute value) */
    float m_mcCurv;                              /**< Curvature of the MCParticle (inverse transverse momentum) */
    float m_mcPhi;                               /**< Phi of the MCParticle */
    float m_mcCotTheta;                          /**< Cotangens theta of the MCParticle (pz/pt) */
    float m_mcTheta;                             /**< Theta of the MCParticle */
    int m_mcPDG;                                 /**< PDG of the MCParticle */
    float m_mcPositionX;                         /**< Vertex of the MCParticle (x-component) */
    float m_mcPositionY;                         /**< Vertex of the MCParticle (y-component) */
    float m_mcPositionZ;                         /**< Vertex of the MCParticle (z-component) */

    //-------------------------------------------------------------------------
    std::vector<int>   m_nMCFitTracks;           /**< Number of fitted MCTracks */
    std::vector<float> m_mcStartMomentumX;       /**< Start values for the MCTrack fit: momentum (x-component) */
    std::vector<float> m_mcStartMomentumY;       /**< Start values for the MCTrack fit: momentum (y-component) */
    std::vector<float> m_mcStartMomentumZ;       /**< Start values for the MCTrack fit: momentum (z-component) */
    std::vector<float> m_mcStartMomentum;        /**< Start values for the MCTrack fit: momentum (absolute value) */
    std::vector<float> m_mcStartCurv;            /**< Start values for the MCTrack fit: curvature (inverse transverse momentum) */
    std::vector<float> m_mcStartPhi;             /**< Start values for the MCTrack fit: phi */
    std::vector<float> m_mcStartCotTheta;        /**< Start values for the MCTrack fit: cotangens theta (pz/pt) */
    std::vector<float> m_mcStartTheta;           /**< Start values for the MCTrack fit: theta */

    std::vector<float> m_mcStartPositionX;       /**< Start values for the MCTrack fit: vertex (x-component) */
    std::vector<float> m_mcStartPositionY;       /**< Start values for the MCTrack fit: vertex (y-component) */
    std::vector<float> m_mcStartPositionZ;       /**< Start values for the MCTrack fit: vertex (z-component) */

    std::vector<float> m_mcFitMomentumX;         /**< Fit result for the MCTrack fit: momentum (x-component) */
    std::vector<float> m_mcFitMomentumY;         /**< Fit result for the MCTrack fit: momentum (y-component) */
    std::vector<float> m_mcFitMomentumZ;         /**< Fit result for the MCTrack fit: momentum (z-component) */
    std::vector<float> m_mcFitMomentum;          /**< Fit result for the MCTrack fit: momentum (absolute value) */
    std::vector<float> m_mcFitCurv;              /**< Fit result for the MCTrack fit: curvature (inverse transverse momentum) */
    std::vector<float> m_mcFitPhi  ;             /**< Fit result for the MCTrack fit: phi */
    std::vector<float> m_mcFitCotTheta;          /**< Fit result for the MCTrack fit: cotangens theta(pz/pt) */
    std::vector<float> m_mcFitTheta;             /**< Fit result for the MCTrack fit: theta */
    std::vector<float> m_mcFitPositionX;         /**< Fit result for the MCTrack fit: vertex (x-component) */
    std::vector<float> m_mcFitPositionY;         /**< Fit result for the MCTrack fit: vertex (y-component) */
    std::vector<float> m_mcFitPositionZ;         /**< Fit result for the MCTrack fit: vertex (z-component) */

    std::vector<int> m_nMCHits;                  /**< Number of hits in the fitted MCTrack */
    std::vector<int> m_mcFitPDG;                 /**< PDG value of the fitted MCTrack */
    std::vector<float> m_mcFitChi2;              /**< Chi2 value of the MCTrack fit */
    std::vector<float> m_mcFitPValue;            /**< Probability value of the MCTrack fit */
    std::vector<int> m_mcSuccessFit;             /**< Marker for successful fit of the MCTrack */
    std::vector<int> m_mcSuccessExtrap;          /**< Marker of successful extrapolation of the MCTrack fit */

    std::vector<float> m_mcFitMomentumXErr;      /**< Fit result for the MCTrack fit: momentum error (x-component) */
    std::vector<float> m_mcFitMomentumYErr;      /**< Fit result for the MCTrack fit: momentum error (y-component) */
    std::vector<float> m_mcFitMomentumZErr;      /**< Fit result for the MCTrack fit: momentum error (z-component) */
    std::vector<float> m_mcFitPositionXErr;      /**< Fit result for the MCTrack fit: vertex error (x-component) */
    std::vector<float> m_mcFitPositionYErr;      /**< Fit result for the MCTrack fit: vertex error (y-component) */
    std::vector<float> m_mcFitPositionZErr;      /**< Fit result for the MCTrack fit: vertex error (z-component) */

    std::vector <int> m_mcFitTracks;             /**< MCTracks Ids for the given MCParticle */

    //---------------------------------------------------------------------------
    std::vector<int>   m_nPRFitTracks;           /**< Number of fitted PRTracks */
    std::vector<float> m_prStartMomentumX;       /**< Start values for the PRTrack fit: momentum (x-component) */
    std::vector<float> m_prStartMomentumY;       /**< Start values for the PRTrack fit: momentum (y-component) */
    std::vector<float> m_prStartMomentumZ;       /**< Start values for the PRTrack fit: momentum (z-component) */
    std::vector<float> m_prStartMomentum;        /**< Start values for the PRTrack fit: momentum (absolute value) */
    std::vector<float> m_prStartCurv;            /**< Start values for the PRTrack fit: curvature (inverse transverse momentum) */
    std::vector<float> m_prStartPhi  ;           /**< Start values for the PRTrack fit: phi */
    std::vector<float> m_prStartCotTheta;        /**< Start values for the PRTrack fit: cotangens theta (pz/pt) */
    std::vector<float> m_prStartTheta;           /**< Start values for the PRTrack fit: theta */

    std::vector<float> m_prStartPositionX;       /**< Start values for the PRTrack fit: vertex (x-component) */
    std::vector<float> m_prStartPositionY;       /**< Start values for the PRTrack fit: vertex (y-component) */
    std::vector<float> m_prStartPositionZ;       /**< Start values for the PRTrack fit: vertex (z-component) */

    std::vector<float> m_prFitMomentumX;         /**< Fit result for the PRTrack fit: momentum (x-component) */
    std::vector<float> m_prFitMomentumY;         /**< Fit result for the PRTrack fit: momentum (y-component) */
    std::vector<float> m_prFitMomentumZ;         /**< Fit result for the PRTrack fit: momentum (z-component) */
    std::vector<float> m_prFitMomentum;          /**< Fit result for the PRTrack fit: momentum (absolute value) */
    std::vector<float> m_prFitCurv;              /**< Fit result for the PRTrack fit: curvature (inverse transverse momentum) */
    std::vector<float> m_prFitPhi  ;             /**< Fit result for the PRTrack fit: phi */
    std::vector<float> m_prFitCotTheta;          /**< Fit result for the PRTrack fit: cotangens theta(pz/pt) */
    std::vector<float> m_prFitTheta;             /**< Fit result for the PRTrack fit: theta */

    std::vector<float> m_prFitPositionX;         /**< Fit result for the PRTrack fit: vertex (x-component) */
    std::vector<float> m_prFitPositionY;         /**< Fit result for the PRTrack fit: vertex (y-component) */
    std::vector<float> m_prFitPositionZ;         /**< Fit result for the PRTrack fit: vertex (z-component) */

    std::vector<int> m_nPRHits;                  /**< Number of hits in the fitted PRTrack */
    std::vector<int> m_prFitPDG;                 /**< PDG value of the fitted PRTrack */
    std::vector<float> m_prFitChi2;              /**< Chi2 value of the PRTrack fit */
    std::vector<float> m_prFitPValue;            /**< Probability value of the PRTrack fit */
    std::vector<int> m_prSuccessFit;             /**< Marker for successful fit of the PRTrack */
    std::vector<int> m_prSuccessExtrap;          /**< Marker of successful extrapolation of the PRTrack fit */
    std::vector<float> m_prPurity;               /**< Purity of the PRTrack ((hits from one MCParticle/total nr of hits) * 100) */

    std::vector<float> m_prFitMomentumXErr;      /**< Fit result for the PRTrack fit: momentum error (x-component) */
    std::vector<float> m_prFitMomentumYErr;      /**< Fit result for the PRTrack fit: momentum error (y-component) */
    std::vector<float> m_prFitMomentumZErr;      /**< Fit result for the PRTrack fit: momentum error (z-component) */
    std::vector<float> m_prFitPositionXErr;      /**< Fit result for the PRTrack fit: vertex error (x-component) */
    std::vector<float> m_prFitPositionYErr;      /**< Fit result for the PRTrack fit: vertex error (y-component) */
    std::vector<float> m_prFitPositionZErr;      /**< Fit result for the PRTrack fit: vertex error (z-component) */

    std::vector <int> m_prFitTracks;             /** PRTracks Ids for the given MCParticle */


    ClassDef(TrackingOutput, 1);
  };

}



#endif /* TRACKINGOUTPUT_H_ */
