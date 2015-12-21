package org.csstudio.belle2.daq.dqm.event;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.dqm.graphics.GAxis;
import org.belle2.daq.dqm.graphics.HistogramCanvas;
import org.csstudio.belle2.daq.dqm.DQMCanvasFigure;
import org.csstudio.belle2.daq.dqm.DQMCanvasModel;
import org.eclipse.draw2d.Label;
import org.eclipse.draw2d.MouseEvent;

public class CanvasToolTipHandler extends Label {

	private double _x = -1;
	private double _y = -1;
	private DQMCanvasModel model;
	
	public CanvasToolTipHandler(DQMCanvasModel dqmmodel) {
		model = dqmmodel;
	}
	
	public void update(MouseEvent arg0) {
		DQMCanvasFigure panel = (DQMCanvasFigure) arg0.getSource();
		double x = (arg0.x - panel.getClientArea().x) / (double) panel.getClientArea().width;
		double y = (arg0.y - panel.getClientArea().y) / (double) panel.getClientArea().height;
		HistogramCanvas hcanvas = panel.getHistoCanvas();
		String text = "";
		if (hcanvas.getLegend() != null && hcanvas.getLegend().hit(x, y)) {
			//text = "legend";
			hcanvas.getLegend();
		}
		if (hcanvas.usePad() && hcanvas.getPad().hit(x, y) ) {
			//text = "pad";
		}
		for (Histo h : hcanvas.getHistograms()) {
			try {
				if (h != null) {
					text = h.getMessage(x, y);
					if ( text.length() > 0 ) {
						if (_x != x || _y != y ) {
							_x = x; _y = y;
						}
					}
				}
			} catch (Exception e) {}
		}
		GAxis axis = null;
		if (hcanvas.usePad() && hcanvas.getAxisY().hit(x, y)) {
			//text = "y-axis";
			axis = hcanvas.getAxisY();
		} else if (hcanvas.usePad() && hcanvas.getAxisX().hit(x, y)) {
			//text = "x-axis";
			axis = hcanvas.getAxisX();
		} else if (hcanvas.usePad() && hcanvas.getAxisY2() != null
				&& hcanvas.getAxisY2().hit(x, y)) {
			//text = "y2-axis";
			axis = hcanvas.getAxisY2();
		} else if (hcanvas.getColorAxis() != null
				&& hcanvas.getColorAxis().hit(x, y)) {
			//text = "color-axis";
			axis = hcanvas.getColorAxis();
		}
		if ( axis != null ) {
		}
		if ( text.length() == 0 ) {
			//text = "canvas";
		}
		model.setPropertyValue("tooltip", text);
	}
		
}
