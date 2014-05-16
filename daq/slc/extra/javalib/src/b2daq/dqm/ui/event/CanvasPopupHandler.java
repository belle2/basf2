package b2daq.dqm.ui.event;

import java.awt.event.MouseEvent;

import javax.swing.JMenu;
import javax.swing.JPopupMenu;

import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.GMonObject;
import b2daq.dqm.ui.AxisPopupMenu;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.dqm.ui.CanvasPopupMenu;
import b2daq.dqm.ui.HistoMenu;
import b2daq.dqm.ui.LegendPopupMenu;
import b2daq.dqm.ui.PadPopupMenu;


public class CanvasPopupHandler {

	public boolean setPopup(MouseEvent arg0) {
		CanvasPanel panel = (CanvasPanel) arg0.getSource();
		double x = arg0.getX() / (double) panel.getWidth();
		double y = arg0.getY() / (double) panel.getHeight();
		Canvas _canvas = panel.getCanvas();
		String name = "";
		JPopupMenu popup_menu = new JPopupMenu();
		if (_canvas.getLegend() != null && _canvas.getLegend().hit(x, y)) {
			name = "legend";
			popup_menu.add(new LegendPopupMenu(_canvas.getLegend()));
			popup_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
		}
		if (_canvas.getPad().hit(x, y) && _canvas.usePad()) {
			name = "pad";
			popup_menu.add(new PadPopupMenu("pad", _canvas.getPad(), panel));
			JMenu menu = new JMenu("histo");
			for (GMonObject obj : panel.getCanvas().getMonObjects()) {
				try {
					if (obj != null) menu.add(new HistoMenu((GHisto)obj, panel));
				} catch (Exception e) {}
			}
			popup_menu.add(menu);
			popup_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
		}
		GAxis axis = null;
		if (_canvas.usePad() && _canvas.getAxisY().hit(x, y)) {
			name = "y-axis";
			axis = _canvas.getAxisY();
		} else if (_canvas.usePad() && _canvas.getAxisX().hit(x, y)) {
			name = "x-axis";
			axis = _canvas.getAxisX();
		} else if (_canvas.usePad() && _canvas.getAxisY2() != null
				&& _canvas.getAxisY2().hit(x, y)) {
			name = "y2-axis";
			axis = _canvas.getAxisY2();
		} else if (_canvas.getColorAxis() != null
				&& _canvas.getColorAxis().hit(x, y)) {
			name = "color-axis";
			axis = _canvas.getColorAxis();
		}
		if (axis != null) {
			popup_menu = new AxisPopupMenu(name, axis, panel);
			popup_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
			return true;
		}
		if (name.length() == 0) {
			popup_menu = new CanvasPopupMenu("canvas " + _canvas.getName(), _canvas, panel);
			popup_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
			return true;
		}
		return false;		
	}
	
}
