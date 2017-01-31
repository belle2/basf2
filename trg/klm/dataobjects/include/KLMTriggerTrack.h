/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dmitri Liventsev                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMTRIGGERTRACK_H
#define KLMTRIGGERTRACK_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  //! Store KLM TRG track information as a ROOT object
  class KLMTriggerTrack : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMTriggerTrack():
      m_fwd(0),
      m_sector(0),
      m_nHits(0),
      m_slopeXY(0.0),
      m_interceptXY(0.0),
      m_impactParameterXY(0.0),
      m_chisqXY(0.0),
      m_slopeXZ(0.0),
      m_interceptXZ(0.0),
      m_impactParameterXZ(0.0),
      m_chisqXZ(0.0),
      m_nFiredLayers(0),
      m_trigger(false)
    { }

    //! Constructor with initial values for a track
    //! @param fwd detector end (forward=true and backward=false)
    //! @param sector sector number
    KLMTriggerTrack(bool fwd, int sector):
      m_fwd(fwd),
      m_sector(sector),
      m_nHits(0),
      m_slopeXY(0.0),
      m_interceptXY(0.0),
      m_impactParameterXY(0.0),
      m_chisqXY(0.0),
      m_slopeXZ(0.0),
      m_interceptXZ(0.0),
      m_impactParameterXZ(0.0),
      m_chisqXZ(0.0),
      m_nFiredLayers(0),
      m_trigger(false)
    { }

    //! Destructor
    virtual ~KLMTriggerTrack() { }

    // accessors

    //! Get detector end
    //! @return detector end (forward=true and backward=false)
    bool isForward() const { return m_fwd; }

    //! Get sector number
    //! @return sector number of the track (0..7)
    int getSector() const { return m_sector; }

    //! Get slope parameter of the track in XY plain
    //! @return slope parameter of the track in XY plain
    double getSlopeXY() const { return m_slopeXY; }

    //! Get intercept parameter of the track in XY plain
    //! @return intercept parameter of the track in XY plain
    double getInterceptXY() const { return m_interceptXY; }

    //! Get impact parameter of the track in XY plain
    //! @return impact parameter of the track in XY plain
    double getImpactParameterXY() const { return m_impactParameterXY; }

    //! Get chi squared of the track in XY plain
    //! @return chi squared of the track in XY plain
    double getChisqXY() const { return m_chisqXY; }

    //! Get slope parameter of the track in XZ plain
    //! @return slope parameter of the track in XZ plain
    double getSlopeXZ() const { return m_slopeXZ; }

    //! Get intercept parameter of the track in XZ plain
    //! @return intercept parameter of the track in XZ plain
    double getInterceptXZ() const { return m_interceptXZ; }

    //! Get impact parameter of the track in XZ plain
    //! @return impact parameter of the track in XZ plain
    double getImpactParameterXZ() const { return m_impactParameterXZ; }

    //! Get chi squared of the track in XZ plain
    //! @return chi squared of the track in XZ plain
    double getChisqXZ() const { return m_chisqXZ; }

    //! Get number of fired layers
    //! @return number of fired layers
    int getNFiredLayers() const { return m_nFiredLayers; }

    //! Get trigger flag
    //! @return if the track generates a trigger
    bool getTrigger() const { return m_trigger; }

    // modifiers

    //! Set slope parameter of the track in XY plain
    //! @param slopeXY slope parameter of the track in XY plain
    void setSlopeXY(double slopeXY)  { m_slopeXY = slopeXY; }

    //! Set intercept parameter of the track in XY plain
    //! @param interceptXY intercept parameter of the track in XY plain
    void setInterceptXY(double interceptXY)  { m_interceptXY = interceptXY; }

    //! Set impact parameter of the track in XY plain
    //! @param ipXY impact parameter of the track in XY plain
    void setImpactParameterXY(double ipXY)  { m_impactParameterXY = ipXY; }

    //! Set chi squared of the track in XY plain
    //! @param chisqXY chi squared of the track in XY plain
    void setChisqXY(double chisqXY)  { m_chisqXY = chisqXY; }

    //! Set slope parameter of the track in XZ plain
    //! @param slopeXZ slope parameter of the track in XZ plain
    void setSlopeXZ(double slopeXZ)  { m_slopeXZ = slopeXZ; }

    //! Set intercept parameter of the track in XZ plain
    //! @param interceptXZ intercept parameter of the track in XZ plain
    void setInterceptXZ(double interceptXZ)  { m_interceptXZ = interceptXZ; }

    //! Set impact parameter of the track in XZ plain
    //! @param ipXZ impact parameter of the track in XZ plain
    void setImpactParameterXZ(double ipXZ)  { m_impactParameterXZ = ipXZ; }

    //! Set chi squared of the track in XZ plain
    //! @param chisqXZ chi squared of the track in XZ plain
    void setChisqXZ(double chisqXZ)  { m_chisqXZ = chisqXZ; }

    //! Set number of fired layers
    //! @param nLayers number of fired layers
    void setNFiredLayers(int nLayers)  { m_nFiredLayers = nLayers; }

    //! Set trigger flag
    //! @param trg if the track generates a trigger
    void setTrigger(bool trg)  { m_trigger = trg; }

  private:
    //! forward-backward
    int m_fwd;

    //! sector number
    int m_sector;

    //! number of hits in the sector
    int m_nHits;

    //! calculated slope of the straight track in XY plain
    double m_slopeXY;

    //! calculated y-intercept of the straight track in XY plain
    double m_interceptXY;

    //! calculated impact parameter of the straight track in XY plain
    double m_impactParameterXY;

    //! calculated chi squared of the straight track in XY plain
    double m_chisqXY;

    //! calculated slope of the straight track in XZ plain
    double m_slopeXZ;

    //! calculated z-intercept of the straight track in XZ plain
    double m_interceptXZ;

    //! calculated impact parameter of the straight track in XZ plain
    double m_impactParameterXZ;

    //! calculated chi squared of the straight track in XZ plain
    double m_chisqXZ;

    //! number of fired layers
    int m_nFiredLayers;

    //! if the track generates a trigger
    bool m_trigger;

    //! Needed to make the ROOT object storable
    //! version 4 adds ctime etc
    ClassDef(KLMTriggerTrack, 1);
  };
} // end of namespace Belle2

#endif //KLMTRIGGERTRACK_H
