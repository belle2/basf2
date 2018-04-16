/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/
//This module
#include <ecl/modules/eclDisplay/MultilineWidget.h>

//Root
#include <TGLabel.h>

using namespace Belle2;

MultilineWidget::MultilineWidget(const TGWindow* p, const char* title,
                                 int line_count) :
  TGGroupFrame(p, title)
{
  SetLayoutManager(new TGVerticalLayout(this));

  for (int i = 0; i < line_count; i++) {
    addLine();
  }
}

MultilineWidget::~MultilineWidget()
{
  for (unsigned int i = 0; i < lines.size(); i++) {
    delete lines[i];
  }
}

int MultilineWidget::getLineCount()
{
  return lines.size();
}

void MultilineWidget::setLineCount(int new_count)
{
  int prev_count = getLineCount();

  if (new_count > prev_count) {
    for (int i = new_count - prev_count; i > 0; i--)
      addLine();
  } else {
    for (int i = prev_count - new_count; i > 0; i--)
      removeLine(new_count);
  }
}

void MultilineWidget::removeLine(int line_id)
{
  TGLabel* line = lines[line_id];

  RemoveFrame(line);
  lines.erase(lines.begin() + line_id);

  delete line;
}

void MultilineWidget::removeLastLine()
{
  removeLine(lines.size() - 1);
}
void MultilineWidget::setLine(int line_id, const char* text)
{
  if (line_id >= getLineCount())
    setLineCount(line_id + 1);

  TGLabel* line = lines[line_id];

  line->SetText(text);
}
void MultilineWidget::addLine(const char* text)
{
  TGLabel* line = new TGLabel(this, text);

  lines.push_back(line);
  AddFrame(line, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
  line->Paint();
}
