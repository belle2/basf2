/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <TLorentzRotation.h>
#include <TLorentzVector.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <stack>

namespace Belle2 {

  /**
   * Abstract base class of all reference frames
   */
  class ReferenceFrame {
  public:
    /**
     * Get vertex 3-vector in reference frame
     * @param particle
     * @return 3-vector in reference frame
     */
    virtual TVector3 getVertex(const Particle* particle) const = 0;

    /**
     * Get momentum 4-vector in reference frame
     * @param particle
     * @return momentum 4-vector in reference frame
     */
    virtual TLorentzVector getMomentum(const Particle* particle) const = 0;

    /**
     * Get Momentum error matrix in reference frame
     * @param particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const = 0;

    /**
     * Get Vertex error matrix in reference frame
     * @param particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const Particle* particle) const = 0;

    /**
     * Get current rest frame
     * @return RestFrame
     */
    static const ReferenceFrame& GetCurrent();

  private:
    /**
     * Push rest frame of given particle
     * @param particle Use RestFrame of this particle
     */
    static void Push(const ReferenceFrame* frame)
    {
      m_reference_frames.push(frame);
    }

    /**
     * Pop current rest frame
     */
    static void Pop()
    {
      m_reference_frames.pop();
    }

    static std::stack<const ReferenceFrame*> m_reference_frames; /**< Stack of current rest frames */

    template<class T>
    friend class UseReferenceFrame;
  };

  /**
  * Rest frame of a particle
  */
  class RestFrame : public ReferenceFrame {

  public:
    /**
     * Create new rest frame
     */
    explicit RestFrame(const Particle* particle);

    /**
     * Get vertex 3-vector in  rest frame system
     * @param particle
     * @return 3-vector in rest frame System
     */
    virtual TVector3 getVertex(const Particle* particle) const;

    /**
     * Get Lorentz vector in rest frame System
     * @param particle
     * @return Lorentz vector in rest frame System
     */
    virtual TLorentzVector getMomentum(const Particle* particle) const;

    /**
     * Get Momentum error matrix in rest frame
     * @param particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const;

    /**
     * Get Vertex error matrix in rest frame
     * @param particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const Particle* particle) const;

  private:
    TLorentzVector m_momentum;         /**< momentum of RF in the lab frame */
    TVector3 m_displacement;           /**< displacement of RF origin in th lab frame */
    TLorentzRotation m_lab2restframe;  /**< Lorentz transformation connecting lab and rest frame */
    TVector3 m_boost;                  /**< boost of RF relative to the lab frame */
  };

  /**
   * Lab frame
   */
  class LabFrame : public ReferenceFrame {

  public:

    /**
     * Get vertex 3-vector in lab frame
     * @param particle
     * @return 3-vector in lab frame
     */
    virtual TVector3 getVertex(const Particle* particle) const;

    /**
     * Get Lorentz vector in lab frame
     * @param particle
     * @return Lorentz vector in lab frame
     */
    virtual TLorentzVector getMomentum(const Particle* particle) const;

    /**
     * Get Momentum error matrix in lab frame
     * @param particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const;

    /**
     * Get Vertex error matrix in lab frame
     * @param particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const Particle* particle) const;

  };

  /**
   * CMS frame
   */
  class CMSFrame : public ReferenceFrame {

  public:
    /**
     * Get vertex 3-vector in cms frame
     * @param particle
     * @return 3-vector in cms frame
     */
    virtual TVector3 getVertex(const Particle* particle) const;

    /**
     * Get Lorentz vector in cms frame
     * @param particle
     * @return Lorentz vector in cms frame
     */
    virtual TLorentzVector getMomentum(const Particle* particle) const;

    /**
     * Get Momentum error matrix in cms frame
     * @param particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const;

    /**
     * Get Vertex error matrix in cms frame
     * @param particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const Particle* particle) const;

  private:
    PCmsLabTransform m_transform; /**< Lab to CMS Transform */
  };

  template<class T>
  class UseReferenceFrame {
  public:
    template<class ...Args>
    UseReferenceFrame(Args&& ... params) : m_frame(std::forward<Args>(params)...)
    {
      ReferenceFrame::Push(&m_frame);
    }

    ~UseReferenceFrame()
    {
      ReferenceFrame::Pop();
    }
  private:
    T m_frame;
  };

} // Belle2 namespace

#endif
