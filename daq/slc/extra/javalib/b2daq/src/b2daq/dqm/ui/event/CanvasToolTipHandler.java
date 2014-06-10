package b2daq.dqm.ui.event;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.GMonObject;
import b2daq.dqm.ui.CanvasPanel;
import javafx.scene.control.Tooltip;
import javafx.scene.input.MouseEvent;

public class CanvasToolTipHandler {

    private Tooltip _tip = new Tooltip();
    private double _x = -1;
    private double _y = -1;

    public void show(MouseEvent arg0) {
        CanvasPanel panel = (CanvasPanel) arg0.getSource();
        double x = arg0.getX() / (double) panel.getWidth();
        double y = arg0.getY() / (double) panel.getHeight();
        HistogramCanvas _canvas = panel.getCanvas();
        String text = "";
        if (_canvas.getLegend() != null && _canvas.getLegend().hit(x, y)) {
            text = "legend";
            _canvas.getLegend();
        }
        if (_canvas.usePad() && _canvas.getPad().hit(x, y)) {
            text = "pad";
        }
        for (Histo h : panel.getCanvas().getHistograms()) {
            try {
                if (h != null) {
                    text = h.getMessage(x, y);
                    if (text.length() > 0) {
                        if (_x != x || _y != y) {
                            _x = x;
                            _y = y;
                        } else {
                            if (h.getSelectedChannelRef() != null) {
                                /*
                                JFrame frame = new JFrame(h.getChannelRefSelected().getCanvas().getTitle());
                                frame.setBounds(arg0.getX(), arg0.getY(), 400, 400);
                                frame.add(new CanvasPanel(h.getChannelRefSelected().getCanvas(), 400, 400));
                                new FrameUpdater(frame).start();
                                        */
                                _x = -1;
                                _y = -1;
                            }
                        }
                    }
                }
            } catch (Exception e) {
            }
        }
        GAxis axis = null;
        if (_canvas.usePad() && _canvas.getAxisY().hit(x, y)) {
            text = "y-axis";
            axis = _canvas.getAxisY();
        } else if (_canvas.usePad() && _canvas.getAxisX().hit(x, y)) {
            text = "x-axis";
            axis = _canvas.getAxisX();
        } else if (_canvas.usePad() && _canvas.getAxisY2() != null
                && _canvas.getAxisY2().hit(x, y)) {
            text = "y2-axis";
            axis = _canvas.getAxisY2();
        } else if (_canvas.getColorAxis() != null
                && _canvas.getColorAxis().hit(x, y)) {
            text = "color-axis";
            axis = _canvas.getColorAxis();
        }
        if (axis != null) {
        }
        if (text.length() == 0) {
            text = "canvas";
        }

        _tip.setText(text);
    }

    public void hide() {
    }

    private class FrameUpdater extends Thread {
/*
        JFrame _frame = null;

        public FrameUpdater(JFrame frame) {
            _frame = frame;
        }

        public void run() {
            _frame.setVisible(true);
            while (_frame.isVisible()) {
                _frame.repaint();
                try {
                    sleep(1000);
                } catch (InterruptedException e) {
                    _frame.dispose();
                }
            }
        }
*/
    }

}
