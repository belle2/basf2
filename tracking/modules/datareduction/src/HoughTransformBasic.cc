#include <tracking/modules/datareduction/HoughTransformBasic.h>

#include <cmath>
#include <iomanip>

using namespace std;
using namespace Belle2;


HoughTransformBasic::HoughTransformBasic()
{
  _minHoughBoxSizeParamA = 5.0e-3;
  _minHoughBoxSizeParamB = 5.0e-2;
  _minNumberTrackerHits = 3;

  _regionEnlWidthStart = 0.0;
  _regionEnlWidthEnd = 0.0;
  _regionEnlLengthStart = 0.0;
  _regionEnlLengthEnd = 0.0;
#ifdef CAIRO_OUTPUT
  cairo = 0;
#endif
}


HoughTransformBasic::~HoughTransformBasic()
{
  clearLists();
}


void HoughTransformBasic::setMinHoughBoxSizeParamA(double minSize)
{
  if (minSize > 0.0) _minHoughBoxSizeParamA = minSize;
}


void HoughTransformBasic::setMinHoughBoxSizeParamB(double minSize)
{
  if (minSize > 0.0) _minHoughBoxSizeParamB = minSize;
}


void HoughTransformBasic::setMinNumberTrackerHits(unsigned int minNumber)
{
  if (minNumber > 0) _minNumberTrackerHits = minNumber;
}


void HoughTransformBasic::setRegionEnlargement(double widthStart, double widthEnd, double lengthStart, double lengthEnd)
{
  _regionEnlWidthStart = widthStart;
  _regionEnlWidthEnd = widthEnd;
  _regionEnlLengthStart = lengthStart;
  _regionEnlLengthEnd = lengthEnd;
}

#ifdef CAIRO_OUTPUT
void HoughTransformBasic::drawBox(HoughTransformBox &box, box_state state)
{
  if (!cairo) return;
  cairo_color color;
  double x = min(box.left, box.right);
  double y = min(box.top, box.bottom);
  double width = fabs(box.left - box.right);
  double height = fabs(box.top - box.bottom);
  switch (state) {
    case INITIAL:
      /*      streamlog_out(MESSAGE4) << "box(" << setprecision(5);
            streamlog_out(MESSAGE4) << x << ",";
            streamlog_out(MESSAGE4) << y << ",";
            streamlog_out(MESSAGE4) << width << ",";
            streamlog_out(MESSAGE4) << height << ")" << endl;*/
      cairo_rectangle(cairo, 0.004, 0.001, 0.005, 0.004);
      cairo_set_source_rgb(cairo, 1, 0, 0);
      cairo_fill(cairo);
    case ITERATION:
      cairo_set_line_width(cairo, 0.05);
      color = cairo_color(0.8, 0.0, 0.0);
      cairo_set_source_rgba(cairo, color, 0.1);
      break;
    case FINAL:
      cairo_set_line_width(cairo, 0.2);
      color = cairo_color(0.0, 1.0, 0.0);
      cairo_set_source_rgb(cairo, color);
  }
  cairo_rectangle(cairo, x, y, width, height);
  //cairo_set_source_rgba(cairo,color,0.2);
  cairo_fill_preserve(cairo);
  cairo_set_source_rgb(cairo, 0, 0, 0);
  cairo_stroke_abs(cairo);
}

void HoughTransformBasic::drawRZ(cairo_t* cairo)
{
  cairo_set_source_rgb(cairo, 0, 0, 0.8);
  cairo_set_line_width(cairo, 0.5);
  for (list<ResultItem*>::iterator iRes = _resultSet.begin(); iRes != _resultSet.end(); iRes++) {
    drawResultItem(cairo, **iRes);
  }
  cairo_set_source_rgb(cairo, 1, 0, 0);
  for (list<rzHit*>::iterator iHit = _rzHits.begin(); iHit != _rzHits.end(); iHit++) {
    rzHit hit = **iHit;
    cairo_arc(cairo, hit.z, hit.r, 1, 0, 2*M_PI);
    cairo_fill(cairo);
  }
}
#endif

