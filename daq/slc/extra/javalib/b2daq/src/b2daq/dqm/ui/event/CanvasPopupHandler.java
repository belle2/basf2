package b2daq.dqm.ui.event;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.ui.CanvasPanel;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.Menu;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;

public class CanvasPopupHandler {

    public boolean setPopup(MouseEvent arg0) {
        CanvasPanel panel = (CanvasPanel) arg0.getSource();
        double x = arg0.getX() / (double) panel.getWidth();
        double y = arg0.getY() / (double) panel.getHeight();
        HistogramCanvas _canvas = panel.getCanvas();
        String name = "";
        ContextMenu popup_menu = new ContextMenu();
        if (_canvas.getLegend() != null && _canvas.getLegend().hit(x, y)) {
            name = "legend";
            popup_menu.getItems().add(new LegendPopupMenu(_canvas.getLegend()));
            popup_menu.show((Pane) arg0.getSource(), arg0.getScreenX(), arg0.getScreenY());
        }
        if (_canvas.getPad().hit(x, y) && _canvas.usePad()) {
            name = "pad";
            popup_menu.getItems().add(new PadPopupMenu("pad", _canvas.getPad(), panel));
            Menu menu = new Menu("histo");
            for (Histo obj : panel.getCanvas().getHistograms()) {
                try {
                    if (obj != null) {
                        menu.getItems().add(new HistoMenu(obj, panel));
                    }
                } catch (Exception e) {
                }
            }
            popup_menu.getItems().add(menu);
            popup_menu.show((Pane) arg0.getSource(), arg0.getScreenX(), arg0.getScreenY());
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
            popup_menu.show((Pane) arg0.getSource(), arg0.getScreenX(), arg0.getScreenY());
            return true;
        }
        if (name.length() == 0) {
            popup_menu = new CanvasPopupMenu("canvas " + _canvas.getName(), _canvas, panel);
            popup_menu.show((Pane) arg0.getSource(), arg0.getScreenX(), arg0.getScreenY());
            return true;
        }
        return false;
    }

}
