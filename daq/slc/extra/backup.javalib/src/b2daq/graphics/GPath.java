package b2daq.graphics;

import java.util.ArrayList;
import java.util.Iterator;

public class GPath extends GShape {

	private double _x = 0, _y = 0;
	private int _iter = 0;
	private ArrayList<Path> _path_v = new ArrayList<Path>();
	
	public GPath(HtmlColor line_color) {
		super(null, line_color);
		_iter = 0;
	}

	public GPath() {
		this(HtmlColor.BLACK);
	}

	public void moveTo(double x, double y) {
		_x = x; _y = y;
	}
	
	public void drawTo(double x, double y) {
		_iter++;
		if (_iter > _path_v.size())
			_path_v.add(new Path(_x, _y, x, y));
		else {
			Path path = _path_v.get(_path_v.size()-1);
			path._x0 = _x;
			path._y0 = _y;
			path._x1 = x;
			path._y1 = y;
			_x = x;	
			_y = y;	
		}
	}
	
	public void clear() {
		_iter = 0;
	}
	
	public void draw(GCanvas c) {
		c.store();
		c.setLine(getLine());
		for (Iterator<Path> it = _path_v.iterator(); it.hasNext(); ) {
			Path path = it.next();
			c.drawLine(path.getX0(), path.getY0(), path.getX1(), path.getY1());
		}
		c.restore();
	}

	private class Path {
		private double _x0 = 0, _y0 = 0;
		private double _x1 = 0, _y1 = 0;

		//public Path() {}
		public Path(double x0, double y0, double x1, double y1) {
			_x0 = x0;
			_y0 = y0;
			_x1 = x1;
			_y1 = y1;
		}
		public double getX0() { return _x0; }
		public double getY0() { return _y0; }
		public double getX1() { return _x1; }
		public double getY1() { return _y1; }
	}
}
