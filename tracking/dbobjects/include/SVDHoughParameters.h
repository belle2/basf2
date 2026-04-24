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

  /** The payload containing all parameters for the SVDHough. */

  class SVDHoughParameters: public TObject {
  public:
    /** Default constructor */
    SVDHoughParameters() : m_finalOverlapResolverMinActivityState(0.7), m_maxRelations(100), m_minQualitiyIndicatorSize3(0.5),
      m_minQualitiyIndicatorSize4(0.5), m_minQualitiyIndicatorSize5(0.5), m_refinerOverlapResolverMinActivityState(0.7) {}

    /** Destructor */
    ~SVDHoughParameters() {}

    /** Set minimal value of activity for acceptance */
    void setFinalOverlapResolverMinActivityState(float finalOverlapResolverMinActivityState)
    {
      m_finalOverlapResolverMinActivityState = finalOverlapResolverMinActivityState;
    }

    /** Get minimal value of activity for acceptance */
    float getFinalOverlapResolverMinActivityState() const
    {
      return m_finalOverlapResolverMinActivityState;
    }

    /** Set Maximum number of relations allowed for entering tree search */
    void setMaxRelations(int maxRelations)
    {
      m_maxRelations = maxRelations;
    }

    /** Get Maximum number of relations allowed for entering tree search */
    int getMaxRelations() const
    {
      return m_maxRelations;
    }

    /** Set the minimum QI for SpacePointTrackCands of size 3 */
    void setMinQualitiyIndicatorSize3(float minQualitiyIndicatorSize3)
    {
      m_minQualitiyIndicatorSize3 = minQualitiyIndicatorSize3;
    }

    /** Get the minimum QI for SpacePointTrackCands of size 3 */
    float getMinQualitiyIndicatorSize3() const
    {
      return m_minQualitiyIndicatorSize3;
    }

    /** Set the minimum QI for SpacePointTrackCands of size 4 */
    void setMinQualitiyIndicatorSize4(float minQualitiyIndicatorSize4)
    {
      m_minQualitiyIndicatorSize4 = minQualitiyIndicatorSize4;
    }

    /** Get the minimum QI for SpacePointTrackCands of size 3 */
    float getMinQualitiyIndicatorSize4() const
    {
      return m_minQualitiyIndicatorSize4;
    }

    /** Set the minimum QI for SpacePointTrackCands of size 5 */
    void setMinQualitiyIndicatorSize5(float minQualitiyIndicatorSize5)
    {
      m_minQualitiyIndicatorSize5 = minQualitiyIndicatorSize5;
    }

    /** Get the minimum QI for SpacePointTrackCands of size 5 */
    float getMinQualitiyIndicatorSize5() const
    {
      return m_minQualitiyIndicatorSize5;
    }

    /** Set minimal value of activity for acceptance**/
    void setRefinerOverlapResolverMinActivityState(float refinerOverlapResolverMinActivityState)
    {
      m_refinerOverlapResolverMinActivityState = refinerOverlapResolverMinActivityState;
    }

    /** Get minimal value of activity for acceptance* */
    float getRefinerOverlapResolverMinActivityState() const
    {
      return m_refinerOverlapResolverMinActivityState;
    }

  private:
    /** minimal value of activity for acceptance */
    float m_finalOverlapResolverMinActivityState;
    /** Maximum number of relations allowed for entering tree search */
    unsigned int m_maxRelations;
    /** minimum QI for SpacePointTrackCands of size 3*/
    float m_minQualitiyIndicatorSize3;
    /** minimum QI for SpacePointTrackCands of size 4*/
    float m_minQualitiyIndicatorSize4;
    /** minimum QI for SpacePointTrackCands of size 5*/
    float m_minQualitiyIndicatorSize5;
    /** minimal value of activity for acceptance*/
    float m_refinerOverlapResolverMinActivityState;

    ClassDef(SVDHoughParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
