/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

//#include <skim/hlt/dataobjects/PhysicsTriggerInformation.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**Class to store the variables for event selection in physics trigger*/
  class PhysicsTriggerInformation: public RelationsObject {

  public:
    PhysicsTriggerInformation();
//    PhysicsTriggerInformation(): m_userCustomOpen(0), m_ngoodTracks(0), m_ngoodECLClusters(0), m_Pzvis(0.), m_Evis(0.), m_Esum(0.),
    //    m_MaxAngleTT(-10.), m_AngleTT(-10.), m_AngleGG(-10.), m_P1(0.), m_P2(0.), m_E1(0.), m_E2(0.), m_nKLMClusters(0), m_ptsummary(0),m_ntrg(0){}

    ~PhysicsTriggerInformation() {}

//setters

    /**
     *set the number of tracks
     */
    void setNGoodTrack(int ngoodTracks) {m_ngoodTracks = ngoodTracks;}

    /**
     *set the number of clusters
     */
    void setNGoodECLClusters(int ngoodECLClusters) {m_ngoodECLClusters = ngoodECLClusters;}

    /**
     *set the number of KLM clusters
     */
    void setNKLMClusters(int nKLMClusters) {m_nKLMClusters = nKLMClusters;}

    /**
     *set the visible energy
     */
    void setEvis(float Evis) {m_Evis = Evis;}

    /**
     *set the visible Pz
     */
    void setPzvis(float Pzvis) {m_Pzvis = Pzvis;}

    /**
     *set the sum of cluster energy
     */
    void setEsum(float Esum) {m_Esum = Esum;}

    /**
     *set the angle between T1 and T2
     *T1 is the track with the largest momentum
     *T2 is the track with the second largest momentum
     */
    void setAngleTT(float AngleTT) {m_AngleTT = AngleTT;}

    /**
     *set the maximum angle tetween two charged tracks
     */
    void setMaxAngleTT(float MaxAngleTT) {m_MaxAngleTT = MaxAngleTT;}

    /**
     *set T1's momentum
     */
    void setP1(float P1) {m_P1 = P1;}

    /**
     *set T2's momentum
     */
    void setP2(float P2) {m_P2 = P2;}

    /**
     *set angle between G1 ang G2
     *G1 is the largest energetic cluster
     *G2 is the second largest energetic cluster
     */
    void setAngleGG(float AngleGG) {m_AngleGG = AngleGG;}

    /**
     *set G1 energy
     */
    void setE1(float E1) {m_E1 = E1;}

    /**
     *set G2 energy
     */
    void setE2(float E2) {m_E2 = E2;}


    /**set the summary of physics trigger results*/
    void setPTSummary(int bitMask) {m_ptsummary |= bitMask;}

    /**set the sub trigger result*/
    void setsubTrgResult(int i, int res) {m_trgresult[i] |= res;}

    /**set the sub triggers*/
    void setnsubTrg(int i) {m_nsubtrg[i]++;}

    /**set the total triggers*/
    void setntotTrg(int i = 1) {m_ntottrg += i;}
    //getters

    /**
     *return the number of tracks
     */
    int getNGoodTrack() const {return m_ngoodTracks;}

    /**
     *return the number of clusters
     */
    int getNGoodECLClusters() const {return m_ngoodECLClusters;}

    /**
     *return the number of KLM layers with hits
     */
    int getnKLMClusters() const {return m_nKLMClusters;}

    /**
     *return the visible energy
     */
    float getEvis() const {return m_Evis;}

    /**
     *return the sum of cluster energy
     */
    float getEsum()const {return m_Esum;}

    /**
     *return the visible Pz
     */
    float getPzvis()const {return m_Pzvis;}

    /**
     *return angle between two tracks with the largest momentum in the rest frame
     */
    float getAngleTT()const {return m_AngleTT;}


    /**
     *return the maximum angle between two charged tracks in the rest frame
     */
    float getMaxAngleTT()const {return m_MaxAngleTT;}

    /**
     *return the largest momentum
     */
    float getP1()const {return m_P1;}

    /**
     *return the second largest momentum
     */
    float getP2()const {return m_P2;}

    /**
     *return the angle bwteen two clusters
     */
    float getAngleGG()const {return m_AngleGG;}

    /**
     *return the largest cluster
     */
    float getE1()const {return m_E1;}

    /**
     *return the seond largest cluster
     */
    float getE2()const {return m_E2;}

    /**return the summary of trigger results*/
    int getPTSummary() const {return m_ptsummary;}

    /**return the sub summary of trigger results*/
    int getsubTrgResult(int i) const {return m_trgresult[i];}

    /**return the number of sub triggers*/
    int getnsubTrg(int i) const {return m_nsubtrg[i];}

    /**return the number of total triggers*/
    int getntotTrg() const {return m_ntottrg;}

  private:

    /**the number of good tracks*/
    int m_ngoodTracks;

    /**the number of good ECL clusters*/
    int m_ngoodECLClusters;

    /**the total absolute of Pz momenta*/
    float m_Pzvis;

    /**the total visible energy*/
    float m_Evis;

    /**the sum of cluster energy*/
    float m_Esum;

    /**the maximum angle between the charged tracks in the rest frame*/
    float m_MaxAngleTT;

    /**the angle between the first and second largest momentum tracks in the rest frame*/
    float m_AngleTT;

    /**the angle between the first and second energetic cluster*/
    float m_AngleGG;

    /**the largest momentum of the tracks*/
    float m_P1;

    /**the second largest momentum of the tracks*/
    float m_P2;

    /**the largest energetic cluster*/
    float m_E1;

    /**the second largest energtic cluster*/
    float m_E2;

    /**the number of KLM clusters*/
    int  m_nKLMClusters;

    /**the summary of physics trigger*/
    int m_ptsummary;
    /**the number of triggers*/
    int m_ntottrg;
    /**the largest trigger index*/
    int m_trgresult[16];
    /**the tag */
    int m_nsubtrg[16];

    ClassDef(PhysicsTriggerInformation, 2)
  };

}
