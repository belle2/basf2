#ifndef ECL_CANVAS_1D
#define ECL_CANVAS_1D

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH1.h>

namespace Belle2 {
  class EclPainter1D : public EclPainter {
  public:
    enum Type {CHANNEL, SHAPER, CRATE};

    EclPainter1D(EclData* data, Type type);
    virtual ~EclPainter1D();

  private:
    Type m_type;
    TH1F* m_hist;

    int ChannelToSegId(int channel);
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

    Type GetType();

    virtual EclPainter* HandleClick(int px, int py);

    void SetXRange(int xmin, int xmax);

    /**
     * @brief Redraw the canvas.
     */
    void Draw();
  };
}

#endif // ECL_CANVAS_1D
