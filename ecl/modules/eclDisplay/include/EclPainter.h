#ifndef ECL_CANVAS
#define ECL_CANVAS

#include <ecl/modules/eclDisplay/EclData.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>

namespace Belle2 {
  class EclPainter {
  private:
    static int m_obj_counter;
    EclData* m_ecl_data;

  public:
    EclPainter(EclData* data);
    virtual ~EclPainter();

    void SetData(EclData* data);
    EclData* GetData();

    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void GetInformation(int px, int py, MultilineWidget* panel);

    virtual EclPainter* HandleClick(int px, int py);

    virtual void SetXRange(int x1, int x2);

    /**
     * @brief Redraw the canvas.
     */
    virtual void Draw() = 0;

  protected:
    void GetNewRootObjectName(char* buf, int size);
  };
}

#endif // ECL_CANVAS
