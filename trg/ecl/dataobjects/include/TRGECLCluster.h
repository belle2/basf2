//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLHit.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <framework/datastore/RelationsObject.h>


namespace Belle2 {

  //! Example Detector
  class TRGECLCluster : public RelationsObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_ClusterId;

    //! The # of TC in Cluster.
    int m_NofTCinCluster;

    //! The maximum TC Id in Cluster.
    int m_MaxTCId;
    //! The Theta Id of maximum TC in Cluster.
    int m_MaxThetaId;
    //! The Phi Id of maximum TC in Cluster.
    int m_MaxPhiId;


    //! icn_flag -  perfect icn : 1 , icn : 2
    /*!
      int m_ICNFlag;
    */
    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //! energy weighted position X
    double m_X;
    //! energy weighted position Y
    double m_Y;
    //! energy weighted position Z
    double m_Z;


    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set cell id
    void setClusterId(int clusterId) { m_ClusterId = clusterId; }

    //! The method to set maximum energy TC ID
    void setMaxTCId(int maxtcid) { m_MaxTCId  = maxtcid; }
    //! The method to set Theta Id of maximum TC in Cluster.
    void setMaxThetaId(int maxthetaid) { m_MaxThetaId  = maxthetaid; }
    //! The method to set Phi Id of maximum TC in Cluster.
    void setMaxPhiId(int maxphiid) { m_MaxPhiId  = maxphiid; }


    //! The method to set # of output per TC
    void setNofTCinCluster(int NofTC) { m_NofTCinCluster = NofTC; }


    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }


    //! Set Energy weighted position X
    void setPositionX(double X) { m_X = X; }
    //! Set Energy weighted position Y
    void setPositionY(double Y) { m_Y = Y; }
    //! Set Energy weighted position Z
    void setPositionZ(double Z) { m_Z = Z; }



    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getClusterId() const { return m_ClusterId; }
    //! The method to get cell id
    int getNofTCinCluster() const { return m_NofTCinCluster; }
    //! The method to get the Maximum TC  id
    int getMaxTCId() const { return m_MaxTCId; }
    //! The method to set Theta Id of maximum TC in Cluster.
    int getMaxThetaId() { return m_MaxThetaId ; }
    //! The method to set Phi Id of maximum TC in Cluster.
    int getMaxPhiId() { return m_MaxPhiId  ;}

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}

    //! The method to get hit average time
    //! Get Energy weighted position X
    double getPositionX() const {return m_X;}
    //! Set Energy weighted position Y
    double getPositionY() const {return m_Y;}
    //! Set Energy weighted position Z
    double getPositionZ() const {return m_Z;}


    // Empty constructor
    //! Recommended for ROOT IO
    TRGECLCluster()
    {
      m_eventId = 0;
      m_ClusterId = 0;
      m_edep = 0;
      m_TimeAve = 0;
      m_NofTCinCluster = 0;
      m_X = 0;
      m_Y = 0;
      m_Z = 0;
      m_MaxTCId = 0;
      m_MaxThetaId = 0;
      m_MaxPhiId = 0;

    }
    //! The Class title
    ClassDef(TRGECLCluster, 2); /*< the class title */

  };

} // end namespace Belle2

//#endif
