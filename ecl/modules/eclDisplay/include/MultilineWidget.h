/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

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
    /// Content of multiline widget.
    std::vector<TGLabel*> lines;

  public:
    /**
     * @brief Create multiline widget with parent window p.
     */
    MultilineWidget(const TGWindow* p = 0, const char* title = 0, int line_count = 0);
    virtual ~MultilineWidget();

    /**
     * @brief Return number of lines in widget.
     */
    int getLineCount();
    /**
     * @brief Add or remove lines depending on current line count.
     * If the specified count is less than previous, lines are removed from the
     * bottom of the widget.
     */
    void setLineCount(int count);
    /**
     * @brief Remove line with specified id
     */
    void removeLine(int line_id);
    /**
     * @brief Removes last line from multiline widget and reduces line count.
     */
    void removeLastLine();
    /**
     * @brief Set content of the specified line to 'text'.
     */
    void setLine(int line_id, const char* text);
    /**
     * @brief Append line to multiline widget.
     */
    void addLine(const char* text = 0);
  };
}

#endif // MULTILINE_WIDGET
