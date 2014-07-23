//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : PeakFinder.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <cstdlib>
#include "trg/trg/Debug.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/PeakFinder.h"
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

////////////////////////////////////////////////save data

#include <iostream>
#include <fstream>
#include <math.h>
////////////////////////////////////////////////save data

using namespace std;

namespace Belle2 {

string
TRGCDCPeakFinder::version(void) const {
    return string("TRGCDCPeakFinder 5.04");
}

TRGCDCPeakFinder::TRGCDCPeakFinder(const string & name)
    : _name(name) {
}

TRGCDCPeakFinder::~TRGCDCPeakFinder() {
}

void
TRGCDCPeakFinder::doit(vector<TCCircle *> & circles,
                       TCHPlane & hp,
                       const unsigned threshold,
                       const bool centerIsPeak) const {
    return peaks5(circles, hp, threshold, centerIsPeak);
}

void
TRGCDCPeakFinder::peaks5(vector<TCCircle *> & circles,
                         TCHPlane & hp,
                         const unsigned threshold,
                         const bool centerIsPeak) const {

    TRGDebug::enterStage("Peak Finding (peaks5)");
    if (TRGDebug::level())
        cout << TRGDebug::tab() << "threshold=" << threshold
             << ",plane name=[" << hp.name() << "]" << endl;

    const unsigned nCircles = circles.size();

    //...Make connected regions (is this the best way???)...
    regions(hp, threshold);

    //...Determine peaks...
    const vector<vector<unsigned> *> & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        TRGDebug::enterStage("Peak position determination");
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "region " << i << " contents" << endl;

        //...Calculate size and center of a region...
        const vector<unsigned> & r = * regions[i];
        unsigned minX = hp.nX();
        unsigned maxX = 0;
        unsigned minY = hp.nY();
        unsigned maxY = 0;
        for (unsigned j = 0; j < (unsigned) r.size(); j++) {
            const unsigned s = r[j];
            unsigned x = 0;
            unsigned y = 0;
            hp.id(s, x, y);
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab(4) << "x=" << x << ",y=" << y << endl;
        }
        const unsigned cX = minX + (maxX - minX) / 2;
        const unsigned cY = minY + (maxY - minY) / 2;

        //...Determine a center of a region...
        unsigned ncX = hp.nX() * hp.nY();
        unsigned ncY = ncX;
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "center of region:x=" << cX << ",y="
                 << cY << endl;
        if (! centerIsPeak) {

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab() << "Searching a cell closest to the "
                     << "region center" << endl;

            //...Search for a cell which is the closest to the center...
            float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                unsigned x = 0;
                unsigned y = 0;
                hp.id(s, x, y);

                const float diff2 = (float(x) - float(cX)) *
                    (float(x) - float(cX))
                    + (float(y) - float(cY)) *
                    (float(y) - float(cY));

                if (diff2 < minDiff2) {
                    minDiff2 = diff2;
                    ncX = x;
                    ncY = y;
                }
                if (TRGDebug::level() > 1)
                    cout << TRGDebug::tab(4) << "x=" << ncX << ",y=" << ncY
                         << ":diff2=" << diff2 << endl;
            }
        }
        else {

            //...Search for a peak...
            float max = 0;
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                const float entry = hp.entry(s);
                if (max < entry) {
                    max = entry;
                    unsigned x = 0;
                    unsigned y = 0;
                    hp.id(s, x, y);
                    ncX = x;
                    ncY = y;
                }
            }
        }

        //...Store a circle...
        const TCHTransformationCircle * tc =
            dynamic_cast<const TCHTransformationCircle *>
            (& hp.transformation());
        TRGPoint2D center =
            tc->circleCenter(hp.position(ncX, ncY));
        circles.push_back(new TCCircle(center.y(),
                                       center.x(),
                                       hp.charge(),
                                       hp));

        if (TRGDebug::level()) {
            cout << TRGDebug::tab() << "region " << i << " final center:x="
                 << ncX << ",y=" << ncY << endl
                 << TRGDebug::tab(4) << "position in HP:x="
                 << hp.position(ncX, ncY).x() << ",y="
                 << hp.position(ncX, ncY).y() << endl;
            cout << TRGDebug::tab() << "A circle made" << endl;
            circles.back()->dump("", TRGDebug::tab(4));
        }

        TRGDebug::leaveStage("Peak position determination");
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << circles.size() - nCircles << " circle(s)"
             << " made in total" << endl;

    TRGDebug::leaveStage("Peak Finding (peaks5)");
    return;
}

