package org.csstudio.belle2.daq.dqm;

import java.util.ArrayList;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.dqm.graphics.HistogramCanvas;
import org.belle2.daq.dqm.graphics.Legend;
import org.belle2.daq.graphics.HtmlColor;
import org.csstudio.belle2.daq.dqm.event.CanvasMouseListener;
import org.csstudio.belle2.daq.dqm.event.CanvasToolTipHandler;
import org.csstudio.opibuilder.editparts.AbstractWidgetEditPart;
import org.csstudio.opibuilder.properties.IWidgetPropertyChangeHandler;
import org.csstudio.opibuilder.util.OPIColor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.swt.graphics.RGB;

public class DQMCanvasEditPart extends AbstractWidgetEditPart {

	public DQMCanvasEditPart() {
	}

	@Override
	public DQMCanvasModel getWidgetModel() {
		return (DQMCanvasModel) super.getWidgetModel();
	}

	@Override
	protected IFigure doCreateFigure() {
		try {
			DQMViewCommunicator.startDQM();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		DQMCanvasFigure figure = new DQMCanvasFigure();
		figure.setCanvasFillColor(getWidgetModel().getCanvasFillColor());
		figure.setCanvasLineColor(getWidgetModel().getCanvasLineColor());
		figure.setPadFillColor(getWidgetModel().getPadFillColor());
		figure.setPadLineColor(getWidgetModel().getPadLineColor());
		getWidgetModel().setHistoPropertyVisible(getWidgetModel().getNHists());
		getWidgetModel().setLegendPropertyVisible(getWidgetModel().useLegend());
		initializeCanvas(figure);
		CanvasToolTipHandler handler = new CanvasToolTipHandler(getWidgetModel());
		CanvasMouseListener listener = new CanvasMouseListener(handler);
		figure.addMouseListener(listener);
		figure.addMouseMotionListener(listener);
		figure.setToolTip(handler);
		figure.setPadLogX(getWidgetModel().getPadLogX());
		figure.setPadLogY(getWidgetModel().getPadLogY());
		figure.setPadAvarageFactor(getWidgetModel().getPadAvarageFactor());
		figure.setPadMinY(getWidgetModel().getPadMinY());
		figure.setPadMaxY(getWidgetModel().getPadMaxY());
		figure.setPadMinZ(getWidgetModel().getPadMinZ());
		figure.setPadMaxZ(getWidgetModel().getPadMaxZ());

		return figure;
	}

	public void initializeCanvas(DQMCanvasFigure figure) {
		String title = getWidgetModel().getCanvasTitle();
		String xtitle = getWidgetModel().getCanvasXTitle();
		String ytitle = getWidgetModel().getCanvasYTitle();
		try {
			HistogramCanvas hcanvas = figure.getHistoCanvas();
			hcanvas.getHistograms().clear();
			for (int n = 0; n < getWidgetModel().getNHists(); n++) {
				String name = getWidgetModel().getHistName(n);
				if (name.length() == 0)
					name = "histo_" + n;
				String[] s = name.split("/");
				String hname = name;
				String packname = "";
				if (s.length > 1) {
					packname = s[0];
					hname = s[1];
				}
				Histo h = figure.replaceHisto(null, packname, hname);
				h.setName(name);
				String[] ss = getWidgetModel().getHistTitle(n).split(";");
				if (ss.length > 0 && ss[0].length() > 0) h.setTitle(ss[0]);
				if (ss.length > 1 && ss[1].length() > 0) h.getAxisX().setTitle(ss[1]);
				if (ss.length > 2 && ss[2].length() > 0) h.getAxisY().setTitle(ss[2]);
				if (n == 0) {
					if (title.length() == 0) title = h.getTitle();
					if (xtitle.length() == 0) xtitle = h.getAxisX().getTitle();
					if (ytitle.length() == 0) ytitle = h.getAxisY().getTitle();
				}
				RGB rgb = (getWidgetModel().getHistFillColor(n)).getRGBValue();
				h.setFillColor(getWidgetModel().getHistUseFillColor(n) ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				rgb = (getWidgetModel().getHistLineColor(n)).getRGBValue();
				h.setLineColor(getWidgetModel().getHistUseLineColor(n) ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				h.setLineWidth((float)getWidgetModel().getHistLineWidth(n));
				hcanvas.addHisto(h);
			}
			if (getWidgetModel().useLegend()) {
				Legend legend = new Legend();
				RGB rgb = getWidgetModel().getLegendFillColor().getRGBValue();
				legend.setFillColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
				rgb = getWidgetModel().getLegendLineColor().getRGBValue();
				legend.setLineColor(new HtmlColor(rgb.red, rgb.green, rgb.blue));
				hcanvas.setLegend(legend);
			} else {
				hcanvas.setLegend(null);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		figure.setCanvasTitle(title);
		figure.setCanvasXTitle(xtitle);
		figure.setCanvasYTitle(ytitle);
		figure.update();
	}

	@Override
	protected void registerPropertyChangeHandlers() {
		setPropertyChangeHandler(DQMCanvasModel.nhists, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				int nhists_old = (int) oldValue;
				int nhists = (int) newValue;
				try {
					DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
					if (nhists > nhists_old) {
						initializeCanvas(dqm);
					} else {
						HistogramCanvas hcanvas = dqm.getHistoCanvas();
						ArrayList<Histo> hists = hcanvas.getHistograms();
						while (nhists < hists.size()) {
							hists.remove(hists.size() - 1);
						}
					}
					getWidgetModel().setHistoPropertyVisible(nhists);
					dqm.update();
				} catch (Exception e) {
					e.printStackTrace();
				}
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.canvasTitle, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setCanvasTitle((String) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.canvasXTitle, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setCanvasXTitle((String) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.canvasYTitle, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setCanvasYTitle((String) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.canvasFillColor, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setCanvasFillColor((OPIColor) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padFillColor, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadFillColor((OPIColor) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padLogX, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.getHistoCanvas().getAxisX().setLogScale((Boolean)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padLogY, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.getHistoCanvas().getAxisY().setLogScale((Boolean)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padAvarageFactor, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadAvarageFactor((Double)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padMinY, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadMinY((Double)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padMaxY, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadMaxY((Double)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padMinZ, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadMinZ((Double)newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.padMaxZ, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setPadMaxZ((Double)newValue);
				dqm.update();
				return false;
			}
		});
		for (int n = 0; n < DQMCanvasModel.maxhists; n++) {
			setPropertyChangeHandler(DQMCanvasModel.histFillColor + n, new HistoFillColorPropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histLineColor + n, new HistoLineColorPropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histLineWidth + n, new HistoLineWidthPropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histUseFillColor + n, new HistoUseFillColorPropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histUseLineColor + n, new HistoUseLineColorPropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histName + n, new HistoNamePropertyChangeHandler(n));
			setPropertyChangeHandler(DQMCanvasModel.histTitle + n, new HistoTitlePropertyChangeHandler(n));
		}
		setPropertyChangeHandler(DQMCanvasModel.useLegend, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				((DQMCanvasModel) getWidgetModel()).setLegendPropertyVisible((Boolean) newValue);
				initializeCanvas((DQMCanvasFigure) figure);
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.legendFillColor, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setLegendFillColor((OPIColor) newValue);
				dqm.update();
				return false;
			}
		});
		setPropertyChangeHandler(DQMCanvasModel.legendLineColor, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.setLegendLineColor((OPIColor) newValue);
				dqm.update();
				return false;
			}
		});
	}

	private class HistoFillColorPropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoFillColorPropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				RGB rgb = ((OPIColor) newValue).getRGBValue();
				boolean use = getWidgetModel().getHistUseFillColor(m_id);
				dqm.getHistoCanvas().getHisto(m_id).setFillColor(use ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoUseFillColorPropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoUseFillColorPropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				boolean use = (boolean) newValue;
				RGB rgb = (getWidgetModel().getHistFillColor(m_id)).getRGBValue();
				use = getWidgetModel().getHistUseFillColor(m_id);
				dqm.getHistoCanvas().getHisto(m_id).setFillColor(use ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoLineColorPropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoLineColorPropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				RGB rgb = ((OPIColor) newValue).getRGBValue();
				boolean use = getWidgetModel().getHistUseFillColor(m_id);
				dqm.getHistoCanvas().getHisto(m_id).setLineColor(use ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoUseLineColorPropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoUseLineColorPropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				boolean use = (boolean) newValue;
				RGB rgb = (getWidgetModel().getHistLineColor(m_id)).getRGBValue();
				use = getWidgetModel().getHistUseLineColor(m_id);
				dqm.getHistoCanvas().getHisto(m_id).setLineColor(use ? new HtmlColor(rgb.red, rgb.green, rgb.blue) : null);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoLineWidthPropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoLineWidthPropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				double width = (double) newValue;
				dqm.getHistoCanvas().getHisto(m_id).setLineWidth((float)width);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoNamePropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoNamePropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.getHistoCanvas().getHisto(m_id).setName((String) newValue);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

	private class HistoTitlePropertyChangeHandler implements IWidgetPropertyChangeHandler {
		private int m_id;

		public HistoTitlePropertyChangeHandler(int id) {
			m_id = id;
		}

		public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
			if (newValue == null)
				return false;
			try {
				DQMCanvasFigure dqm = ((DQMCanvasFigure) figure);
				dqm.getHistoCanvas().getHisto(m_id).setName((String) newValue);
				dqm.update();
			} catch (Exception e) {
				// e.printStackTrace();
			}
			return false;
		}
	}

}