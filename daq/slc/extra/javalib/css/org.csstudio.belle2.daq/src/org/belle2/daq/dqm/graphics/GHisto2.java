package org.belle2.daq.dqm.graphics;

import java.util.ArrayList;
import org.belle2.daq.base.NumberString;
import org.belle2.daq.dqm.Histo2;
import org.belle2.daq.graphics.GraphicsDrawer;
import org.belle2.daq.graphics.HtmlColor;

public class GHisto2 extends GHisto {

	public GHisto2() {
		super();
	}

	public GHisto2(HistogramCanvas canvas, Histo2 histo) {
		super(canvas, histo);
	}

	@Override
	public void update() {
		super.update();
	}

	@Override
	public void draw(GraphicsDrawer canvas) {
		// if (canvas == null) return;
		update();
		axisY = this.canvas.getColorAxis();
		ArrayList<HtmlColor> color_pattern = ((GColorAxis) this.canvas.getColorAxis()).getColorPattern();
		if (color_pattern.size() == 0) {
			((GColorAxis) this.canvas.getColorAxis()).setColorPattern(GColorAxis.COLOR_LIST);
			color_pattern = ((GColorAxis) this.canvas.getColorAxis()).getColorPattern();
		}
		double x0 = this.canvas.getPad().getX();
		double y0 = this.canvas.getPad().getY() + this.canvas.getPad().getHeight();
		double xmin = histo.getAxisX().getMin(), xmax = histo.getAxisX().getMax();
		double dbinx = (xmax - xmin) / histo.getAxisX().getNbins();
		double ymin = histo.getAxisY().getMin(), ymax = histo.getAxisY().getMax();
		double dbiny = (ymax - ymin) / histo.getAxisY().getNbins();
		double max = axisY.get().getMax();
		double min = axisY.get().getMin();
		double X, Y, width, height;
		HtmlColor cline = canvas.getLine().getColor();
		HtmlColor cfill = canvas.getFill().getColor();
		HtmlColor color = color_pattern.get(0);
		for (int ny = 0; ny < histo.getAxisY().getNbins(); ny++) {
			Y = y0 - this.canvas.getAxisY().eval(dbiny * ny + ymin);
			height = Y - (y0 - this.canvas.getAxisY().eval(dbiny * (ny + 1) + ymin));
			for (int nx = 0; nx < histo.getAxisX().getNbins(); nx++) {
				X = x0 + this.canvas.getAxisX().eval(dbinx * nx + xmin);
				width = x0 + this.canvas.getAxisX().eval(dbinx * (nx + 1) + xmin) - X;
				double data = histo.getBinContent(nx, ny);
				if (data < max && data > min) {
					color = color_pattern.get((int) (color_pattern.size() * ((data - min) / (max - min))));
				}
				if (data >= max) {
					color = color_pattern.get(color_pattern.size() - 1);
				}
				if (data == 0 && min >= 0) {
					color = HtmlColor.WHITE;
				}
				if (color != null) {
					canvas.getFill().setColor(color);
					canvas.getLine().setColor(null);
					canvas.drawRect(X, Y - height, width, height, 1);
					canvas.getFill().setColor(cfill);
					canvas.getLine().setColor(cline);
				}
				//if (histo.getLineColor() != null) {
				//	HtmlColor c = canvas.getFill().getColor();
				//	canvas.getFill().setColor(null);
				//	canvas.getLine().setColor(histo.getLineColor());
				//	canvas.getLine().setWidth(histo.getLineWidth());
				//	canvas.drawRect(X, Y - height, width, height, 1);
				//	canvas.getFill().setColor(c);
				//}
			}
		}
	}

	@Override
	public String getMessage(double x, double y) {
		if (!canvas.getPad().hit(x, y)) return "";
		double x0 = this.canvas.getPad().getX();
		double y0 = this.canvas.getPad().getY() + this.canvas.getPad().getHeight();
		double xmin = histo.getAxisX().getMin(), xmax = histo.getAxisX().getMax();
		double dbinx = (xmax - xmin) / histo.getAxisX().getNbins();
		double ymin = histo.getAxisY().getMin(), ymax = histo.getAxisY().getMax();
		double dbiny = (ymax - ymin) / histo.getAxisY().getNbins();
		double X, Y, width, height;
		for ( int nx = 0; nx < histo.getAxisX().getNbins(); nx++ ) {
			for ( int ny = 0; ny < histo.getAxisY().getNbins(); ny++ ) {
				X = x0 + this.canvas.getAxisX().eval(dbinx * nx + xmin);
				Y = y0 - this.canvas.getAxisY().eval(dbiny * ny + ymin);
				width = x0 + this.canvas.getAxisX().eval(dbinx * (nx + 1) + xmin) - X;
				height = Y - (y0 - this.canvas.getAxisY().eval(dbiny * (ny + 1) + ymin));
				if (x >= X && x <= X + width && y >= Y && y <= Y + height) {
        			return "" +
        				"histo = " + histo.getName() + "\n" +
        				"bin = " + (nx+1) +", " + (ny+1) + " \n"  +
        				"value = " + NumberString.toString(histo.getBinContent(nx, ny), 2) +  
        				"";
        		}
    		}
    	}
		return "";
	}

}
