package org.csstudio.belle2.daq.dqm.event;

import org.belle2.daq.dqm.graphics.HistogramCanvas;
import org.csstudio.belle2.daq.dqm.DQMCanvasFigure;
import org.eclipse.draw2d.MouseEvent;
import org.eclipse.draw2d.MouseListener;
import org.eclipse.draw2d.MouseMotionListener;

public class CanvasMouseListener implements MouseListener, MouseMotionListener {

	private boolean dragged = false;
	private CanvasToolTipHandler handler;

	public CanvasMouseListener(CanvasToolTipHandler label) {
		handler = label;
	}
	
	@Override
	public void mouseDoubleClicked(MouseEvent arg0) {
		System.out.println("mouseDoubleClicked");
	}

	@Override
	public void mousePressed(MouseEvent arg0) {
		DQMCanvasFigure panel = (DQMCanvasFigure) arg0.getSource();
		HistogramCanvas hcanvas = panel.getHistoCanvas();
		if (arg0.button == 3) {

		} else if (arg0.button == 1) {
			if (!dragged) {
				double x = (arg0.x - panel.getClientArea().x) / (double) panel.getClientArea().width;
				double y = (arg0.y - panel.getClientArea().y) / (double) panel.getClientArea().height;
				double x1 = hcanvas.getPad().getX();
				double y1 = hcanvas.getPad().getY();
				double x2 = hcanvas.getPad().getX() + hcanvas.getPad().getWidth();
				double y2 = hcanvas.getPad().getY() + hcanvas.getPad().getHeight();
				if (hcanvas.usePad()) {
					if (hcanvas.getAxisY().hit(x, y)) {
						hcanvas.getAxisY().mousePressed(x1, y, x2, y);
						dragged = true;
					} else if (hcanvas.getAxisY2() != null && hcanvas.getAxisY2().hit(x, y)) {
						hcanvas.getAxisY2().mousePressed(x1, y, x2, y);
						dragged = true;
					} else if (hcanvas.getAxisX().hit(x, y)) {
						hcanvas.getAxisX().mousePressed(x, y1, x, y2);
						dragged = true;
					}
				}
				if (hcanvas.getColorAxis() != null && hcanvas.getColorAxis().hit(x, y)) {
					hcanvas.getColorAxis().setFocused(true);
				}
				panel.repaint();
			} else {
				hcanvas.getAxisY().mouseReleased();
				hcanvas.getAxisX().mouseReleased();
				if (hcanvas.getAxisY2() != null) {
					hcanvas.getAxisY2().mouseReleased();
				}
				panel.repaint();
				dragged = false;
			}
		}
	}

	@Override
	public void mouseReleased(MouseEvent arg0) {
	}


	@Override
	public void mouseDragged(MouseEvent arg0) {
	}

	@Override
	public void mouseEntered(MouseEvent arg0) {
	}

	@Override
	public void mouseExited(MouseEvent arg0) {
		DQMCanvasFigure panel = (DQMCanvasFigure) arg0.getSource();
		HistogramCanvas hcanvas = panel.getHistoCanvas();
		hcanvas.getAxisY().mouseReleased();
		hcanvas.getAxisX().mouseReleased();
		if (hcanvas.getAxisY2() != null) {
			hcanvas.getAxisY2().mouseReleased();
		}
		panel.repaint();
		dragged = false;
	}

	@Override
	public void mouseHover(MouseEvent arg0) {
		handler.update(arg0);
	}

	@Override
	public void mouseMoved(MouseEvent arg0) {
		handler.update(arg0);
		if (!dragged) {
			return;
		}
		DQMCanvasFigure panel = (DQMCanvasFigure) arg0.getSource();
		HistogramCanvas hcanvas = panel.getHistoCanvas();
		double x = (arg0.x - panel.getClientArea().x) / (double) panel.getClientArea().width;
		double y = (arg0.y - panel.getClientArea().y) / (double) panel.getClientArea().height;
		double x1 = hcanvas.getPad().getX();
		double y1 = hcanvas.getPad().getY();
		double x2 = hcanvas.getPad().getX() + hcanvas.getPad().getWidth();
		double y2 = hcanvas.getPad().getY() + hcanvas.getPad().getHeight();
		if (hcanvas.usePad()) {
			if (hcanvas.getAxisY().hit(x, y)) {
				hcanvas.getAxisY().mouseDragged(x1, y, x2, y);
			} else if (hcanvas.getAxisY2() != null && hcanvas.getAxisY2().hit(x, y)) {
				hcanvas.getAxisY2().mouseDragged(x1, y, x2, y);
			} else if (hcanvas.getAxisX().hit(x, y)) {
				hcanvas.getAxisX().mouseDragged(x, y1, x, y2);
			}
		}
		if (hcanvas.getColorAxis() != null && hcanvas.getColorAxis().hit(x, y)) {
			hcanvas.getColorAxis().setFocused(true);
		}
		panel.repaint();
	}


}
