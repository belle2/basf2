/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {

  /**
   *   Crystal shapes and positions
   */

  class ECLCrystalsShapeAndPosition: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLCrystalsShapeAndPosition() {}

    /**
     * Destructor
     */
    ~ECLCrystalsShapeAndPosition() {}

    /** Return crystal shape in forward endcap
     * @return crystal shape in forward endcap
     */
    const std::string& getShapeForward() const {return m_shape_forward; }

    /** Set crystal shape in forward endcap
     * @param shape_forward crystal shape in forward endcap
     */
    void setShapeForward(const std::string& shape_forward) { m_shape_forward = shape_forward; }

    /** Return crystal shape in barrel
     * @return crystal shape in barrel
     */
    const std::string& getShapeBarrel() const {return m_shape_barrel; }

    /** Set crystal shape in barrel
     * @param shape_barrel crystal shape in barrel
     */
    void setShapeBarrel(const std::string& shape_barrel) { m_shape_barrel = shape_barrel; }

    /** Return crystal shape in backward endcap
     * @return crystal shape in backward endcap
     */
    const std::string& getShapeBackward() const {return m_shape_backward; }

    /** Set crystal shape in backward endcap
     * @param shape_backward crystal shape in backward endcap
     */
    void setShapeBackward(const std::string& shape_backward) { m_shape_backward = shape_backward; }

    /** Return crystal placement in forward endcap
     * @return crystal placement in forward endcap
     */
    const std::string& getPlacementForward() const {return m_placement_forward; }

    /** Set crystal placement in forward endcap
     * @param placement_forward crystal placement in forward endcap
     */
    void setPlacementForward(const std::string& placement_forward) { m_placement_forward = placement_forward; }

    /** Return crystal placement in barrel
     * @return crystal placement in barrel
     */
    const std::string& getPlacementBarrel() const {return m_placement_barrel; }

    /** Set crystal placement in barrel
     * @param placement_barrel crystal placement in barrel
     */
    void setPlacementBarrel(const std::string& placement_barrel) { m_placement_barrel = placement_barrel; }

    /** Return crystal placement in backward endcap
     * @return crystal placement in backward endcap
     */
    const std::string& getPlacementBackward() const {return m_placement_backward; }

    /** Set crystal placement in backward endcap
     * @param placement_backward crystal placement in backward endcap
     */
    void setPlacementBackward(const std::string& placement_backward) { m_placement_backward = placement_backward; }

  private:
    std::string m_shape_forward; /**< Crystal shapes in forward endcap */
    std::string m_shape_barrel; /**< Crystal shapes in barrel */
    std::string m_shape_backward; /**< Crystal shapes in backward endcap */
    std::string m_placement_forward; /**< Crystal placements in forward endcap */
    std::string m_placement_barrel; /**< Crystal placements in barrel */
    std::string m_placement_backward; /**< Crystal placements in backward endcap */

    ClassDef(ECLCrystalsShapeAndPosition, 1); /**< ClassDef */
  };
} // end namespace Belle2
