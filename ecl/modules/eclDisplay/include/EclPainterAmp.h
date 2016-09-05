#ifndef ECL_PAINTER_AMP
#define ECL_PAINTER_AMP

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH1.h>

namespace Belle2 {
  class EclPainterAmp : public EclPainter {
  public:
    EclPainterAmp(EclData* data, int amp_min, int amp_max);
    virtual ~EclPainterAmp();

  private:
    TH1F* m_hist;
    /// Min amplitude threshold
    int m_amp_min;
    /// Max amplitude threshold
    int m_amp_max;

    int GetMaxX();
    void InitHisto();

  public:
    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void GetInformation(int px, int py, MultilineWidget* panel);

    /**
     * @brief Redraw the canvas.
     */
    void Draw();
  };
}

#endif // ECL_PAINTER_AMP
