/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Rotation3D.h>
#include <Math/Boost.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
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
     * @param vector 3-vector from particle
     * @return 3-vector in reference frame
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const = 0;

    /**
     * Wrapper for particles
     * @param particle
     * @return 3-vector in reference frame
     */
    virtual ROOT::Math::XYZVector getVertex(const Particle* particle) const
    {
      return getVertex(particle->getVertex());
    }

    /**
     * Get momentum 4-vector in reference frame
     * @param vector 4-vector from particle
     * @return momentum 4-vector in reference frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const = 0;

    /**
     * Wrapper for particles
     * @param particle particle
     * @return momentum 4-vector in reference frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const Particle* particle) const
    {
      return getMomentum(particle->get4Vector());
    }

    /**
     * Get Momentum error matrix in reference frame
     * @param matrix Covariance matrix of particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const = 0;

    /**
     * Wrapper for particles
     * @param particle particle
     * @return Covariance matrix in reference frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const Particle* particle) const
    {
      return getMomentumErrorMatrix(particle->getMomentumErrorMatrix());
    }

    /**
     * Get Vertex error matrix in reference frame
     * @param matrix Covariance matrix of particle
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
     * @param frame Use this reference frame
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
     * @param vector 3-vector from particle
     * @return 3-vector in rest frame System
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const override;

    /**
     * Get Lorentz vector in rest frame System
     * @param vector Lorentz vector from particle
     * @return Lorentz vector in rest frame System
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const override;

    /**
     * Get Momentum error matrix in rest frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const override;

    /**
     * Get Vertex error matrix in rest frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rest frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const override;

  private:
    ROOT::Math::PxPyPzEVector m_momentum; /**< momentum of RF in the lab frame */
    ROOT::Math::XYZVector m_displacement; /**< displacement of RF origin in th lab frame */
    ROOT::Math::XYZVector m_boost;        /**< boost of RF relative to the lab frame */
    ROOT::Math::Boost m_lab2restframe;    /**< Lorentz transformation connecting lab and rest frame */
  };

  /**
   * Lab frame
   */
  class LabFrame : public ReferenceFrame {

  public:

    /**
     * Get vertex 3-vector in lab frame
     * @param vector 3-vector from particle
     * @return 3-vector in lab frame
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const override;

    /**
     * Get Lorentz vector in lab frame
     * @param vector Lorentz vector from particle
     * @return Lorentz vector in lab frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const override;

    /**
     * Get Momentum error matrix in lab frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const override;

    /**
     * Get Vertex error matrix in lab frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in lab frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const override;

  };

  /**
   * CMS frame
   */
  class CMSFrame : public ReferenceFrame {

  public:
    /**
     * Get vertex 3-vector in cms frame
     * @param vector 3-vector from particle
     * @return 3-vector in cms frame
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const override;

    /**
     * Get Lorentz vector in cms frame
     * @param vector Lorentz vector from particle
     * @return Lorentz vector in cms frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const override;

    /**
     * Get Momentum error matrix in cms frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const override;

    /**
     * Get Vertex error matrix in cms frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in cms frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const override;

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
    explicit RotationFrame(const ROOT::Math::XYZVector& newX, const ROOT::Math::XYZVector& newY, const ROOT::Math::XYZVector& newZ);

    /**
     * Get vertex 3-vector in rotation frame
     * @param vector 3-vector from particle
     * @return 3-vector in rotation frame
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const override;

    /**
     * Get Lorentz vector in rotation frame
     * @param vector Lorentz vector from particle
     * @return Lorentz vector in rotation frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const override;

    /**
     * Get Momentum error matrix in rotation frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const override;

    /**
     * Get Vertex error matrix in rotation frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const override;

  private:
    ROOT::Math::Rotation3D m_rotation; /**< Rotation */
  };

  /**
   * Stack frame for cms and Rotation frame
   */
  class CMSRotationFrame : public ReferenceFrame {

  public:
    /**
     * Create new rotation frame
     */
    explicit CMSRotationFrame(const ROOT::Math::XYZVector& newX, const ROOT::Math::XYZVector& newY, const ROOT::Math::XYZVector& newZ);

    /**
     * Get vertex 3-vector in rotation frame
     * @param vector 3-vector from particle
     * @return 3-vector in rotation frame
     */
    virtual ROOT::Math::XYZVector getVertex(const ROOT::Math::XYZVector& vector) const override;

    /**
     * Get Lorentz vector in rotation frame
     * @param vector Lorentz vector from particle
     * @return Lorentz vector in rotation frame
     */
    virtual ROOT::Math::PxPyPzEVector getMomentum(const ROOT::Math::PxPyPzEVector& vector) const override;

    /**
     * Get Momentum error matrix in rotation frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getMomentumErrorMatrix(const TMatrixFSym& matrix) const override;

    /**
     * Get Vertex error matrix in rotation frame
     * @param matrix Covariance matrix from particle
     * @return Covariance matrix in rotation frame
     */
    virtual TMatrixFSym getVertexErrorMatrix(const TMatrixFSym& matrix) const override;

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
    explicit UseReferenceFrame(Args&& ... params) : m_frame(std::forward<Args>(params)...)
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

