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

#ifndef SGCOSMIC_H
#define SGCOSMIC_H

#include <framework/core/FrameworkExceptions.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <TRandom3.h>

namespace Belle2 {

  /**
   * Class to generate tracks in the cosmics generator and store them in a MCParticle graph.
   * The class supports two version (levels) of event generation with IP and pt parameters.
   */
  class SGCosmic {

  public:

    /**
     * Struct to keep all necessary parameters for the cosmic generator
     * level: version 1 (default) and 2 of the generator
     * ipRequirement: restrict the vertex to IP or not (default)
     * ipdr: vertex restriction in the radial direction
     * ipdz: vertex restriction in z direction
     * ptmin: minimum value of the transverse momentum
     */
    struct Parameters {
      int level;
      int ipRequirement;
      double ipdr;
      double ipdz;
      double ptmin;
    };

    /**
     * Default constructor
     */
    SGCosmic() {}

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
     * Distributions for the version (level) 1
     */
    void mkdist_v1(const int charge, double*);
    int mkDr_pos_v1(const double dr, const float rndm);
    int mkDr_neg_v1(const double dr, const float rndm);
    int mkPhi_pos_v1(const double phi, const float rndm);
    int mkPhi_neg_v1(const double phi, const float rndm);
    int mkPt_pos_v1(const double pt, const float rndm);
    int mkPt_neg_v1(const double pt, const float rndm);
    int mkDz_pos_v1(const double dz, const float rndm);
    int mkDz_neg_v1(const double dz, const float rndm);
    int mkTanl_pos_v1(const double tanl, const float rndm);
    int mkTanl_neg_v1(const double tanl, const float rndm);

    /**
     * Distributions for the version (level) 2
     */
    void mkdist_v2(const int charge, double*);
    int mkDr_pos_v2(const double dr, const float rndm);
    int mkDr_neg_v2(const double dr, const float rndm);
    int mkPhi_pos_v2(const double phi, const float rndm);
    int mkPhi_neg_v2(const double phi, const float rndm);
    int mkPt_pos_v2(const double pt, const float rndm);
    int mkPt_neg_v2(const double pt, const float rndm);
    int mkDz_pos_v2(const double dz, const float rndm);
    int mkDz_neg_v2(const double dz, const float rndm);
    int mkTanl_pos_v2(const double tanl, const float rndm);
    int mkTanl_neg_v2(const double tanl, const float rndm);

  };

} //end namespace Belle2

#endif //SGCOSMIC_H
