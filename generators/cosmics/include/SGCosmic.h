/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Based on the Belle single track generator by KUNIYA Toshio             *
 * Contributors: Sergey Yashchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>

namespace Belle2 {

  /**
   * Class to generate tracks in the cosmics generator and store them in a MCParticle graph.
   * The class supports two version (levels) of event generation with IP and pt parameters.
   */
  class SGCosmic {

  public:

    /**
     * Struct to keep all necessary parameters for the cosmic generator
     */
    struct Parameters {
      /**
       * Generator version: level 1 (default) or 2
       */
      int level;
      /**
       * Restrict the vertex to IP or not (default)
       */
      int ipRequirement;
      /**
       * Vertex restriction in the radial direction
       */
      double ipdr;
      /**
       * Vertex restriction in z direction
       */
      double ipdz;
      /**
       * Minimum value of the transverse momentum
       */
      double ptmin;
      /**
       * Cylindrical radius of generation
       */
      double cylindricalR;
    };

    /**
     * Default constructor
     */
    SGCosmic();

    /**
     * Default destructor
     */
    ~SGCosmic() {}

    /**
     * Generates the next event and store the result in the given MCParticle graph.
     * @return true if the event was generated.
     */
    bool generateEvent(MCParticleGraph& graph);

    /**
     * Sets the parameters for generating the Particles
     */
    bool setParameters(const Parameters& parameters);

  protected:

    /**
     * All relevant parameters
     */
    Parameters m_params;

  private:

    /**
     * Finds maximum value in an array
     */
    double findMax(const double* dim, const int num);

    /**
     * Generates cosmic events according to tabulated distributions in 5-dimensional space:
     * dr, phi, Pt, dz, tanl.
     */
    void genCosmic(const int level, int& charge,
                   double& dr, double& phi, double& Pt, double& dz, double& tanl);

    /**
     * Generates the muon charge according to the positively/negatively charged muon ratio
     */
    int muChargeFlag(const double);

    /**
     * Generates distributions in 5-parameter space for different particle charges
     */
    void mkdist_v1(const int charge, double*);
    /**
     * Generates vertex distributions in the radial direction dr by accept-reject method
     * for positively charged particles
     */
    int mkDr_pos_v1(const double dr, const float rndm);
    /**
     * Generates vertex distributions in the radial direction dr by accept-reject method
     * for negatively
     */
    int mkDr_neg_v1(const double dr, const float rndm);
    /**
     * Generates azimuthal angle phi distributions by accept-reject method
     * for positively charged particles
     */
    int mkPhi_pos_v1(const double phi, const float rndm);
    /**
     * Generates azimuthal angle phi distributions by accept-reject method
     * for negatively charged particles
     */
    int mkPhi_neg_v1(const double phi, const float rndm);
    /**
     * Generates transverse momentum pt distributions by accept-reject method
     * for positively charged particles
     */
    int mkPt_pos_v1(const double pt, const float rndm);
    /**
     * Generates transverse momentum pt distributions by accept-reject method
     * for negatively charged particles
     */
    int mkPt_neg_v1(const double pt, const float rndm);
    /**
     * Generates z vertex dz distributions by accept-reject method
     * for positively charged particles
     */
    int mkDz_pos_v1(const double dz, const float rndm);
    /**
     * Generates z vertex dz distributions by accept-reject method
     * for negatively charged particles
     */
    int mkDz_neg_v1(const double dz, const float rndm);
    /**
     * Generates tangent of the polar angle tanl distributions by accept-reject method
     * for positively charged particles
     */
    int mkTanl_pos_v1(const double tanl, const float rndm);
    /**
     * Generates tangent of the polar angle tanl distributions by accept-reject method
     * for negatively charged particles
     */
    int mkTanl_neg_v1(const double tanl, const float rndm);

    /**
     * Generates distributions in 5-parameter space for different particle charges
     */
    void mkdist_v2(const int charge, double*);
    /**
     * Generates vertex distributions in the radial direction dr by accept-reject method
     * for positively charged particles
     */
    int mkDr_pos_v2(const double dr, const float rndm);
    /**
     * Generates vertex distributions in the radial direction dr by accept-reject method
     * for negatively charged particles
     */
    int mkDr_neg_v2(const double dr, const float rndm);
    /**
     * Generates azimuthal angle phi distributions by accept-reject method
     * for positively charged particles
     */
    int mkPhi_pos_v2(const double phi, const float rndm);
    /**
     * Generates azimuthal angle phi distributions by accept-reject method
     * for negatively charged particles
     */
    int mkPhi_neg_v2(const double phi, const float rndm);
    /**
     * Generates transverse momentum pt distributions by accept-reject method
     * for positively charged particles
     */
    int mkPt_pos_v2(const double pt, const float rndm);
    /**
     * Generates transverse momentum pt distributions by accept-reject method
     * for negatively charged particles
     */
    int mkPt_neg_v2(const double pt, const float rndm);
    /**
     * Generates z vertex dz distributions by accept-reject method
     * for positively charged particles
     */
    int mkDz_pos_v2(const double dz, const float rndm);
    /**
     * Generates z vertex dz distributions by accept-reject method
     * for negatively charged particles
     */
    int mkDz_neg_v2(const double dz, const float rndm);
    /**
     * Generates tangent of the polar angle tanl distributions by accept-reject method
     * for positively charged particles
     */
    int mkTanl_pos_v2(const double tanl, const float rndm);
    /**
     * Generates tangent of the polar angle tanl distributions by accept-reject method
     * for negatively charged particles
     */
    int mkTanl_neg_v2(const double tanl, const float rndm);

  };

} //end namespace Belle2

