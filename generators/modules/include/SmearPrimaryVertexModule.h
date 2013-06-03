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

  /** The SmearPrimaryVertex module.
   * Prints the content of the MCParticle collection
   * as tree using the B2INFO message to the logging system.
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

    virtual void initialize();
    virtual void terminate();

    /** Method is called for each run. */
    virtual void beginRun();

    /** Method is called for each event. */
    virtual void event();

  private:

    // Module parameters for the user interface

    // new nominal interaction point (cm)
    double m_new_ip_x, m_new_ip_y, m_new_ip_z;

    // new spread (standard deviation) for interaction point (cm)
    double m_sigma_ip_x, m_sigma_ip_y, m_sigma_ip_z;

    // new angle of beam profile (rad)
    double m_new_angle_ip_xy;
    double m_new_angle_ip_yz;
    double m_new_angle_ip_zx;

    // new nominal ip point and sigma (cm)
    TVector3 m_new_nominal_ip;
    TVector3 m_sigma_ip;
    TVector3 m_new_angle_ip;

    // new and old ip (smeared) (cm)
    TVector3 old_ip, m_new_ip;

    // time difference
    double difft;

    // returns new vertex positions
    TVector3 getShiftedVertex(TVector3 oldVertex);

    // sets new PV
    void setNewPrimaryVertex(void);
    void setOldPrimaryVertex(void);

    std::string m_particleList; /**< The name of the MCParticle collection. */

    bool m_useDB; /**< Use values from the Database. */


    // TODO: delete!
    TFile* m_rootFile;
    TTree* m_tree;
    double m_o_vx, m_o_vy, m_o_vz;
    double m_n_vx, m_n_vy, m_n_vz;
  };

} // end namespace Belle2

#endif // SMEARPRIMARYVERTEXMODULE_H
