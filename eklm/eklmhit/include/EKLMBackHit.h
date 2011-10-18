/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributor: Timofey Uglov                                             *
* based on class by  Luka Santelj and Marko Petric                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#ifndef EKLMBACKHIT_H
#define EKLMBACKHIT_H

#include <TObject.h>
#include <TVector3.h>
#include <TString.h>

namespace Belle2 {


  //! Class EKLMBackHit - Stores hits from  backgound simulation
  /*!
   This is a class to store  backgound hits hits in datastore.
   It is a general class for all subdetectors
   */

  class EKLMBackHit : public TObject {
  public:

    //! Default constructor
    EKLMBackHit():
        m_subDet(0),
        m_identifier(0),
        m_PDG(0),
        m_t(0),
        m_E(0),
        m_position(0., 0., 0.),
        m_momentum(0., 0., 0.),
        m_energyDeposit(0.),
        m_trackID(-1),
        m_parentTrackID(-1),
        m_isFirstStep(false),
        m_pvName("not initialized") {
    }

    //! Full constructor.
    /*!
     \param m_PDG the PDG code of the particle
     \param m_t the global time the at which the hit occured
     \param m_E energy of the particle
     \param m_position the global position at which the track occured
     \param m_momentum the of the particle that produced the hit
     */

    EKLMBackHit(
      int subDet,
      int iden,
      int PDG,
      double t,
      double E,
      TVector3 position,
      TVector3 momentum,
      double edep ,
      int    trID,
      int  ptrID,
      bool firstStep,
      std::string name
    )  {
      m_subDet = subDet;
      m_identifier = iden;
      m_PDG = PDG;
      m_t = t;
      m_E = E;
      m_position = position;
      m_momentum = momentum;
      m_energyDeposit = edep;
      m_trackID = trID;
      m_parentTrackID = ptrID;
      m_isFirstStep = firstStep;
      m_pvName = name;
    }

    //! Get the subdetector name in which the hit occured
    TString getSubDetName() const {
      switch (m_subDet) {
        case 0: return "IR";
        case 1: return "PDX";
        case 2: return "SVD";
        case 3: return "CDC";
        case 4: return "ARICH";
        case 5: return "TOP";
        case 6: return "ECL";
        case 7: return "EKLM";
        case 8: return "BKLM";
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


    //! Get energy deposition
    const double &getEDep() const { return m_energyDeposit; }

    //! Get track ID
    const int &getTrackID() const { return m_trackID;}

    //! Get  ID of parent track
    const int &getParentTrackID() const { return m_parentTrackID;}

    //! check if the step is the first
    const bool &getFirstStep() const { return m_isFirstStep;}

    //! get volume name
    const std::string getName()  const { return m_pvName;}


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


    double m_energyDeposit;
    int m_trackID;
    int m_parentTrackID;
    bool m_isFirstStep;
    std::string m_pvName;



    ClassDef(EKLMBackHit, 1);   /**< the class title */

  };

} // end namespace Belle2

#endif
