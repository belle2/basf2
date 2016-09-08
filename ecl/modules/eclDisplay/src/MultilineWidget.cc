#include <ecl/modules/eclDisplay/MultilineWidget.h>

using namespace Belle2;

MultilineWidget::MultilineWidget(const TGWindow* p, const char* title,
                                 int line_count) :
  TGGroupFrame(p, title)
{
  SetLayoutManager(new TGVerticalLayout(this));

  for (int i = 0; i < line_count; i++) {
    AddLine();
  }
}

MultilineWidget::~MultilineWidget()
{
  for (unsigned int i = 0; i < lines.size(); i++) {
    delete lines[i];
  }
}

int MultilineWidget::GetLineCount()
{
  return lines.size();
}

void MultilineWidget::SetLineCount(int new_count)
{
  int prev_count = GetLineCount();

  if (new_count > prev_count) {
    for (int i = new_count - prev_count; i > 0; i--)
      AddLine();
  } else {
    for (int i = prev_count - new_count; i > 0; i--)
      RemoveLine(new_count);
  }
}

void MultilineWidget::RemoveLine(int line_id)
{
  TGLabel* line = lines[line_id];

  RemoveFrame(line);
  lines.erase(lines.begin() + line_id);

  delete line;
}

void MultilineWidget::RemoveLastLine()
{
  RemoveLine(lines.size() - 1);
}
void MultilineWidget::SetLine(int line_id, const char* text)
{
  if (line_id >= GetLineCount())
    SetLineCount(line_id + 1);

  TGLabel* line = lines[line_id];

  line->SetText(text);
}
void MultilineWidget::AddLine(const char* text)
{
  TGLabel* line = new TGLabel(this, text);

  lines.push_back(line);
  AddFrame(line, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
  line->Paint();
}
