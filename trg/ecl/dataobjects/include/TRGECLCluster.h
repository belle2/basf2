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
#ifndef TRGECLCluster_H
#define TRGECLCluster_H


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLCluster : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_ClusterId;

    //! The # of TC in Cluster.
    int m_NofTCinCluster;

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

    }

    //! Useful Constructor
    TRGECLCluster(
      int eventId,
      int clusterId,
      double edep,
      double TimeAve,
      int noftchit,
      int x,
      int y,
      int z
    )
    {
      m_eventId = eventId;
      m_ClusterId = clusterId;
      m_edep = edep;
      m_TimeAve = TimeAve;
      m_NofTCinCluster = noftchit;
      m_X = x;
      m_Y = y;
      m_Z = z;


    }
    //! The Class title
    ClassDef(TRGECLCluster, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
