/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric and Luka Santelj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMBACKHIT_H
#define BEAMBACKHIT_H

#include <TObject.h>
#include <TVector3.h>
#include <TString.h>

namespace Belle2 {


  //! Class BeamBackHit - Stores hits from beam backgound simulation
  /*!
   This is a class to store beam backgound hits hits in datastore.
   It is a general class for all subdetectors
   */

  class BeamBackHit : public TObject {
  public:

    //! Default constructor
    BeamBackHit():
        m_subDet(0),
        m_identifier(0),
        m_PDG(0),
        m_t(0),
        m_E(0),
        m_position(0., 0., 0.),
        m_momentum(0., 0., 0.) {
    }

    //! Full constructor.
    /*!
     \param m_PDG the PDG code of the particle
     \param m_t the global time the at which the hit occured
     \param m_E energy of the particle
     \param m_position the global position at which the track occured
     \param m_momentum the of the particle that produced the hit
     */

    BeamBackHit(
      int subDet,
      int iden,
      int PDG,
      double t,
      double E,
      TVector3 position,
      TVector3 momentum
    )  {
      m_subDet = subDet;
      m_identifier = iden;
      m_PDG = PDG;
      m_t = t;
      m_E = E;
      m_position = position;
      m_momentum = momentum;

    }

    //! Get the subdetector name in which the hit occured
    TString getSubDetName() const {
      switch (m_subDet) {
        case 0: return "IR";
        case 1: return "PDX";
        case 2: return "SVD";
        case 3: return "CDC";
        case 4: return "TOP";
        case 5: return "ARICH";
        case 6: return "ECL";
        case 7: return "BKLM";
        case 8: return "EKLM";
      }
      return "";
    }

    //! Get the identifier of subdetector component in which hit occured
    int getIdentifier() const {
      return m_identifier;
    }

    //! Det the index of subdetector in which hit occured
    int getSubDet() const {return m_subDet; }

    //! Get the lund code of the particle that hit the sensitive area
    int getPDG() const { return m_PDG; }

    //! Get the time at which the hit occured
    double getTime() const { return m_t; }

    //! Get energy of the particle
    double getEnergy() const { return m_E; }

    //! Get global position of the particle hit
    const TVector3 &getPosition() const { return m_position; }

    //! Get momentum of the particle hit
    const TVector3 &getMomentum() const { return m_momentum; }

    //! Set the subdetector group in which the hit occured
    void setSubDet(int subDet) { m_subDet = subDet; }

    //! Set the subdetector component identifier
    void setIdentifier(int iden) { m_identifier = iden; }

    //! Set the lund code of the particle that hit the sensitive area
    void setPDG(int PDG)  { m_PDG = PDG; }

    //! Set the time at which the hit occured
    void setTime(double t)  {  m_t = t; }

    //! Set energy of the particle
    void setEnergy(double E)  {  m_E = E; }

    //! Set global position of the particle hit
    void getPosition(TVector3 position)  {  m_position = position; }

    //! Set momentum of the particle hit
    void getMomentum(TVector3 momentum)  {  m_momentum = momentum; }


  private:
    int m_subDet;               /**< The name of the subdetector */
    int m_identifier;           /**< The identifier of subdetector component */
    int m_PDG;                  /**< The PDG code of the particle that hit the sensitive area */
    double m_t;                 /**< time at which the hit occured */
    double m_E;                 /**< energy of particle */
    TVector3 m_position;        /**< global position of the hit */
    TVector3 m_momentum;        /**< momentum of the hit */

    ClassDef(BeamBackHit, 1);   /**< the class title */

  };

} // end namespace Belle2

#endif
