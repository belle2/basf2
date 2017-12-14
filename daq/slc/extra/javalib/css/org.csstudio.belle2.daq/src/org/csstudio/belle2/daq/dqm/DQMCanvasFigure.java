package org.csstudio.belle2.daq.dqm;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.dqm.Histo1D;
import org.belle2.daq.dqm.graphics.HistogramCanvas;
import org.belle2.daq.graphics.HtmlColor;
import org.belle2.daq.graphics.JavaGraphicDrawer;
import org.csstudio.opibuilder.util.OPIColor;
import org.eclipse.draw2d.Figure;
import org.eclipse.draw2d.Graphics;
import org.eclipse.swt.graphics.RGB;

public class DQMCanvasFigure extends Figure {

	private JavaGraphicDrawer drawer = new JavaGraphicDrawer();
	private HistogramCanvas hcanvas = new HistogramCanvas();

	public DQMCanvasFigure() {
		super();
		drawer.add(hcanvas);
		DQMViewCommunicator.add(this);
	}

	@Override
	protected void paintClientArea(Graphics graphics) {
		super.paintClientArea(graphics);
		drawer.setX(getClientArea().x);
		drawer.setY(getClientArea().y);
		drawer.setBound(getClientArea().width, getClientArea().height);
		drawer.setGraphics(graphics);
		drawer.update();
	}

	public Histo replaceHisto(Histo h, String packname, String hname) {
		Histo hd = DQMViewCommunicator.getHisto(packname, hname);
		if (hd != null) {
			if (h != null) {
				h.reset();
				if (h.getDim() == 1 && hd.getDim() == 1) {
					h.getAxisX().copy(hd.getAxisX());
					h.setNbinsx(hd.getNbinsx());
				} else if (h.getDim() == 2 && hd.getDim() == 2) {
					h.getAxisX().copy(hd.getAxisX());
					h.getAxisY().copy(hd.getAxisY());
					h.getAxisZ().copy(hd.getAxisZ());
					h.setNbinsxy(hd.getNbinsx(), hd.getNbinsy());
				} else if (h.getDim() == 1 && hd.getDim() == 2) {
					Histo hnew = hd.clone();
					hnew.setName(h.getName());
					hcanvas.addHisto(hnew);
					h = hnew;
				}
			} else if (hd.getDim() == 1) {
				Histo hnew = hd.clone();
				hcanvas.addHisto(hnew);
				h = hnew;
			} else if (hd.getDim() == 2) {
				Histo hnew = hd.clone();
				hcanvas.addHisto(hnew);
				h = hnew;
			}
			h.add(hd);
		}
		if (h == null) {
			Histo hnew = new Histo1D("", "", 100, 0, 100);
			hcanvas.addHisto(hnew);
			h = hnew;
		}
		return h;
	}

	public void update() {
		for (Histo h : hcanvas.getHistograms()) {
			String hname = h.getName();
			String[] s = hname.split("/");
			String packname = "";
			if (s.length > 1) {
				packname = s[0];
				hname = s[1];
			}
			replaceHisto(h, packname, hname);
		}
		hcanvas.update();
		drawer.update();
		repaint();
	}

	public void setUseLegend(boolean useLegend) {
	}

	public void setNHists(int nhists) {

	}

	public void setCanvasTitle(String title) {
		hcanvas.setTitle(title);
	}

	public void setCanvasXTitle(String title) {
		hcanvas.getAxisX().setTitle(title);
	}

	public void setCanvasYTitle(String title) {
		hcanvas.getAxisY().setTitle(title);
	}

	public void setCanvasFillColor(OPIColor color) {
		RGB rgb = color.getRGBValue();
		hcanvas.setFillColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
	}

	public void setCanvasLineColor(OPIColor color) {
		RGB rgb = color.getRGBValue();
		hcanvas.setLineColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
	}

	public void setPadFillColor(OPIColor color) {
		RGB rgb = color.getRGBValue();
		hcanvas.getPad().setFillColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
	}

	public void setPadLineColor(OPIColor color) {
		RGB rgb = color.getRGBValue();
		hcanvas.getPad().setLineColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
	}

	public void setPadLogX(boolean logx) {
		hcanvas.getAxisX().setLogScale(logx);
	}

	public void setPadLogY(boolean logy) {
		hcanvas.getAxisY().setLogScale(logy);
	}

	public void setPadMinY(double min) {
		if (min > 0 && min < hcanvas.getAxisY().getMax()) {
			hcanvas.getAxisY().setFixedMin(false);
			hcanvas.getAxisY().setMin(min);
			hcanvas.getAxisY().setFixedMin(true);
		} else {
			hcanvas.getAxisY().setFixedMin(false);
		}
	}

	public void setPadMaxY(double max) {
		if (max > 0 && max > hcanvas.getAxisY().getMin()) {
			hcanvas.getAxisY().setFixedMax(false);
			hcanvas.getAxisY().setMax(max);
			hcanvas.getAxisY().setFixedMax(true);
		} else {
			hcanvas.getAxisY().setFixedMax(false);
		}
	}

	public void setPadMinZ(double min) {
		if (hcanvas.getColorAxis() == null) return;
		if (min > 0 && min < hcanvas.getColorAxis().getMax()) {
			hcanvas.getColorAxis().setFixedMin(false);
			hcanvas.getColorAxis().setMin(min);
			hcanvas.getColorAxis().setFixedMin(true);
		} else {
			hcanvas.getColorAxis().setFixedMin(false);
		}
	}
	
	public void setPadMaxZ(double max) {
		if (hcanvas.getColorAxis() == null) return;
		if (max > 0 && max > hcanvas.getColorAxis().getMin()) {
			hcanvas.getColorAxis().setFixedMax(false);
			hcanvas.getColorAxis().setMax(max);
			hcanvas.getColorAxis().setFixedMax(true);
		} else {
			hcanvas.getColorAxis().setFixedMax(false);
		}
	}


	public void setLegendFillColor(OPIColor color) {
		if (hcanvas.getLegend() != null) {
			RGB rgb = color.getRGBValue();
			hcanvas.getLegend().setFillColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
		}
	}

	public void setLegendLineColor(OPIColor color) {
		if (hcanvas.getLegend() != null) {
			RGB rgb = color.getRGBValue();
			hcanvas.getLegend().setLineColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
		}
	}

	public HistogramCanvas getHistoCanvas() {
		return hcanvas;
	}

	public void setPadAvarageFactor(double factor) {
		if (hcanvas.getColorAxis() != null) {
			hcanvas.getColorAxis().get().setAvarageFactor(factor);
		} else {
			hcanvas.getAxisY().get().setAvarageFactor(factor);
		}
	}

}