void
TRGCDCPeakFinder::regions(TCHPlane & hp, const unsigned threshold) const {

    TRGDebug::enterStage("Making regions");

    //...Search cells above threshold...
    unsigned nCells = hp.nX() * hp.nY();
    static unsigned * candidates =
        (unsigned *) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 11; j < hp.nY(); j++) {
      //minus x direction ,  plus  -x direction
      if ((hp.name())=="circle hough minus")
	  for (unsigned i = 0; i < hp.nX(); i++) {
            //...Threshold check...
            const unsigned n = hp.entry(i, j);
            if (n < threshold) continue;
            candidates[nActive] = hp.serialId(i, j);
            ++nActive;
	}
      else
          for (unsigned z = hp.nX(); z> 0 ; --z) {
 	    //...Threshold check...
	    unsigned i=0;
	    i=z-1;
            const unsigned n = hp.entry(i, j);
            if (n < threshold) continue;
            candidates[nActive] = hp.serialId(i, j);
            ++nActive;
        }
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << "Active cells=" << nActive << endl;

    //...Loop over active cells...
    const unsigned used = nCells;
    for (unsigned i = 0; i < nActive; i++) {
        if (candidates[i] == used) continue;
        const unsigned id0 = candidates[i];
        candidates[i] = used;

        //...Make a new region...
        vector<unsigned> * region = new vector<unsigned>;
        region->push_back(id0);
        if (TRGDebug::level() > 2)
            cout << TRGDebug::tab(4) << "new region made" << endl;

        //...Search neighbors...
        for (unsigned j = 0; j < nActive; j++) {
            if (candidates[j] == used) continue;
            const unsigned id1 = candidates[j];

            unsigned x1 = 0;
            unsigned y1 = 0;
            hp.id(id1, x1, y1);
            if (TRGDebug::level() > 2)
                cout << TRGDebug::tab(8) << "cell:x=" << x1 << ",y=" << y1
                     << endl;

            for (unsigned k = 0; k < unsigned(region->size()); k++) {
                unsigned id2 = (* region)[k];
                unsigned x2 = 0;
                unsigned y2 = 0;
                hp.id(id2, x2, y2);
                int difx = abs(int(x1) - int(x2));
                int dify = abs(int(y1) - int(y2));
                if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
                if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;

                if (TRGDebug::level() > 2) {
                    cout << TRGDebug::tab(12) << "x=" << x2 << ",y=" << y2
                         << ":difx=" << difx << ",dify=" << dify;
                    if ((difx < 2) && (dify < 3))
                        cout << " ... connected" << endl;
                    else
                        cout << endl;
                }

                if ((difx < 2) && (dify < 3)) {
                    region->push_back(id1);
                    candidates[j] = used;
                    break;
                }

            }
        }
        hp.setRegion(region);
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << "Regions=" << hp.regions().size() << endl;

    TRGDebug::leaveStage("Making regions");
}

////////////////////////////////////////////////

///...rlrel...
bool rlrel(vector<unsigned short> a, vector<unsigned short> b) {
bool t=true;
//case 1 
if(b[1]==0 && b[3]==0){
t=false;}
//case 2
if(a[2]==0 && a[4]==0){
t=false;}
return t;
}
///...udrel...
bool udrel(vector<unsigned short> a, vector<unsigned short> b) {
bool t=true;
//case 1 
if(a[1]==0 && a[2]==0 && a[3]==0 && a[4]==0){
t=false;}
//case 2
if(a[3]==0 && a[4]==0 && b[1]==0 && b[2]==0){
t=false;}
//case 3 
if(b[1]==0 && b[2]==0 && b[3]==0 && b[4]==0){
t=false;}

return t;
}
///...mirel...
bool mirel(vector<unsigned short> a, vector<unsigned short> b) {
bool t=true;
//case 1 
if(a[4]==0 && b[1]==0){
t=false;}
//case 2 
if(a[2]==0 && a[4]==0){
t=false;}
//case 3 
if(b[1]==0 && b[3]==0){
t=false;}
return t;
}

///...Find center Pattern1 form Pattern 2 
unsigned short FindCP1 (vector<unsigned short> a){
unsigned center=0;
//...1...
if (a[0]==1 && a[1]==0 && a[3]==0 && a[4]==0 && a[6]==0 && a[7]==0 ){center=0;}
//...2...
if (a[0]==1 && a[1]==1 && a[3]==0 && a[4]==0 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==0 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==0 && a[3]==0 && a[4]==1 && a[6]==0 && a[7]==0 ){center=0;}
//...3...
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==0 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==1 && a[3]==0 && a[4]==1 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==1 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==0 && a[6]==1 && a[7]==0 ){center=3;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==0 && a[6]==0 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==0 && a[3]==0 && a[4]==1 && a[6]==1 && a[7]==0 ){center=4;}
if (a[0]==1 && a[1]==0 && a[3]==0 && a[4]==1 && a[6]==0 && a[7]==1 ){center=4;}
//...4...
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==1 && a[6]==0 && a[7]==0 ){center=0;}
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==0 && a[6]==1 && a[7]==0 ){center=3;}
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==0 && a[6]==0 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==1 && a[3]==0 && a[4]==1 && a[6]==1 && a[7]==0 ){center=4;}
if (a[0]==1 && a[1]==1 && a[3]==0 && a[4]==1 && a[6]==0 && a[7]==1 ){center=4;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==1 && a[6]==1 && a[7]==0 ){center=3;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==1 && a[6]==0 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==0 && a[6]==1 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==0 && a[3]==0 && a[4]==1 && a[6]==1 && a[7]==1 ){center=4;}
//...5...
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==1 && a[6]==1 && a[7]==0 ){center=3;}
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==1 && a[6]==0 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==0 && a[6]==1 && a[7]==1 ){center=3;}
if (a[0]==1 && a[1]==1 && a[3]==0 && a[4]==1 && a[6]==1 && a[7]==1 ){center=4;}
if (a[0]==1 && a[1]==0 && a[3]==1 && a[4]==1 && a[6]==1 && a[7]==1 ){center=3;}
//...6...
if (a[0]==1 && a[1]==1 && a[3]==1 && a[4]==1 && a[6]==1 && a[7]==1 ){center=3;}
return center;
}

