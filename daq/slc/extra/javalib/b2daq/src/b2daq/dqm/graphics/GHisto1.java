package b2daq.dqm.graphics;

import b2daq.core.NumberString;
import b2daq.dqm.core.Histo;
import b2daq.dqm.core.Histo1;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GText;

public final class GHisto1 extends GHisto {

    private GText[] _text_v = null;

    public GHisto1() {
        super();
    }

    public GHisto1(HistogramCanvas canvas, Histo1 histo) {
        super(canvas, histo);
        setNbins();
    }

    public void setNbins() {
        int npoints = (this.histo.getAxisX().getNbins() + 1) * 2;
        if (npoints > pointX.length) {
            pointX = new double[npoints];
            pointY = new double[npoints];
            _text_v = new GText[this.histo.getAxisX().getNbins()];
            for (int n = 0; n < _text_v.length; n++) {
                _text_v[n] = new GText();
            }
        }
    }
    
    @Override
    public void update() {
        super.update();
        if (canvas == null) return;
        setNbins();
        double x0 = canvas.getPad().getX();
        double y0 = canvas.getPad().getY() + canvas.getPad().getHeight();
        double xmin = canvas.getAxisX().get().getMin();
        double ymin = axisY.get().getMin();
        if (axisY.isLogScale()) {
            ymin = Math.log(ymin);
        }
        double xscale = canvas.getAxisX().getScale();
        double dbinx = (histo.getAxisX().getMax() - histo.getAxisX().getMin())
                / histo.getAxisX().getNbins();

        double X = x0 + (histo.getAxisX().getMin() - xmin) * xscale;
        double X2 = 0;
        if (X < x0) {
            X = x0;
        }
        pointX[0] = X;
        pointY[0] = y0;
        double Y;
        int nin = 0;
        int n_digit = 1;
        if (histo.getDataType().matches(".*I")
                || histo.getDataType().matches(".*C") || histo.getDataType().matches(".*S")) {
            n_digit = 0;
        }
        for (int n = 0; n < histo.getAxisX().getNbins(); n++) {
            X = x0 + canvas.getAxisX().eval(dbinx * n + histo.getAxisX().getMin());
            Y = y0 - axisY.eval(histo.getBinContent(n));
            pointX[2 * nin + 1] = X;
            pointY[2 * nin + 1] = Y;
            X2 = x0 + canvas.getAxisX().eval(dbinx * (n + 1) + histo.getAxisX().getMin());
            pointX[2 * nin + 2] = X2;
            pointY[2 * nin + 2] = Y;
            _text_v[n].setFont(getObject().getFont());
            _text_v[n].setPosition((X + X2) / 2, Y);
            _text_v[n].setAlignment("center bottom");
            _text_v[n].setText(NumberString.toString(histo.getBinContent(n), n_digit));
            nin++;
        }
        pointX[2 * nin + 1] = X2;
        pointY[2 * nin + 1] = y0;
    }

    public String getMessage(double x, double y) {
        if (!canvas.getPad().hit(x, y)) {
            return "";
        }
        for (int n = 1; n <= histo.getAxisX().getNbins(); n++) {
            if (x < pointX[2 * n]) {
                if (y >= pointY[2 * n] && y < canvas.getPad().getY() + canvas.getPad().getHeight()) {
                    return "<html>"
                            + "histo = " + getObject().getName() + "<br />"
                            + "bin = " + n + " <br />"
                            + "value = " + NumberString.toString(((Histo) getObject()).getBinContent(n - 1), 2)
                            + "</html>";
                }
            }
        }
        return "";
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        update();
        canvas.setLine(getObject().getLine());
        canvas.setFill(getObject().getFill());
        canvas.drawPolygon(pointX, pointY);
        if (histo.getDraw().matches(".*T.*")) {
            for (GText text : _text_v) {
                text.draw(canvas);
            }
        }
    }

}
