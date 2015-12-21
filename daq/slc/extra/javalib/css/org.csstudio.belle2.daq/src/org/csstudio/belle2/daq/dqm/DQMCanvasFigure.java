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

}
