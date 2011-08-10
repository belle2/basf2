/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCSIGNAL_H
#define CDCSIGNAL_H

namespace Belle2 {

//! The Class for Holding CDC Signals
  /*! This class is used to hold CDC signals(digits).
  */

  class CDCSignal {

  public:

    //! Constructor
    CDCSignal() {;}

    //! Overloaeded constructor
    /*!
        \param layerId Layer id.
        \param wireId  Wire id.
        \param charge The charge of this signal.
        \param driftLength Drift length of this signal.
    */
    CDCSignal(int hitNumber, int layerId, int wireId, double charge, double driftLength, double driftTime) : m_hitNumber(hitNumber), m_layerId(layerId), m_wireId(wireId), m_charge(charge), m_driftLength(driftLength), m_driftTime(driftTime) {;}

    //CDCSignal(int layerId, int wireId, double charge, double driftLength, EVENT::MCParticle * mcPart) : m_layerId(layerId), m_wireId(wireId), m_charge(charge),
    //       m_driftLength(driftLength) { m_MCPartVec.push_back(mcPart); m_MCWeightVec.push_back(charge);}

    //CDCSignal(int layerId, int wireId, double charge, double driftLength, MCPartVec mcPartVec, MCWeightVec mcWeightVec) :
    //       m_layerId(layerId), m_wireId(wireId), m_charge(charge), m_driftLength(driftLength) { m_MCPartVec = mcPartVec; m_MCWeightVec = mcWeightVec;}

    //! Destructor
    ~CDCSignal() {};

    //! Set hit number
    /*!
        \param hitNumber
    */
    inline void setHitNumber(int hitNumber) { m_hitNumber = hitNumber; }

    //! Set layer id
    /*!
        \param layerId Layer id.
    */
    inline void setLayerId(int layerId) { m_layerId = layerId; }

    //! Set wire id
    /*!
        \param wireId Wire id.
    */
    inline void setWireId(int wireId) { m_wireId = wireId; }

    //! Set signal
    /*!
        \param charge The charge of this signal.
    */
    inline void setCharge(double charge) { m_charge = charge; }

    //! Update signal
    /*!
        \param charge The value added to current charge.
    */
    inline void updateCharge(double charge) { m_charge += charge; }

    //! Set drift length when signal was created
    /*!
        \param driftLength Drift length when signal was created.
    */
    inline void setDriftLength(double driftLength) { m_driftLength = driftLength; }

    //! Set drift time when signal was created
    /*!
        \param driftTime Drift time when signal was created.
    */
    inline void setDriftTime(double driftTime) { m_driftTime = driftTime; }

    //! Update MC truth information
    //void updateMCParticles(EVENT::MCParticle * mcPart, float weight);

    //! Update MC truth information
    //void updateMCParticles(MCPartVec mcPartVec, MCWeightVec mcWeightVec);


    //! Get Hit number
    /*!
        \return hit number.
    */
    inline int getHitNumber() const { return m_hitNumber; }

    //! Get layer id
    /*!
        \return Layer id.
    */
    inline int getLayerId() const { return m_layerId; }

    //! Get wire id
    /*!
        \return Wire id.
    */
    inline int getWireId() const { return m_wireId; }

    //! Get signal
    /*!
        \return Charge of current signal.
    */
    inline double getCharge() const {return m_charge;}

    //! Get drift length when signal was created
    /*!
        \return Drift length of current signal.
    */
    inline double getDriftLength() const {return m_driftLength;}

    //! Get drift time when signal was created
    /*!
        \return Drift time of current signal.
    */
    inline double getDriftTime() const {return m_driftTime;}

    //! Get MC truth information - particle pointers
    //inline MCPartVec getMCPartVec() const {return m_MCPartVec;}

    //! Get MC truth information - particle statistics (weights)
    //inline MCWeightVec getMCWeightVec() const {return m_MCWeightVec;}

    //! Get MC truth information - total sum of individual weights
    //float getMCWeightSum();

  private:

    int m_hitNumber;   /*!< The number of hit in CDCSimHitArray */
    int m_layerId;   /*!< Layer id */
    int m_wireId;    /*!< Wire id */
    double m_charge; /*!< Charge of CDC signal */
    double m_driftLength;    /*!< Drift length when signal has been created */
    double m_driftTime;    /*!< Drift time when signal has been created */

  };

} // end of Belle2 namespace

#endif // CDCSIGNAL_H
