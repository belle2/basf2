#ifndef ECL_CANVAS_POLAR
#define ECL_CANVAS_POLAR

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH2.h>
#include <TCrown.h>
#include <TText.h>
#include <TAxis.h>

namespace Belle2 {
  class EclPainterPolar : public EclPainter {
  public:
    enum Type {PHI, THETA};

    EclPainterPolar(EclData* data, Type type);
    ~EclPainterPolar();

  private:
    Type m_type;
    TH2F* m_hist;
    TCrown** m_segs;
    TText** m_labels;

    void InitHisto();
    int ChannelToSegId(int channel);

  public:
    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void GetInformation(int px, int py, MultilineWidget* panel);

    Type GetType();

    /**
     * @brief Redraw the canvas.
     */
    virtual void Draw();
  };
}

#endif // ECL_CANVAS_POLAR
