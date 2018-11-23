/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SMEARPRIMARYVERTEXMODULE_H
#define SMEARPRIMARYVERTEXMODULE_H

#include <framework/core/Module.h>

#include <TVector3.h>

#include <string>
#include <vector>

//TODO: delete
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  class MCParticle;

  /**
   * The SmearPrimaryVertex module moves the Primary Vertex (e+e- collision point) to
   * to a new point defined by user. In addition smearing is applied randomly on event
   * by event basis. Module loops over all MCParticles and shiftes their decay and
   * production vertices.
   */
  class SmearPrimaryVertexModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    SmearPrimaryVertexModule();

    /** Destructor. */
    virtual ~SmearPrimaryVertexModule() {}

    virtual void initialize() override;
    virtual void terminate() override;

    /** Method is called for each run. */
    virtual void beginRun() override;

    /** Method is called for each event. */
    virtual void event() override;

  private:

    // Module parameters for the user interface

    // new nominal interaction point (cm)
    double m_new_ip_x; /**< New nominal position of Primary Vertex in x (cm) */
    double m_new_ip_y; /**< New nominal position of Primary Vertex in y (cm) */
    double m_new_ip_z; /**< New nominal position of Primary Vertex in z (cm) */

    // new spread (standard deviation) for interaction point (cm)
    double m_sigma_ip_x; /**< Spread (standard deviation) of Primary Vertex in x (cm) */
    double m_sigma_ip_y; /**< Spread (standard deviation) of Primary Vertex in y (cm) */
    double m_sigma_ip_z; /**< Spread (standard deviation) of Primary Vertex in z (cm) */

    // new angle of beam profile (rad)
    double m_new_angle_ip_xy; /**< Angle of rotation of Primary Vertex Profile wrt. z-axis (in xy-plane) in (rad) */
    double m_new_angle_ip_yz; /**< Angle of rotation of Primary Vertex Profile wrt. x-axis (in yz-plane) in (rad) */
    double m_new_angle_ip_zx; /**< Angle of rotation of Primary Vertex Profile wrt. y-axis (in zx-plane) in (rad) */

    // new nominal ip point and sigma (cm)
    TVector3 m_new_nominal_ip; /**< New nominal position of Primary Vertex in (cm) */
    TVector3 m_sigma_ip;       /**< Spread (standard deviation) of Primary Vertex in (cm) */
    TVector3 m_new_angle_ip;   /**< Angle of rotation of Primary Vertex Profile */

    // new and old ip (smeared) (cm)
    TVector3 old_ip;    /**< Old Primary Vertex position (before smearing) */
    TVector3 m_new_ip;  /**< New Primery Vertex position (after smearing) */

    /**
     * Returns the shifted vertex given as an input.
     * @param oldVertex to be shifted
     * @return shifted vertex (given by  oldVertex + (newIP - oldIP))
     */
    TVector3 getShiftedVertex(TVector3 oldVertex);


    /**
     * Determines the new Primary Vertex for this event from the primary vertex specified by the user and
     * random shift given by the user specified spread.
     */
    void setNewPrimaryVertex(void);

    /**
     * Old primary vertex is taken to be the production vertex of the first MCParticle in the MCParticles array.
     */
    void setOldPrimaryVertex(void);

    std::string m_particleList; /**< The name of the MCParticle collection. */

    bool m_useDB; /**< Use values from the Database. */
  };

} // end namespace Belle2

#endif // SMEARPRIMARYVERTEXMODULE_H
