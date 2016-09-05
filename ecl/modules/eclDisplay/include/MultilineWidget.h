#ifndef MULTILINE_WIDGET
#define MULTILINE_WIDGET

#include <vector>
#include <TGLabel.h>
#include <TGFrame.h>

namespace Belle2 {
  /**
   * @brief Widget which contains the dynamic amount of TGLabel objects.
   * Compared to TGTextView, this class allows to update lines frequently without
   * without redrawing the entire text.
   * There is no internal checks for the correctness of remove operations.
   */
  class MultilineWidget : public TGGroupFrame {
  private:
    std::vector<TGLabel*> lines;

  public:
    MultilineWidget(const TGWindow* p = 0, const char* title = 0, int line_count = 0);
    virtual ~MultilineWidget();

    int GetLineCount();
    /**
     * @brief Add or remove lines depending on current line count.
     * If the specified count is less than previous, lines are removed from the
     * bottom of the widget.
     */
    void SetLineCount(int count);
    void RemoveLine(int line_id);
    void RemoveLastLine();
    void SetLine(int line_id, const char* text);
    void AddLine(const char* text = 0);
  };
}

#endif // MULTILINE_WIDGET