///...Pattern1 Center...
unsigned FindP1C(vector<unsigned short> a){
unsigned hits=0;
unsigned short center=0;
for (unsigned short k=1; k<a.size(); k++){
if (a[k]==1){hits++;}
}
//...1...
if(hits==1){
if (a[1]==1 && a[2]==0 && a[3]==0 && a[4]==0 ){center=1;}
if (a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0 ){center=2;}
if (a[1]==0 && a[2]==0 && a[3]==1 && a[4]==0 ){center=3;}
if (a[1]==0 && a[2]==0 && a[3]==0 && a[4]==1 ){center=4;}

}
//...2...
if(hits==2){
if (a[1]==1 && a[2]==1 && a[3]==0 && a[4]==0 ){center=1;}
if (a[1]==1 && a[2]==0 && a[3]==1 && a[4]==0 ){center=1;}
if (a[1]==1 && a[2]==0 && a[3]==0 && a[4]==1 ){center=1;}


if (a[1]==0 && a[2]==1 && a[3]==1 && a[4]==0 ){center=2;}
if (a[1]==0 && a[2]==1 && a[3]==0 && a[4]==1 ){center=2;}

if (a[1]==0 && a[2]==0 && a[3]==1 && a[4]==1 ){center=3;}

}
//...3...
if(hits==3){
if (a[1]==1 && a[2]==1 && a[3]==1 && a[4]==0 ){center=1;}
if (a[1]==1 && a[2]==1 && a[3]==0 && a[4]==1 ){center=2;}
if (a[1]==1 && a[2]==0 && a[3]==1 && a[4]==1 ){center=3;}
if (a[1]==0 && a[2]==1 && a[3]==1 && a[4]==1 ){center=4;}


}
//...4...
if(hits==4){
if (a[1]==1 && a[2]==1 && a[3]==1 && a[4]==1 ){center=3;}

}

return center;
}


