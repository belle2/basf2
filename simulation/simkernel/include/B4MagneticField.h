/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4MAGNETICFIELD_H
#define B4MAGNETICFIELD_H

#include <simkernel/B4MagneticFieldMessenger.h>

#include "globals.hh"
#include "G4MagneticField.hh"
#include "G4ThreeVector.hh"
#include "G4FieldManager.hh"
#include "G4ChordFinder.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4MagIntegratorStepper.hh"

namespace Belle2 {

//! The Class for Belle2 Magnetic Field
  /*! This class inherits from G4MagneticField in GEANT4.
      It is used to provide magnetic field during the simulation.

      The magnetic field value at each space point should be calculated based on field map gotten from gearbox,
      but at current stage, we just use a very simple magnetic field distribution.
      It is a uniform field, bx = 0, by = 0, bz = 1.5Tesla.

      In this class, users can also define the accuracy of propagating tracks in magnetic field and
      select different Runge-Kutta method and stepper.
  */

  class B4MagneticField : public G4MagneticField {

  public:

    //! Constructor
    B4MagneticField();

    //! Destructor
    ~B4MagneticField();

    //! The method to get magnetic field at a point and will be called automatically during the simulation.
    /*!
        \param Point A space point in detector at which magnetic field value will be provided.
        \param Bfield The value of magnetic field which should be returned.
    */
    void  GetFieldValue(const G4double Point[3], G4double *Bfield) const;

    //! Initialize method
    void initialiseAll();

    //! The method to create stepper and chord finder.
    /*! Several important limits are listed below.
        1. Minimal step = 0.01mm, is used during integration to limit the amount of work in difficult cases,
           It is possible that strong fields or integration problems can force the integrator to try very small steps;
           this parameter stops them from becoming unnecessarily small.
        2. Maximum miss distance (delta chord) = 0.25*mm, is an upper bound for the distance between
           the 'real' curved trajectory and the approximate linear trajectory of the chord.
        3. Delta intersection = 1.0e-3*mm, is the accuracy to which an intersection with a volume boundary is calculated.
           If a candidate boundary intersection is estimated to have a precision better than this, it is accepted.
        4. Delta one step =1.0e-2*mm, is the accuracy for the endpoint of 'ordinary' integration steps.
           This parameter is a limit on the estimated error of the endpoint of each physics step.

        Parameter 3 and 4 are correlated with potential systematic errors in the momentum of reconstructed tracks.
    */
    void createStepperAndChordFinder();

    //! The method to set stepper
    /*! Use the classical fourth-order Runge-Kutta stepper as default.
    */
    void setStepper();

    //! The method to set stepper type
    /*!
        \param stepperType The new stepper type.
    */
    void setStepperType(G4int stepperType) { m_stepperType = stepperType; }

    //! The method to set minimal step
    /*!
        \minStep The new minimal step value.
    */
    void setMinStep(G4double minStep) { m_minStep = minStep; }

    //! The method to set delta one step
    /*!
        \param deltaOneStep The new value of delta one step.
    */
    void setDeltaOneStep(double deltaOneStep) { m_deltaOneStep = deltaOneStep; }

    //! The method to set delta intersection
    /*!
        \param deltaIntersection The new value of delta intersection.
    */
    void setDeltaIntersection(double deltaIntersection) { m_deltaIntersection = deltaIntersection; }

    //! The method to set minimum epsilon step
    /*!
        \param minEpsilonStep The new value of minimum epsilon step.
    */
    void setMinEpsilonStep(double minEpsilonStep) { m_minEpsilonStep = minEpsilonStep; }

    //! The method to set maximum epsilon step
    /*!
        \param maxEpsilonStep The new value of maximum epsilon step.
    */
    void setMaxEpsilonStep(double maxEpsilonStep) { m_maxEpsilonStep = maxEpsilonStep; }

  private:

    B4MagneticFieldMessenger* m_fieldMessenger;    /*!< Pointer of B4MagneticFieldMessenger */
    G4FieldManager*           m_fieldManager;      /*!< Pointer that points class G4FieldManager. */
    G4ChordFinder*            m_chordFinder;       /*!< Pointer that points class G4ChordFinder. */
    G4Mag_UsualEqRhs*         m_equation;          /*!< Pointer that points class G4Mag_UsualEqRhs. */
    G4MagIntegratorStepper*   m_stepper;           /*!< Pointer that points class G4MagIntegratorStepper. */
    G4int                     m_stepperType;       /*!< Stepper type. */
    G4double                  m_minStep;           /*!< Minimal step. */
    G4double                  m_deltaOneStep;      /*!< Delta one step. */
    G4double                  m_deltaIntersection; /*!< Delta intersection. */
    G4double                  m_minEpsilonStep;    /*!< Minimal epsilon step. */
    G4double                  m_maxEpsilonStep;    /*!< Maximum epsilon step. */
  };

} // end namespace Belle2

#endif /* B4MAGNETICFIELD_H */
