package b2daq.dqm.ui.event;

import java.awt.Cursor;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

import javax.swing.SwingUtilities;

import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.ui.CanvasPanel;


public class CanvasMouseListener implements MouseListener, MouseMotionListener {

	private Canvas _canvas = null;
	private CanvasPopupHandler _popup = new CanvasPopupHandler();
	private CanvasToolTipHandler _tip = new CanvasToolTipHandler();

	public CanvasMouseListener(Canvas c) {
		_canvas = c;
	}

	public void mouseClicked(MouseEvent arg0) {
		if ( SwingUtilities.isRightMouseButton(arg0) ) {
			_popup.setPopup(arg0);
		}
	}

	public void mouseEntered(MouseEvent arg0) {}

	public void mouseExited(MouseEvent arg0) {
		CanvasPanel panel = (CanvasPanel) arg0.getSource();
		_canvas.getAxisY().mouseReleased();
		_canvas.getAxisX().mouseReleased();
		if ( _canvas.getAxisY2() != null ) {
			_canvas.getAxisY2().mouseReleased();
		}
		panel.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
		panel.repaint();
	}

	public void mousePressed(MouseEvent arg0) {
		if (SwingUtilities.isLeftMouseButton(arg0)) {
			CanvasPanel panel = (CanvasPanel) arg0.getSource();
			double x = arg0.getX() / (double) ((CanvasPanel) arg0.getSource()).getWidth();
			double y = arg0.getY() / (double) ((CanvasPanel) arg0.getSource()).getHeight();
			double x1 = _canvas.getPad().getX();
			double y1 = _canvas.getPad().getY();
			double x2 = _canvas.getPad().getX() + _canvas.getPad().getWidth();
			double y2 = _canvas.getPad().getY() + _canvas.getPad().getHeight();
			if (_canvas.usePad()) {
				if (_canvas.getAxisY().hit(x, y)) {
					_canvas.getAxisY().mousePressed(x1, y, x2, y);
				} else if (_canvas.getAxisY2() != null
						&& _canvas.getAxisY2().hit(x, y)) {
					_canvas.getAxisY2().mousePressed(x1, y, x2, y);
				} else if (_canvas.getAxisX().hit(x, y)) {
					_canvas.getAxisX().mousePressed(x, y1, x, y2);
				}
			}
			if (_canvas.getColorAxis() != null && _canvas.getColorAxis().hit(x, y)) {
				_canvas.getColorAxis().setFocused(true);
			}
			panel.repaint();
			_tip.show(arg0);
		}
	}

	public void mouseReleased(MouseEvent arg0) {
		CanvasPanel panel = (CanvasPanel) arg0.getSource();
		_canvas.getAxisY().mouseReleased();
		_canvas.getAxisX().mouseReleased();
		if (_canvas.getAxisY2() != null)
			_canvas.getAxisY2().mouseReleased();
		panel.repaint();
	}

	public void mouseDragged(MouseEvent arg0) {
		CanvasPanel panel = (CanvasPanel) arg0.getSource();
		double x = arg0.getX()
				/ (double) ((CanvasPanel) arg0.getSource()).getWidth();
		double y = arg0.getY()
				/ (double) ((CanvasPanel) arg0.getSource()).getHeight();
		double x1 = _canvas.getPad().getX();
		double y1 = _canvas.getPad().getY();
		double x2 = _canvas.getPad().getX() + _canvas.getPad().getWidth();
		double y2 = _canvas.getPad().getY() + _canvas.getPad().getHeight();
		if (_canvas.usePad()) {
			if (_canvas.getAxisY().hit(x, y)) {
				_canvas.getAxisY().mouseDragged(x1, y, x2, y);
			} else if (_canvas.getAxisY2() != null
					&& _canvas.getAxisY2().hit(x, y)) {
				_canvas.getAxisY2().mouseDragged(x1, y, x2, y);
			} else if (_canvas.getAxisX().hit(x, y)) {
				_canvas.getAxisX().mouseDragged(x, y1, x, y2);
			}
		}
		if (_canvas.getColorAxis() != null && _canvas.getColorAxis().hit(x, y)) {
			_canvas.getColorAxis().setFocused(true);
		}
		panel.repaint();
		_tip.hide();
	}

	public void mouseMoved(MouseEvent arg0) {
		CanvasPanel panel = (CanvasPanel) arg0.getSource();
		double x = arg0.getX()
				/ (double) ((CanvasPanel) arg0.getSource()).getWidth();
		double y = arg0.getY()
				/ (double) ((CanvasPanel) arg0.getSource()).getHeight();
		if ((_canvas.usePad() && (_canvas.getAxisY().hit(x, y)
				|| _canvas.getAxisX().hit(x, y) || (_canvas.getAxisY2() != null && _canvas
				.getAxisY2().hit(x, y))))
				|| (_canvas.getColorAxis() != null && _canvas.getColorAxis()
						.hit(x, y))) {
			panel.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
		} else {
			panel.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
		}
		_tip.hide();
	}

}
