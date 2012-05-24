/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRACKHIT_H
#define CDCTRACKHIT_H

#include "TObject.h"
#include "TVector3.h"
#include <cdc/dataobjects/CDCHit.h>


namespace Belle2 {

  /** CDC Hit Class used for pattern recognition. */
  class CDCTrackHit : public TObject {
  public:

    /** Empty constructor. */
    CDCTrackHit();

    /** Constructor to create a CDCTrackHit from a CDCHit object.
     * Some member variables of CDCHit are copied and other to CDCTrackHit specific variables are initialized (e.g. the position of the hit wire in normal space and in the conformal plane).
     * The index should be the index of the CDCTrackHit/CDCHit in the StoreArray (CDCTrackHit objects should be created in the same order).
     * For the moment the definition of layerID differs between CDCHit and CDCTrackHit, this will be made consistent later on.
     */
    CDCTrackHit(CDCHit* hit, int index);

    /* Destructor. */
    ~CDCTrackHit();

    /** Assigns the coordinates of the hit wire (from CDC Geometry database) and sets the wire vector.*/
    void setWirePosition();

    /** Assigns values for conformal coordinates by transforming the wire coordinates. */
    void ConformalTransformation();

    /** Adds an index of the TrackCandidate to which this Hit may belong.*/
    void addTrackIndex(int index);

    /** Returns the layerId (0 - 55). */
    int getLayerId() const {return m_layerId;};

    /** Returns the wireId (0 - 383, depends on layer). */
    int getWireId() const {return m_wireId;};

    /** Returns the superlayerId (0-8). */
    int getSuperlayerId() const {return m_superlayerId;};

    /** Returns the DriftTime (which maybe saved as DriftLenght for the moment, see documentation of CDCHit).*/
    double getDriftTime() const {return m_driftTime;}

    /** Returns the index of the original CDCHit in the StoreArray. */
    int getStoreIndex() const { return m_storeIndex; };

    /** Returns true for a Hit in an axial layer, false for a Hit in a stereo layer. */
    bool getIsAxial() const {return m_isAxial;};

    /** Returns the position of the center(!!!) of the wire. */
    TVector3 getWirePosition() const {return m_wirePosition ;}

    /** Returns the Hit position (X coordinate) in the conformal plane.*/
    double getConformalX() const {return m_conformalX;};

    /** Returns the Hit position (Y coordinate) in the conformal plane.*/
    double getConformalY() const {return m_conformalY;};

    /** Returns the phi angle of the center wire position.
      * From the Hit position (= center of the wire) the angle is calculated so that it goes from 0 to 2*pi.
      */
    double getPhi() const;


    /** Returns a vector with indices of Tracks, to which this Hit may belong. */
    std::vector<int> getTrackIndices() const { return m_TrackIndices; }

    /** Searches for the best position (z coordinate) of a stereo hit and overwrites the position coordinates with new values.
     *  This method searches for a given track (direction and one 'starting' hit) and this (stereo) hit for the z-coordinate of this hit, which fits best to the given track.
     *  For this the wire length is parametrised and for each point the 'fitting' of this new position is checked by calculating the shortest (perpendicular) distance between this hit and the track.
     *  After the optimal wire position is found, the coordinates (member variables) are overwritten according to it (!).
     */
    void shiftAlongZ(const TVector3 trackDirection, const CDCTrackHit trackHit);

    /** Sets a temporary cell state for the cellular automaton. */
    void setTempCellState(const int tempCellState);

    /** Sets the 'permanent' cell state to the current temporary cell state. */
    void updateCellState();

    /** Returns the 'permanent' cell state.*/
    int getCellState() const {return m_cellState;};

    /** Adds an Id of a segment to which this hit may belong. */
    void setSegmentId(const int segmentId);

    /** Adds several Ids of segments to which this hit may belong. */
    void setSegmentIds(const std::vector<int> segmentIds);

    /** Clears/Erases all stored segment Ids. */
    void clearSegmentIds();

    /** Returns the vectors with Ids of segments to which this hit may belong. */
    std::vector<int>getSegmentIds() const {return m_segmentIds;};




  private:

    int m_layerId;                     /**< ID of the layer of the hit (0-55)*/
    int m_wireId;                      /**< ID of the wire of the hit */
    int m_superlayerId;                /**< ID of the superlayer of the hit (0-8) (superlayer = group of 6/8 layers with the same orientation)*/
    double m_driftTime;                /**< Drifttime of the CDCHit (may be DriftLegth, see CDCHit documentation)*/
    double m_charge;                   /**< Charge deposit the CDCHit */

    bool m_isAxial;                    /**< Boolean to mark a Hit as belonging to an axial superlayer */
    int m_storeIndex;                  /** Index within the StoreArray of the Hit */

    TVector3 m_wirePosition;           /**< Coordinates of the center (!) of the hit wire. */

    double m_conformalX;               /**< X coordinate of the hit in the conformal plane*/
    double m_conformalY;               /**< Y coordinate of the hit in the conformal plane*/

    std::vector<int> m_segmentIds;     /**< Vector with segment indices, to which this hit may belong */
    std::vector<int> m_TrackIndices;   /**< Vector with track indices, to which this hit may belong */

    int m_tempCellState;               /**< Temporary cell state for the use of cellular automaton*/
    int m_cellState;                   /**< Final cell state for the use of cellular automaton*/


    /** ROOT ClassDef macro to make this class a ROOT class. */
    ClassDef(CDCTrackHit, 1);

  }; //end class CDCTrackHit
} //end namespace Belle2

#endif //CDCTRACKHIT


