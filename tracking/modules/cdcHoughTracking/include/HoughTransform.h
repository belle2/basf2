/**
 * @author Jan Bauer
 *
 */

#ifndef HOUGHTRANSFORM_H_
#define HOUGHTRANSFORM_H_
#include <tracking/modules/cdcHoughTracking/Coordinates.h>
#include <tracking/modules/cdcHoughTracking/Angle.h>
#include <tracking/modules/cdcHoughTracking/Tools.h>
#include <tracking/modules/cdcHoughTracking/CDCHoughHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <genfit/GFTrackCand.h>
#include <vector>
#include <algorithm>


namespace Belle2 {
  namespace Tracking {
    class HoughTransform {
    public:
      HoughTransform();
      HoughTransform(const unsigned int precision, const unsigned int pBins, const unsigned int cBins, const std::vector<CDCHoughHit>* hits);
      ~HoughTransform();
      void vote();
      void peakFinder(const int charge);
      void trackBuilder(const int charge);
      void zFinder(const int charge);

      void printDebugInfo();

      float getCScale() const {
        return m_cScale;
      }

      float getPScale() const {
        return m_pScale;
      }

      const std::vector<std::vector<int> >& getBinsN() const {
        return m_binsN;
      }

      const std::vector<std::vector<int> >& getBinsP() const {
        return m_binsP;
      }

      const std::vector<GFTrackCand>& getGfTracks() const {
        return m_gfTracks;
      }

    private:
      const unsigned int m_precision;
      const unsigned int m_pBins;
      const unsigned int m_cBins;


      const std::vector<CDCHoughHit>* m_hits;
      std::vector<std::vector<int> > m_binsN;
      std::vector<std::vector<int> > m_binsP;

      const float m_pScale;
      const float m_cScale;

      std::vector<Coordinates> m_candidatesN;
      std::vector<Coordinates> m_candidatesP;
      std::vector<Track> m_tracksN;
      std::vector<Track> m_tracksP;
      std::vector<GFTrackCand> m_gfTracks;
    };

    Coordinates getCenterFromBin(const float phi, const float curve, const HoughTransform& hough);


  }
}

#endif /* CDCHOUGHTRANSFORMCURVE_H_ */
