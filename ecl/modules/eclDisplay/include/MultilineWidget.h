/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//STL
#include <vector>

//Root
#include <TGFrame.h>

class TGLabel;

namespace Belle2 {
  /**
   * Widget which contains the dynamic amount of TGLabel objects.
   * Compared to TGTextView, this class allows to update lines frequently without
   * without redrawing the entire text.
   * There is no internal checks for the correctness of remove operations.
   */
  class MultilineWidget : public TGGroupFrame {
  private:
    /**  Content of multiline widget. */
    std::vector<TGLabel*> lines;

  public:
    /**
     * Create multiline widget with parent window p.
     */
    MultilineWidget(const TGWindow* p = 0, const char* title = 0, int line_count = 0);
    virtual ~MultilineWidget();

    /**
     * Return number of lines in widget.
     */
    int getLineCount();
    /**
     * Add or remove lines depending on current line count.
     * If the specified count is less than previous, lines are removed from the
     * bottom of the widget.
     */
    void setLineCount(int count);
    /**
     * Remove line with specified id
     */
    void removeLine(int line_id);
    /**
     * Removes last line from multiline widget and reduces line count.
     */
    void removeLastLine();
    /**
     * Set content of the specified line to 'text'.
     */
    void setLine(int line_id, const char* text);
    /**
     * Append line to multiline widget.
     */
    void addLine(const char* text = 0);
  };
}
