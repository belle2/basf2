package b2dqm.graphics;

import b2daq.graphics.GArc;
import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;
import b2dqm.core.PieTable;

public class GPieTable extends GMonShape {

	private GArc [] _arc_v;
	private PieTable _pie;
	
	public GPieTable(Canvas canvas, PieTable pie) {
		super(canvas, pie);
		_pie = pie;
		_arc_v = new GArc [_pie.getNIndex()];
		for ( int n = 0; n < _arc_v.length; n++ ) {
			_arc_v[n] = new GArc(0.5, 0.5, 0.2, 0.2);
			if (n < HtmlColor.ColorGrad.length) {
				_arc_v[n].setFillColor(HtmlColor.ColorGrad[n]);
				_arc_v[n].setLineColor(HtmlColor.ColorGrad[n]);
			}
		}
	}

	public PieTable get() {
		return _pie;
	}
	
	public void draw(GCanvas c) {
		update();
		for ( GArc arc : _arc_v ) {
			arc.draw(c);
		}
	}

	public void update() {
		double sum = 0;
		for ( int n = 0; n < _pie.getNIndex(); n++ ) {
			sum += _pie.getData(n);
		}
		double angle0 = 0;
		double angle1 = 0;
		int n = 0;
		for ( GArc arc : _arc_v ) {
			angle1 = _pie.getData(n) / sum * 360;
			arc.setAngle(angle0, angle1);
			angle0 += angle1;
			n++;
		}
	}

}