////////////////////////////////////////////////
////
void
TRGCDCPeakFinder::p1p2(TCHPlane & hp, const unsigned threshold) const{

    TRGDebug::enterStage("p1p2");

    //...Search cells above threshold...
    unsigned nCells = hp.nX() * hp.nY();
    static unsigned * candidates =
        (unsigned *) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 11; j < hp.nY(); j++) {
      //minus x direction ,  plus  -x direction
      if ((hp.name())=="circle hough minus")
	  for (unsigned i = 0; i < hp.nX(); i++) {
            //...Threshold check...
            const unsigned n = hp.entry(i, j);
            if (n < threshold) continue;
            candidates[nActive] = hp.serialId(i, j);
            ++nActive;
	}
      else
          for (unsigned z = hp.nX(); z> 0 ; --z) {
 	    //...Threshold check...
	    unsigned i=0;
	    i=z-1;
            const unsigned n = hp.entry(i, j);
            if (n < threshold) continue;
            candidates[nActive] = hp.serialId(i, j);
            ++nActive;
        }
    }

		vector<unsigned short> p;
		vector< vector<unsigned short> > p1m;
		vector< vector<unsigned short> > peak_xy;
		unsigned short no=0;
	//...create pattern1...begin
	for (unsigned n=0; n<13; n++){
		for (unsigned m=0; m<80; m++){
		unsigned a=0;
		unsigned b=0;
		bool ot=false;
		++no;
		p.push_back(no);
		a=m*2;
		b=n*2;
//...find 4 cells...begin
		for(unsigned j=0; j<2; j++){
				unsigned yy=0;
				yy=11+b+j;
				//cout <<"y="<<yy<<endl;//testp1
			for(unsigned k=0; k<2; k++){
				unsigned xx=0;
				xx=a+k;
				//cout <<"x="<<xx<<endl;//testp1
				///Plus plane transform
				if ((hp.name())=="circle hough plus"){
				int ixx=xx;
				xx=abs(ixx-159);
									}

				unsigned short t=0;
        			for (unsigned i=0; i< nActive; i++){
               				 unsigned id1=candidates[i];
                			 unsigned x1=0;
                			 unsigned y1=0;
               				 hp.id(id1,x1,y1);
				if(xx==x1 && yy==y1){
				t=1;
				ot=true;
				break;
							}
				          			   	}
				p.push_back(t);
						   	 }
					    	}
//...find 4 cells...end
	if (ot==true){
	p1m.push_back(p);
	}

/*for(vector<vector<unsigned short> >::size_type l=0;l<p1m.size();l++)
{for(vector<unsigned short>::size_type m=0;m<p1m[l].size();m++)
cout<<"p1m["<<l<<"]="<<p1m[l][m]<<endl;
cout <<"========"<<endl; } *///testp1
//for(vector<unsigned short>::size_type l=0;l<p.size();l++)
//cout <<"p["<<no<<"]="<<p[l]<<endl;
//cout <<"========"<<endl;  //testp1
	vector<unsigned short>().swap(p);
							}
						}
cout << "size of p1m=" << p1m.size() << endl;
cout << "~~~~~~~~~~~~~~~~~~~~~~~~~pattern1~~~~~~~~~~~~~~~~~~~" << endl;
//...create pattern1...end   (output p1m)


//...Pattern2 & Find Peak...begin
	cout << ">>>>>>>>>>Pattern 2 & Find Peak Begin!!!>>>>>>>>>>" << endl;
	vector<unsigned short> p0(5,0);
	vector< vector<unsigned short> > op2;
