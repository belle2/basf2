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
#include <vector>

namespace Belle2 {

  /**Class to store the variables for event selection in physics trigger*/
  class PhysicsTriggerInformation: public RelationsObject {

  public:

    PhysicsTriggerInformation(): m_ngoodTracks(0), m_ngoodECLClusters(0), m_Evis(0.), m_Esum(0.), m_AngleTT(0.), m_AngleGG(0.), m_P1(0.), m_P2(0.), m_E1(0.), m_E2(0.) {}

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
     *set T1's momentum
     */
    void setP1(float P1) {m_P1 = P1;}

    /**
     *set T2's momentum
     */
    void setP2(float P2) {m_P2 = P2;}

    /**
     *set Dr
     */
    void setDr(float Dr) {m_dr.push_back(Dr);}

    /**
     *set Dz
     */
    void setDz(float Dz) {m_dz.push_back(Dz);}

    /**
     *set the transverse momentum of the track
     */
    void setPt(float Pt) {m_pt.push_back(Pt);}

    /**
     *set the momentum of track
     */
    void setP(float P) {m_p.push_back(P);}

    /**
     *set polar angle of track
     */
    void setCosTheta(float CosTheta) {m_costheta.push_back(CosTheta);}


    /**
     *set azimuth angle of track
     */
    void setPhi(float phi) {m_phi.push_back(phi);}


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

    /**
     *set chrage of track
     */
    void setCharge(int Charge) {m_Charge.push_back(Charge);}

    /**
     *set cluster energy
     */
    void setECLClusterE(float ECLClusterE) {m_ECLClusterE.push_back(ECLClusterE);}

    /**
     *set polar angle of cluster
     */
    void setECLClusterTheta(float ECLClusterTheta) {m_ECLClusterTheta.push_back(ECLClusterTheta);}

    /**
     *set azimuth angle of cluster
     */
    void setECLClusterPhi(float ECLClusterPhi) {m_ECLClusterPhi.push_back(ECLClusterPhi);}

    /**
     *set timing of cluster
     */
    void setECLClusterTiming(float ECLClusterTiming) {m_ECLClusterTiming.push_back(ECLClusterTiming);}

    /**
     *set use user custom or not
     */
    void setUserCustomValue(int userCustomOpen) {m_userCustomOpen = userCustomOpen;}

//getters

    /**
     *return the value of user custom
     */
    int getUserCustomValue()const {return m_userCustomOpen;}

    /**
     *return the number of tracks
     */
    int getNGoodTrack() const {return m_ngoodTracks;}

    /**
     *return the number of clusters
     */
    int getNGoodECLClusters() const {return m_ngoodECLClusters;}

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
     *return Dr
     */
    std::vector<float> getDr()const {return m_dr;}

    /**
     *return Dz
     */
    std::vector<float> getDz()const {return m_dz;}

    /**
     *return the transverse momentum of tracks
     */
    std::vector<float> getPt()const {return m_pt;}

    /**
     *return the momentum of tracks
     */
    std::vector<float> getP()const {return m_p;}

    /**
     *return polar angle of tracks
     */
    std::vector<float> getCosTheta()const {return m_costheta;}

    /**
     *return azimuth angle of tracks
     */
    std::vector<float> getPhi()const {return m_phi;}


    /**
     *return angle between two tracks in the rest frame
     */
    float getAngleTT()const {return m_AngleTT;}

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

    /**
     *return the charge of tracks
     */
    std::vector<int> getCharge()const {return m_Charge;}

    /**
     *return the cluster energy
     */
    std::vector<float> getECLClusterE()const {return m_ECLClusterE;}

    /**
     *return the polar angle of cluster
     */
    std::vector<float> getECLClusterTheta()const {return m_ECLClusterTheta;}

    /**
     *return the azimuth angle of cluster
     */
    std::vector<float> getECLClusterPhi()const {return m_ECLClusterPhi;}

    /**
     *return the timing of the cluster
     */
    std::vector<float> getECLClusterTiming()const {return m_ECLClusterTiming;}


  private:

    int m_userCustomOpen;/**switch of custom seletion cretria for user*/

    int m_ngoodTracks;/**the number of good tracks*/
    int m_ngoodECLClusters; /*the number of good ECL clusters*/
    float m_Pzvis; /**the total absolute of Pz momenta*/
    float m_Evis; /**the total visible energy*/
    std::vector<float> m_dr; /**dr*/
    std::vector<float> m_dz; /**dz*/
    std::vector<float> m_pt; /**transverse momentum of tracks*/
    std::vector<float> m_p; /**track momenta*/
    std::vector<float> m_costheta; /**the polar angle of tracks*/
    std::vector<float> m_phi; /**the azimuth angle of tracks*/
    std::vector<int> m_Charge; /**chage*/
    std::vector<float> m_ECLClusterE; /**the cluster energy*/
    std::vector<float> m_ECLClusterTheta; /**the polar angle of cluster*/
    std::vector<float> m_ECLClusterPhi; /**the azimuth angle of the cluster*/
    std::vector<float> m_ECLClusterTiming; /**the timing of the cluster*/
    float m_Esum; /**the sum of cluster energy*/
    float m_AngleTT; /*the angle between the first and second largest momentum tracks in the rest frame*/
    float m_AngleGG;/**the angle between the first and second energetic cluster*/
    float m_P1; /**the largest momentum of the tracks*/
    float m_P2; /**the second largest momentum of the tracks*/
    float m_E1; /**the largest energetic cluster*/
    float m_E2; /**the second largest energtic cluster*/


    ClassDef(PhysicsTriggerInformation, 1)
  };

}