void HoughTransformBasic::doHoughSearch(SectorBasic& sector)
{
  clearLists();

  list<TrackerHit*>& hits = sector.getTrackerHitList();

  if (hits.size() < _minNumberTrackerHits) return;

  //Transform Hits to rz hits
  double currR;
  double currZ;
  list<TrackerHit*>::iterator trackListIter;
  for (trackListIter = hits.begin(); trackListIter != hits.end(); ++trackListIter) {
    TrackerHit* currHit = *trackListIter;

    currR = sqrt((currHit->getPosition()[0] * currHit->getPosition()[0]) +
                 (currHit->getPosition()[1] * currHit->getPosition()[1]));
    currZ = currHit->getPosition()[2];
    _rzHits.push_back(new rzHit(currR, currZ));
  }



  //Add initial hough box
  list<rzHit*>::iterator hitIter;
  double min_a = 0;
  double min_b = 0;
  double max_a = 0;
  double max_b = 0;
  getInitialHoughBoxParams(min_a, max_a, min_b, max_b);
  HoughTransformBox* newBox = new HoughTransformBox(min_a, max_a, min_b, max_b);
  for (hitIter = _rzHits.begin(); hitIter != _rzHits.end(); ++hitIter) newBox->containedHits.push_back(*hitIter);
  _houghBoxes.push_back(newBox);

#ifdef CAIRO_OUTPUT
  if (cairo) {
    cairo_plot(cairo, min_a, max_a, min_b, max_b);
    //cairo_plot(cairo,0,1,0,1);
    //drawBox(*newBox,INITIAL);
  }
#endif

  //Take first hough box from the list and check if the min. number of functions are inside
  //if there are enough fcts., split the box into 2 or 4 and append those to the end of the list.
  list<rzHit*> bufferHits;
  bool divideA = false;
  bool divideB = false;

  while (_houghBoxes.size() > 0) {
    bufferHits.clear();
    HoughTransformBox* currBox = _houghBoxes.front();
    _houghBoxes.pop_front();

    for (hitIter = currBox->containedHits.begin(); hitIter != currBox->containedHits.end(); ++hitIter) {
      rzHit* currHit = *hitIter;
      if (isHitInHoughBox(*currBox, *currHit)) bufferHits.push_back(currHit);
    }

    if (bufferHits.size() >= _minNumberTrackerHits) {
      if ((fabs(currBox->right - currBox->left) > _minHoughBoxSizeParamA) ||
          (fabs(currBox->top - currBox->bottom) > _minHoughBoxSizeParamB)) {

        divideA = false;
        divideB = false;

        double halfLeft = currBox->right;
        if (fabs(currBox->right - currBox->left) > _minHoughBoxSizeParamA) {
          halfLeft = 0.5 * (currBox->right + currBox->left);
          divideA = true;
        }

        double halfTop = currBox->bottom;
        if (fabs(currBox->top - currBox->bottom) > _minHoughBoxSizeParamB) {
          halfTop = 0.5 * (currBox->bottom + currBox->top);
          divideB = true;
        }

        //Divide into 2 or 4 boxes
        if (divideA) {
          newBox = new HoughTransformBox(currBox->left, halfLeft, currBox->top, halfTop);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);

          newBox = new HoughTransformBox(halfLeft, currBox->right, currBox->top, halfTop);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);
        }

        if ((divideB) && (!divideA)) {
          newBox = new HoughTransformBox(currBox->left, halfLeft, currBox->top, halfTop);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);

          newBox = new HoughTransformBox(currBox->left, halfLeft, halfTop, currBox->bottom);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);
        }

        if ((divideA) && (divideB)) {
          newBox = new HoughTransformBox(currBox->left, halfLeft, halfTop, currBox->bottom);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);

          newBox = new HoughTransformBox(halfLeft, currBox->right, halfTop, currBox->bottom);
          newBox->setHits(bufferHits);
          _houghBoxes.push_back(newBox);
        }
#ifdef CAIRO_OUTPUT
        drawBox(*currBox, ITERATION);
#endif

      } else {
        //Add box to the result set
#ifdef CAIRO_OUTPUT
        drawBox(*currBox, FINAL);
#endif

        _resultSet.push_back(new ResultItem(0.5*(currBox->right + currBox->left),
                                            0.5*(currBox->bottom + currBox->top),
                                            0.5*fabs(currBox->right - currBox->left),
                                            0.5*fabs(currBox->top - currBox->bottom)));
      }
    }
    delete currBox;
  }

// B2INFO("ResultSetNumber:" << _resultSet.size() << endl);

  //Create regions of interest
  createRegionsOfInterest(sector);
#ifdef CAIRO_OUTPUT
  drawHits(min_a, max_a, min_b, max_b);
#endif
}


void HoughTransformBasic::createRegionsOfInterest(SectorBasic& sector)
{
  list<LadderEntry>& ladderList = sector.getPXDLadderList();
  list<LadderEntry>::iterator ladderIter;
  list<ResultItem*>::iterator resIter;

  double minZValue = 0.0;
  double maxZValue = 0.0;
  double ladderWidth = 0.0;
  double ladderLength = 0.0;
  ResultItem* currResult;
  PXDLadder* currLadder;

  for (resIter = _resultSet.begin(); resIter != _resultSet.end(); ++resIter) {
    currResult = *resIter;

    for (ladderIter = ladderList.begin(); ladderIter != ladderList.end(); ++ladderIter) {
      currLadder = ladderIter->ladder;
      ladderWidth = currLadder->getSize()[0];
      ladderLength = currLadder->getSize()[2];

      minZValue = (getZValue(currResult->a, currResult->b, ladderIter->minRadius) - currLadder->getPosition()(2) + ladderLength / 2) / ladderLength;
      maxZValue = (getZValue(currResult->a, currResult->b, ladderIter->maxRadius) - currLadder->getPosition()(2) + ladderLength / 2) / ladderLength;

      currLadder->addRegion(
        ladderIter->start - _regionEnlWidthStart, ladderIter->end + _regionEnlWidthEnd,
        min(minZValue, maxZValue) - _regionEnlLengthStart, max(minZValue, maxZValue) + _regionEnlLengthEnd
      );

#ifdef CAIRO_OUTPUT
      currLadder->getRegionList().back()->color = cairo_color(sector.color[0], sector.color[1], sector.color[2]);
#endif

    }
  }
}

void HoughTransformBasic::clearLists()
{
  list<ResultItem*>::iterator resIter;
  for (resIter = _resultSet.begin(); resIter != _resultSet.end(); ++resIter) delete *resIter;
  list<HoughTransformBox*>::iterator boxIter;
  for (boxIter = _houghBoxes.begin(); boxIter != _houghBoxes.end(); ++boxIter) delete *boxIter;
  list<rzHit*>::iterator hitIter;
  for (hitIter = _rzHits.begin(); hitIter != _rzHits.end(); ++hitIter) delete *hitIter;

  _resultSet.clear();
  _houghBoxes.clear();
  _rzHits.clear();
}
