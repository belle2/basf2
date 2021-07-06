/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Root
#include <TObject.h>
#include <TGraph2D.h>

namespace Belle2 {

  /**
   * Class to hold the information for the ECL shower energy corrections
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
     * Angles are in degrees!!
     */
    ECLShowerEnergyCorrectionTemporary(const TGraph2D& graph,
                                       const float angleMin,
                                       const float angleMax,
                                       const float energyMin,
                                       const float energyMax) :
      m_graph(graph),
      m_angleMin(angleMin),
      m_angleMax(angleMax),
      m_energyMin(energyMin),
      m_energyMax(energyMax)
    {

    }

    /**
     * Destructor
     */
    ~ECLShowerEnergyCorrectionTemporary() {};

    /*! Get the TGraph2D graph
     * x - angle (degrees)
     * y - energy
     * z - correction factor
     * Angles are in degrees!!
     * @return graph holding correction information
     */
    TGraph2D getGraph2D() const {return m_graph;}

    /*! Get Minimum value of angle which the corrections are valid for.
     * Angles are in degrees!!
     * @return Minimum angle
     */
    float getAngleMin() const {return m_angleMin;}

    /*! Get Maximum value of angle which the corrections are valid for.
     * Angles are in degrees!!
     * @return Maximum angle
     */
    float getAngleMax() const {return m_angleMax;}

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
    float m_angleMin; /**< Minimum value of angle which the corrections are valid for (in degrees). */
    float m_angleMax; /**< Maximum value of angle which the corrections are valid for (in degrees). */
    float m_energyMin; /**< Minimum value of energy which the corrections are valid for. */
    float m_energyMax; /**< Maximum value of energy which the corrections are valid for. */

    // 1: Initial version
    ClassDef(ECLShowerEnergyCorrectionTemporary, 1); /**< ClassDef */
  };
} // end namespace Belle2

