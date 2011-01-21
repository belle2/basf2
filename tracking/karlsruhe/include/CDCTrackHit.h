/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#ifndef CDCTRACKHIT_H
#define CDCTRACKHIT_H

#include "TObject.h"
#include<cdc/hitcdc/HitCDC.h>


namespace Belle2 {

  /** CDC Hit Class used for pattern recognition, will soon be changed an updated. */
  class CDCTrackHit : public HitCDC {
  public:

    /*! Empty constructor. */
    CDCTrackHit();

    /*! Copy Constructor to cast CDCTrackHit objects from HitCDC objects. */
    CDCTrackHit(HitCDC &hitcdc);

    /*! Destructor. */
    ~CDCTrackHit();

    /*!Perfoms type casting: HitCDC->CDCTrackHit an sets all additional member variables. */
    static CDCTrackHit castTrackHit(HitCDC aHit);

    /*! Assigns to each Hit a correct SuperlayerId (based on the LayerId) and the sets the boolean m_isAxial (true for axial layers, false for stereo layers)
     */
    void setSuperlayerId();

    /*!Assigns the coordinates of the hit wire (from CDC Geometry database) and sets the wire vector.*/
    void setWirePosition();

    /*!Assigns a boolean to mark the Hit as one already used for reconstruction.  */
    void setIsUsed(bool isUsed);

    /*!Assigns values for conformal coordinates. */
    void setConformalPosition(double conformalX, double conformalY);

    /*!Returns the SuperlayerId. */
    int getSuperlayerId() const {return m_superlayerId;};

    /*!Returns true for a Hit in an axial layer, false for a Hit in a stereo layer. */
    bool getIsAxial() const {return m_isAxial;};

    /*!Returns true if the Hit was already used for track reconstruction. */
    bool getIsUsed() const {return m_isUsed;};

    /*!Returns the position of the center(!!!) of the wire. */
    double getWirePosX() const {return m_posWireX;};
    double getWirePosY() const {return m_posWireY;};
    double getWirePosZ() const {return m_posWireZ;};

    /*!Returns the position of the forward end of the wire. */
    double getWirePosX_f() const {return m_posWireX_f;};
    double getWirePosY_f() const {return m_posWireY_f;};
    double getWirePosZ_f() const {return m_posWireZ_f;};

    /*!Returns the position of the backward end of the wire. */
    double getWirePosX_b() const {return m_posWireX_b;};
    double getWirePosY_b() const {return m_posWireY_b;};
    double getWirePosZ_b() const {return m_posWireZ_b;};

    /*!Returns the Hit position in the conformal plane.*/
    double getConformalX() const {return m_conformalX;};
    double getConformalY() const {return m_conformalY;};

    TVector3 getWireVector() const {return m_wireVector;};

    /*!Returns the phi angle of the center wire position.*/
    /*!From the Hit position (= center of the wire) the angle is calculated so that it goes from 0 to 2*pi.
     */
    double getPhi() const;

    /*!Searches for the best position (z coordinate) of a stereo hit and overwites the position coordinates with new values. */
    /*! This method searches for a given track (direction and one 'starting' hit) and this (stereo) hit for the z-coordinate of this hit, which fits best to the given track. For this the wire length is parametrised and for each point the 'fitting' of this new position is checked by calculating the shortest (perpendicular) distance between this hit and the track. After the optimal wire position is found, the coordinates (member variables) are overwritten according to it (!). The return value is the index from the wire parametrisation, by using the same paramatrisation for another (stereo) segments and hits can be shiftet to new positions without further calculations.*/
    void shiftAlongZ(TVector3 trackDirection, CDCTrackHit trackHit);

    /*!Set function to overwrite the hit position calculated by setWirePosition.*/
    /*!This function should be only used for stereo Hits in CDCSegment class, when the coordinates of the whole segment are shifted to other values (along the z wire). */
    void setStereoPosition(TVector3 position);


  private:

    int m_superlayerId; /**< ID of the superlayer of the Hit (superlayer = group of 6/8 layers with the same orientation)*/
    bool m_isAxial; /**< Boolean to mark a Hit as belonging to an axial superlayer (axial superlayers are 1,3,5,7,9)*/
    bool m_isUsed; /**<Boolean to mark a Hit as already used for reconstruction of a track candidate*/

    double m_posWireX; /**< Central X coordinate of the wire*/
    double m_posWireY; /**< Central Y coordinate of the wire*/
    double m_posWireZ; /**< Central Z coordinate of the wire*/

    double m_posWireX_f; /**<X coordinate of the forward end of the wire*/
    double m_posWireY_f; /**<Y coordinate of the forward end of the wire*/
    double m_posWireZ_f; /**<Z coordinate of the forward end of the wire*/

    double m_posWireX_b; /**<X coordinate of the backward end of the wire*/
    double m_posWireY_b; /**<Y coordinate of the backward end of the wire*/
    double m_posWireZ_b; /**<Z coordinate of the backward end of the wire*/

    TVector3 m_wireVector; /**< A vector along the wire*/

    double m_conformalX; /**<X coordinate of the hit in the conformal plane*/
    double m_conformalY; /**<Y coordinate of the hit in the conformal plane*/

    //! ROOT ClassDef macro to make this class a ROOT class.
    ClassDef(CDCTrackHit, 1);

  }; //end class CDCTrackHit
} //end namespace Belle2

#endif //CDCTRACKHIT


