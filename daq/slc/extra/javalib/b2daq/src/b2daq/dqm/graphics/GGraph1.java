package b2daq.dqm.graphics;

import b2daq.dqm.core.Graph1;
import b2daq.graphics.GraphicsDrawer;

public class GGraph1 extends GHisto {

    public GGraph1() {
        super();
    }

    public GGraph1(HistogramCanvas canvas, Graph1 graph) {
        super(canvas, graph);
        pointX = new double[histo.getAxisX().getNbins()];
        pointY = new double[histo.getAxisX().getNbins()];
    }

    @Override
    public void update() {
        super.update();
        double x0 = canvas.getPad().getX();
        double y0 = canvas.getPad().getY() + canvas.getPad().getHeight();;

        if (histo.getAxisX().getNbins() > pointX.length) {
            pointX = new double[histo.getAxisX().getNbins()];
            pointY = new double[histo.getAxisX().getNbins()];
        }
        for (int n = 0; n < pointX.length && n < pointY.length; n++) {
            pointX[n] = x0 + canvas.getAxisX().eval(histo.getPointX(n));
            pointY[n] = y0 - canvas.getAxisY().eval(histo.getPointY(n));
        }
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        update();
        canvas.setLine(histo.getLine());
        canvas.setFill(histo.getFill());
        if (histo.getDraw().matches(".*P.*")) {
            canvas.drawMarkers(pointX, pointY);
        }
        if (histo.getDraw().matches(".*L.*")) {
            canvas.drawPolyline(pointX, pointY);
        }
    }

}