///test(CFI)
//unsigned short test_c=0;
//unsigned short test_f=0;
//unsigned short test_i=0;
///test(CFI)

	for (unsigned short i=0; i<p1m.size(); i++){
	unsigned short j=p1m[i][0];
	unsigned short a=0;
	unsigned short aa=0;
	bool p1rel=false;
	cout << "no."<< j << endl;//test

// XYZ  (begin)
                //X
		if ( (j%80) ==1 ){
				a=j+79;}
		else{
				a=j-1;}

	for(unsigned k=0; k<p1m.size(); k++){
		if (a==p1m[k][0]) {
			aa=k;

			if( !rlrel(p1m[aa],p1m[i]) ){
						cout << "no." <<  j << " & no." << a << " / X no rel" << endl;  ///test
						p1rel=false;}
			else {
						cout << "no." <<  j << " & no." << a << " / X rel" << endl;  ///test
						p1rel=true;}
				break;}
						}
	if (p1rel==true){
		continue;}
					p1rel=false;

                //Y
		if(j>80){
                        a=j-80;}

	for(unsigned k=0; k<p1m.size(); k++){
		if (a==p1m[k][0]) {
			aa=k;
			if( !udrel(p1m[aa],p1m[i]) ){
						cout << "no." <<  j << " & no." << a << " / Y no rel" << endl;  ///test
						p1rel=false;}
			else {
						cout << "no." <<  j << " & no." << a << " / Y rel" << endl;  ///test
						p1rel=true;}
				break;}
						}
	if (p1rel==true){
		continue;}
					p1rel=false;

                //Z
		if(j>80){
			if( (j%80) ==1){
					a=j-1;}
			else	       {
                        		a=j-81;}
	for(unsigned k=0; k<p1m.size(); k++){
		if (a==p1m[k][0]) {
			aa=k;

			if( !mirel(p1m[aa],p1m[i]) ){
						cout << "no." <<  j << " & no." << a << " / Z no rel" << endl;  ///test
						p1rel=false;}
			else {
						cout << "no." <<  j << " & no." << a << " / Z rel" << endl;  ///test
						p1rel=true;}
				break;}
						}
	if (p1rel==true){
		continue;}
					p1rel=false;
						}

/// XYZ (End)

///Pattern2 value (begin)
	vector<unsigned short> p2v;
	if ( (j%80) ==79){
			p2v.push_back(j);
			p2v.push_back(j+1);
			p2v.push_back(j-78);
			p2v.push_back(j+80);
			p2v.push_back(j+81);
			p2v.push_back(j+2);
			p2v.push_back(j+160);
			p2v.push_back(j+161);
			p2v.push_back(j+82);
				}
	if ( (j%80) ==0){
			p2v.push_back(j);
			p2v.push_back(j-79);
			p2v.push_back(j-78);
			p2v.push_back(j+80);
			p2v.push_back(j+1);
			p2v.push_back(j+2);
			p2v.push_back(j+160);
			p2v.push_back(j+81);
			p2v.push_back(j+82);
				}

	if ( (j%80)!=0 && (j%80)!=79 ) {
			p2v.push_back(j);
			p2v.push_back(j+1);
			p2v.push_back(j+2);
			p2v.push_back(j+80);
			p2v.push_back(j+81);
			p2v.push_back(j+82);
			p2v.push_back(j+160);
			p2v.push_back(j+161);
			p2v.push_back(j+162);
				}

///Pattern2 value(End)

///make Pattern2(begin)
	for (unsigned short imp2=0; imp2<p2v.size(); imp2++){
		unsigned short p2v_i=p2v[imp2];
		for (unsigned short jmp2=0; jmp2<p1m.size(); jmp2++){
		unsigned short p1m_no=p1m[jmp2][0];
		/// if none of match number in p1m then pass this scan
			if (p2v_i!=p1m_no){
					if( jmp2==(p1m.size()-1) ) {
							op2.push_back(p0);}
					continue;}
		op2.push_back(p1m[jmp2]);

								break;}

								}
///make Pattern2(End)

///Pattern2 relation(Begin)
	vector< vector<unsigned short> >final_op2;
	vector<unsigned short> p2_state;
	///A
		final_op2.push_back(op2[0]);
		p2_state.push_back(1);
	///B
	if( rlrel(op2[0], op2[1]) ){
		final_op2.push_back(op2[1]);
		p2_state.push_back(1);}
	else{
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///C
	if( rlrel(op2[1], op2[2]) ){
		//test_c=3; //test(CFI)
		final_op2.push_back(op2[2]);
		p2_state.push_back(1);}
	else{
		//test_c=0; //test(CFI)
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///D
	if( udrel(op2[0],op2[3]) ){
		final_op2.push_back(op2[3]);
		p2_state.push_back(1);}
	else{
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///E
	if( mirel(op2[0],op2[4]) || udrel(op2[1],op2[4]) || rlrel(op2[3],op2[4]) ){
		final_op2.push_back(op2[4]);
		p2_state.push_back(1);}
	else{
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///F
	if( mirel(op2[1],op2[5]) || udrel(op2[2],op2[5]) || rlrel(op2[4],op2[5]) ){
		//test_f=6; //test(CFI)
		final_op2.push_back(op2[5]);
		p2_state.push_back(1);}
	else{
		//test_f=0; //test(CFI)
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///G
	if( udrel(op2[3],op2[6]) ){
		final_op2.push_back(op2[6]);
		p2_state.push_back(1);}
	else{
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///H
	if(  mirel(op2[3],op2[7]) || udrel(op2[4],op2[7]) || rlrel(op2[6],op2[7]) ){
		final_op2.push_back(op2[7]);
		p2_state.push_back(1);}
	else {
		final_op2.push_back(p0);
		p2_state.push_back(0);}
	///I
	if( mirel(op2[4],op2[8]) || udrel(op2[5],op2[8]) || rlrel(op2[7],op2[8]) ){
		//test_i=9; //test(CFI)
		final_op2.push_back(op2[8]);
		p2_state.push_back(1);}
	else {
		//test_i=0; //test(CFI)
		final_op2.push_back(p0);
		p2_state.push_back(0);}
/*for(vector<vector<unsigned short> >::size_type l=0;l<final_op2.size();l++)
{for(vector<unsigned short>::size_type m=0;m<final_op2[l].size();m++)
cout<<final_op2[l][m]<<endl;
cout <<"========"<<endl; }*/ //testp2					

///Pattern2 relation(End)

///Find center peak(begin)
	unsigned short fcpi=0;
	unsigned short fcpn=0;
	unsigned short fcpx=0;
	unsigned short fcpxs=0;
	unsigned short fcpy=11;
	unsigned short fcpys=0;
	fcpi=FindP1C(op2[FindCP1(p2_state)]);
		
	fcpn=op2[FindCP1(p2_state)][0];
		
		if (fcpi>=3){
			fcpxs=fcpi-3;}
			
		else {
			fcpxs=fcpi-1;}
	fcpx=((fcpn-1)%80)*2+fcpxs;
		if ((hp.name())=="circle hough plus"){
					int ix=fcpx;
					fcpx=abs(ix-159);}
		if(fcpi>=3){
			fcpys=1;}
	fcpy=fcpy+(fcpn/80)*2+fcpys;

	cout << "center of peak  x=" << fcpx << "  y=" << fcpy << endl; //test
	ofstream vhdlxOut("/home/ph202/p1p2/phii", fstream::app);//test
	//	vhdlxOut << "VHDL center of peak  x=" << fcpx << "  y=" << (fcpy-10) <<endl; //test
		vhdlxOut<< (2*fcpx+1)*1.125<<endl; //test
	ofstream vhdlyOut("/home/ph202/p1p2/ptt", fstream::app);
		vhdlyOut << 0.0045*pow(10,(1.780875+(fcpy-10)*0.08625)) <<endl;

                
		
//	ofstream vhdlyOut("/home/ph202/p1p2/vhdlinfory", fstream::app);//test
//		vhdlyOut << (fcpy-10)  << endl;//test
		
	p.push_back(fcpx);
	p.push_back(fcpy);
	peak_xy.push_back(p);

///Find center peak(end)
cout << "~~~~~~~~~~Pattern 2 & Find Peak End!!!~~~~~~~~~~" << endl;
	vector<unsigned short>().swap(p2_state);
	vector< vector<unsigned short> >().swap(final_op2);
	vector<unsigned short>().swap(p2v);
	vector< vector<unsigned short> >().swap(op2);
}
		//ofstream vhdlOut("/home/ph202/p1p2/vhdlinfor", fstream::app);//test
		//vhdlOut <<  "                  "  << endl;//test
		//vhdlOut.close();
//		ofstream vhdlxOut("/home/ph202/p1p2/vhdlinforx", fstream::app);//test
//		vhdlxOut << "   " << endl;//test
//		vhdlxOut.close();
//		ofstream vhdlyOut("/home/ph202/p1p2/vhdlinfory", fstream::app);//test
//		vhdlyOut << "  "  << endl;//test
//		vhdlyOut.close();
//... Pattren 2...end

cout << "total peaks="<< peak_xy.size() << endl; //test
if(peak_xy.size()>1)
{ofstream effiOut("/home/ph202/p1p2/effi", fstream::app);
		effiOut << "1"  << endl;}
/*else if(peak_xy.size()==0)
{ofstream effiOut("/home/ph202/p1p2/effi", fstream::app);
		effiOut << "0"  << endl;}*/

//////////////////////////////////////////////

        if ((hp.name())=="circle hough minus")
{
//ofstream MinusOut("/home/ph202/p1p2/minus_p2_CFI_12", fstream::app);
//MinusOut << test_c <<" " << test_f <<" "<< test_i << endl;
//ofstream MinOut("/home/ph202/p1p2/test10mv", fstream::app);
//MinOut << peak_xy.size() << endl;

//cout << peak_xy.size()  << "   " << test_c << "   "<< test_f << "   " << test_i << endl;//test for cfi
//MinOut.close();
}
        else
{
//ofstream PlusOut("/home/ph202/p1p2/test10v", fstream::app);
//PlusOut << test_c << " "<< test_f << " " << test_i << endl;
//ofstream PinOut("/home/ph202/p1p2/test10pv", fstream::app);
//PinOut << peak_xy.size() << endl;
//cout << peak_xy.size()  << "   " << test_c << "   "<< test_f << "   " << test_i << endl;
//PinOut.close();
}

//////////////////////////////////////////////

vector< vector<unsigned short> >().swap(p1m);
vector< vector<unsigned short> >().swap(peak_xy); //test

TRGDebug::leaveStage("p1p2");
}

////////////////////////////////////////////////

void
TRGCDCPeakFinder::doit(TCHPlaneMulti2 & hp,
                       const unsigned threshold,
                       const bool centerIsPeak,
		       vector<unsigned> & peakSerialIds) const {
    return peaks6(hp, threshold, centerIsPeak, peakSerialIds);
}

void
TRGCDCPeakFinder::peaks6(TCHPlaneMulti2 & hp,
                         const unsigned threshold,
                         const bool centerIsPeak,
			 vector<unsigned> & peakSerialIds) const {

    TRGDebug::enterStage("Peak Finding (peaks6)");
    if (TRGDebug::level())
        cout << TRGDebug::tab() << "threshold=" << threshold
             << ",plane name=[" << hp.name() << "]" << endl;

    //...Make ionnected regions (is this the best way???)...
    regions(hp, threshold);
///
p1p2(hp,threshold);
///
    //...Determine peaks...
    const vector<vector<unsigned> *> & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "region " << i << " contents" << endl;

        //...Calculate size and center of a region...
        const vector<unsigned> & r = * regions[i];
        unsigned minX = hp.nX();
        unsigned maxX = 0;
        unsigned minY = hp.nY();
        unsigned maxY = 0;
        for (unsigned j = 0; j < (unsigned) r.size(); j++) {
            const unsigned s = r[j];
            unsigned x = 0;
            unsigned y = 0;
            hp.id(s, x, y);
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab(4) << "x=" << x << ",y=" << y << endl;
        }
        const unsigned cX = minX + (maxX - minX) / 2;
        const unsigned cY = minY + (maxY - minY) / 2;

        //...Determine a center of a region...
        unsigned ncX = hp.nX() * hp.nY();
        unsigned ncY = ncX;
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "center of region:x=" << cX << ",y="
                 << cY << endl;
//ofstream vhdlxOut("/home/ph202/p1p2/vhdlinforx", fstream::app);//test
		//vhdlxOut << "Tsim center of peak  x=" << cX << "  y=" << (cY-10) <<endl; //test
//		vhdlxOut << 0.0045*pow(10,(1.780875+(cY-10)*0.08625)) <<endl;
	
        if (! centerIsPeak) {

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab() << "Searching a cell closest to the "
                     << "region center" << endl;

            //...Search for a cell which is the closest to the center...
            float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                unsigned x = 0;
                unsigned y = 0;
                hp.id(s, x, y);

                const float diff2 = (float(x) - float(cX)) *
                    (float(x) - float(cX))
                    + (float(y) - float(cY)) *
                    (float(y) - float(cY));

                if (diff2 < minDiff2) {
                    minDiff2 = diff2;
                    ncX = x;
                    ncY = y;
                }
                if (TRGDebug::level() > 1)
                    cout << TRGDebug::tab(4) << "x=" << ncX << ",y=" << ncY
                         << ":diff2=" << diff2 << endl;
            }
        }
        else {

            //...Search for a peak...
            float max = 0;
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                const float entry = hp.entry(s);
                if (max < entry) {
                    max = entry;
                    unsigned x = 0;
                    unsigned y = 0;
                    hp.id(s, x, y);
                    ncX = x;
                    ncY = y;
                }
            }
        }

	//...Store the center cell...
	const unsigned serialId = hp.serialId(ncX, ncY);
	peakSerialIds.push_back(serialId);

        if (TRGDebug::level()) {
            cout << TRGDebug::tab() << "region " << i << " final center:x="
                 << ncX << ",y=" << ncY << endl
                 << TRGDebug::tab(4) << "position in HP:x="
                 << hp.position(ncX, ncY).x() << ",y="
                 << hp.position(ncX, ncY).y() << endl;
	ofstream vhdlxOut("/home/ph202/p1p2/tsimpt", fstream::app);//test
		vhdlxOut << "Tsim center of peak  x=" << ncX << "  y=" << ncY <<endl; //test
		vhdlxOut << 0.0045*pow(10,(1.780875+(ncY-10)*0.08625)) <<endl;
	

        }
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << peakSerialIds.size() << " peak(s)"
             << " found in total" << endl;

//////////////////////////////////////////////

        if ((hp.name())=="circle hough minus")
{
//ofstream MinOut("/home/ph202/p1p2/minus_p1p2_test_CFI_07", fstream::app);
//MinOut <<"minus"<<  peakSerialIds.size() << endl;
//ofstream MinOut("/home/ph202/p1p2/test10m", fstream::app);
//MinOut <<  peakSerialIds.size() << endl;
//cout << "    "  <<  peakSerialIds.size() << endl;
//MinOut.close();
}
        else
{
//ofstream PinOut("/home/ph202/p1p2/plus_p1p2_test_CFI_07", fstream::app);
//PinOut <<"plus"<<  peakSerialIds.size() << endl;
//ofstream PinOut("/home/ph202/p1p2/test10p", fstream::app);
//PinOut <<  peakSerialIds.size() << endl;
//cout << "    "  <<  peakSerialIds.size() << endl;
//PinOut.close();
}

//////////////////////////////////////////////


    TRGDebug::leaveStage("Peak Finding (peaks6)");
    return;

}

} // namespace Belle2

