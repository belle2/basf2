/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Pablo Goldenzweig                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLECONTINUUMSUPPRESSIONTOOL_H
#define NTUPLECONTINUUMSUPPRESSIONTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes continuum suppression variables to flat ntuple. */
  class NtupleContinuumSuppressionTool : public NtupleFlatTool {
  private:

    bool m_useKsfwFS1;  /**< Flag to calculate KSFW moments using B final state particles (FS1). Default is set to calculate from B daughters (FS0). */
    bool m_useCcROE;    /**< Flag to calculate CleoCones using ROE particles only. Default is set to calculate from all final state particles. */

    float m_fThrustB; /**< Magnitude of B thrust axis */
    float m_fThrustO; /**< Magnitude of ROE thrust axis */

    float m_fCosTBTO; /**< Cosine of the angle between the thrust axis of the B and the thrust axis of the ROE */
    float m_fCosTBz;  /**< Cosine of the angle between the thrust axis of the B and the z-axis */

    float m_fR2;      /**< Reduced Fox-Wolfram moment R2 */

    float m_fmm2;     /**< Missing mass squared */
    float m_fet;      /**< Transverse energy */
    float m_fhso00;   /**< Hso(0,0) */
    float m_fhso01;   /**< Hso(0,1) */
    float m_fhso02;   /**< Hso(0,2) */
    float m_fhso03;   /**< Hso(0,3) */
    float m_fhso04;   /**< Hso(0,4) */
    float m_fhso10;   /**< Hso(1,0) */
    float m_fhso12;   /**< Hso(1,2) */
    float m_fhso14;   /**< Hso(1,4) */
    float m_fhso20;   /**< Hso(2,0) */
    float m_fhso22;   /**< Hso(2,2) */
    float m_fhso24;   /**< Hso(2,4) */
    float m_fhoo0;    /**< Roo(0) */
    float m_fhoo1;    /**< Roo(1) */
    float m_fhoo2;    /**< Roo(2) */
    float m_fhoo3;    /**< Roo(3) */
    float m_fhoo4;    /**< Roo(4) */

    float m_fcc1;     /**< Cleo Cone 1 */
    float m_fcc2;     /**< Cleo Cone 2 */
    float m_fcc3;     /**< Cleo Cone 3 */
    float m_fcc4;     /**< Cleo Cone 4 */
    float m_fcc5;     /**< Cleo Cone 5 */
    float m_fcc6;     /**< Cleo Cone 6 */
    float m_fcc7;     /**< Cleo Cone 7 */
    float m_fcc8;     /**< Cleo Cone 8 */
    float m_fcc9;     /**< Cleo Cone 9 */

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleContinuumSuppressionTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions)
    {
      m_useKsfwFS1 = false;
      m_useCcROE = false;
      setupTree();
    }
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLECONTINUUMSUPPRESSIONTOOL_H
