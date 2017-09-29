/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Leakage corrections for ECL showers (N1).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//Root
#include <TObject.h>
#include <TGraph2D.h>

namespace Belle2 {

  /**
   * Class to hold the information for the ECL shower energy corrections
   * Corrections produced by Claudia Cecchi (claudia.cecchi@pg.infn.itElisa) Manoni (elisa.manoni@pg.infn.it)
   * This is a temperary class as there is no information about phi dependence.
   * An improved version will be implemented at a later time.
   *
   * NOTE - angles are in degrees
   */

  class ECLShowerEnergyCorrectionTemporary: public TObject {
  public:

    /**
     * Default constructor
     *
     */
    ECLShowerEnergyCorrectionTemporary() {}

    /**
     * Constructor
     * Theta angles are in degrees!!
     */
    ECLShowerEnergyCorrectionTemporary(const TGraph2D& graph,
                                       const float thetaMin,
                                       const float thetaMax,
                                       const float energyMin,
                                       const float energyMax) :
      m_graph(graph),
      m_thetaMin(thetaMin),
      m_thetaMax(thetaMax),
      m_energyMin(energyMin),
      m_energyMax(energyMax)
    {

    }

    /**
     * Destructor
     */
    ~ECLShowerEnergyCorrectionTemporary() {};

    /*! Get the TGraph2D graph
     * x - theta (degrees)
     * y - energy
     * z - correction factor
     * Theta angles are in degrees!!
     * @return graph holding correction information
     */
    TGraph2D getGraph2D() const {return m_graph;}

    /*! Get Minimum value of theta which the corrections are valid for.
     * Angles are in degrees!!
     * @return Minimum theta
     */
    float getThetaMin() const {return m_thetaMin;}

    /*! Get Maximum value of theta which the corrections are valid for.
     * Angles are in degrees!!
     * @return Maximum theta
     */
    float getThetaMax() const {return m_thetaMax;}

    /*! Get Minimum value of energy which the corrections are valid for.
     * @return Minimum energy
     */
    float getEnergyMin() const {return m_energyMin;}

    /*! Get Maximum value of energy which the corrections are valid for.
     * @return Maximum energy
     */
    float getEnergyMax() const {return m_energyMax;}

  private:
    TGraph2D m_graph; /**< graph holding the shower energy corrections. */
    float m_thetaMin; /**< Minimum value of theta which the corrections are valid for (in degrees). */
    float m_thetaMax; /**< Maximum value of theta which the corrections are valid for (in degrees). */
    float m_energyMin; /**< Minimum value of energy which the corrections are valid for. */
    float m_energyMax; /**< Maximum value of energy which the corrections are valid for. */

    // 1: Initial version
    ClassDef(ECLShowerEnergyCorrectionTemporary, 1); /**< ClassDef */
  };
} // end namespace Belle2

