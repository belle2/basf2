/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Dennis Weyland                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <TLorentzRotation.h>
#include <TRotation.h>
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
     * @param 3-vector from particle
     * @return 3-vector in reference frame
     */
    virtual TVector3 getVertex(const TVector3& vector) const = 0;

    /**
     * Wrapper for particles
     * @param particle
     * @return 3-vector in reference frame
     */
    virtual TVector3 getVertex(const Particle* particle) const
    {
      return getVertex(particle->getVertex());
    }

    /**
     * Get momentum 4-vector in reference frame
     * @param momentum 4-vector from particle
     * @return momentum 4-vector in reference frame
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const = 0;

    /**
     * Wrapper for particles
     * @param  particle
     * @return momentum 4-vector in reference frame
     */
    virtual TLorentzVector getMomentum(const Particle* particle) const
    {
      return getMomentum(particle->get4Vector());
    }

    /**
     * Get Momentum error matrix in reference frame
     * @param Covariance matrix of particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const = 0;

    /**
     * Wrapper for particles
     * @param  particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const
    {
      return getMomentumErrorMatrix(particle->getMomentumErrorMatrix());
    }

    /**
     * Get Vertex error matrix in reference frame
     * @param Covariance matrix of particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const = 0;

    /**
     * Wrapper for particles
     * @param particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const Particle* particle) const
    {
      return getVertexErrorMatrix(particle->getVertexErrorMatrix());
    }

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
     * @param 3-vector from particle
     * @return 3-vector in rest frame System
     */
    virtual TVector3 getVertex(const TVector3& vector) const;

    /**
     * Get Lorentz vector in rest frame System
     * @param Lorentz vector from particle
     * @return Lorentz vector in rest frame System
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const;

    /**
     * Get Momentum error matrix in rest frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const;

    /**
     * Get Vertex error matrix in rest frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const;

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
     * @param 3-vector from particle
     * @return 3-vector in lab frame
     */
    virtual TVector3 getVertex(const TVector3& vector) const;

    /**
     * Get Lorentz vector in lab frame
     * @param Lorentz vector from particle
     * @return Lorentz vector in lab frame
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const;

    /**
     * Get Momentum error matrix in lab frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const;

    /**
     * Get Vertex error matrix in lab frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const;

  };

  /**
   * CMS frame
   */
  class CMSFrame : public ReferenceFrame {

  public:
    /**
     * Get vertex 3-vector in cms frame
     * @param 3-vector from particle
     * @return 3-vector in cms frame
     */
    virtual TVector3 getVertex(const TVector3& vector) const;

    /**
     * Get Lorentz vector in cms frame
     * @param Lorentz vector from particle
     * @return Lorentz vector in cms frame
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const;

    /**
     * Get Momentum error matrix in cms frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const;

    /**
     * Get Vertex error matrix in cms frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const;

  private:
    PCmsLabTransform m_transform; /**< Lab to CMS Transform */
  };

  /**
   * Rotation frame around vector
   */
  class RotationFrame : public ReferenceFrame {

  public:
    /**
     * Create new rotation frame
     */
    explicit RotationFrame(const TVector3& newX, const TVector3& newY, const TVector3& newZ);

    /**
     * Get vertex 3-vector in rotation frame
     * @param 3-vector from particle
     * @return 3-vector in rotation frame
     */
    virtual TVector3 getVertex(const TVector3& vector) const;

    /**
     * Get Lorentz vector in rotation frame
     * @param Lorentz vector from particle
     * @return Lorentz vector in rotation frame
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const;

    /**
     * Get Momentum error matrix in rotation frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const;

    /**
     * Get Vertex error matrix in rotation frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const;

  private:
    TRotation m_rotation; /**< Rotation */
  };

  /**
   * Stack frame for cms and Rotation frame
   */
  class CMSRotationFrame : public ReferenceFrame {

  public:
    /**
     * Create new rotation frame
     */
    explicit CMSRotationFrame(const TVector3& newX, const TVector3& newY, const TVector3& newZ);

    /**
     * Get vertex 3-vector in rotation frame
     * @param 3-vector from particle
     * @return 3-vector in rotation frame
     */
    virtual TVector3 getVertex(const TVector3& vector) const;

    /**
     * Get Lorentz vector in rotation frame
     * @param Lorentz vector from particle
     * @return Lorentz vector in rotation frame
     */
    virtual TLorentzVector getMomentum(const TLorentzVector& vector) const;

    /**
     * Get Momentum error matrix in rotation frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const;

    /**
     * Get Vertex error matrix in rotation frame
     * @param Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const;

  private:
    CMSFrame cmsframe; /**< CMSFrame*/
    RotationFrame rotationframe; /**< Rotationframe*/

  };

  /** A template class to apply the reference frame */
  template<class T>
  class UseReferenceFrame {
  public:
    /** Constructor which applies the reference frame  */
    template<class ...Args>
    UseReferenceFrame(Args&& ... params) : m_frame(std::forward<Args>(params)...)
    {
      ReferenceFrame::Push(&m_frame);
    }

    /** */
    ~UseReferenceFrame()
    {
      ReferenceFrame::Pop();
    }
  private:
    T m_frame; /**< the reference frame */
  };

} // Belle2 namespace

#endif
