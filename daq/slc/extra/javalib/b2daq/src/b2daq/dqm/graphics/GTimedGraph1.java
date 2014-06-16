package b2daq.dqm.graphics;

import b2daq.dqm.core.TimedGraph1;
import b2daq.graphics.GraphicsDrawer;

public class GTimedGraph1 extends GHisto {

    public GTimedGraph1() {
        super();
    }

    public GTimedGraph1(HistogramCanvas canvas, TimedGraph1 graph) {
        super(canvas, graph);
        pointX = new double[graph.getAxisX().getNbins() + 2];
        pointY = new double[graph.getAxisX().getNbins() + 2];
    }

    @Override
    public void update() {
        if (canvas == null) return;
        super.update();
        double x0 = canvas.getPad().getX() + canvas.getPad().getWidth();
        double y0 = canvas.getPad().getY() + canvas.getPad().getHeight();
        TimedGraph1 g = (TimedGraph1) histo;
        if (pointX.length != g.getAxisX().getNbins() + 2) {
            pointX = new double[g.getAxisX().getNbins() + 2];
            pointY = new double[g.getAxisX().getNbins() + 2];
        }
        double min = g.getPoint(0);
        double max = g.getPoint(0);
        for (int n = 0; n < g.getAxisX().getNbins(); n++) {
            if (max < g.getPoint(n)) {
                max = g.getPoint(n);
            }
            if (min > g.getPoint(n)) {
                min = g.getPoint(n);
            }
        }
        if (!g.getAxisY().isFixedMin()) {
            g.getAxisY().setMin((max + min) / 2. - (max - min) / 2. * 1.05);
        }
        if (!g.getAxisY().isFixedMax()) {
            g.getAxisY().setMax((max + min) / 2. + (max - min) / 2. * 1.05);
        }
        if (min == max && max == 0) {
            if (!g.getAxisY().isFixedMin()) {
                g.getAxisY().setMin(0);
            }
            if (!g.getAxisY().isFixedMax()) {
                g.getAxisY().setMax(1);
            }
        }
        double dt = (g.getAxisX().getMax() - canvas.getAxisX().getMax());
        double t0 = g.getUpdateTime() / 1000
                + Math.abs(canvas.getAxisX().getMin() - g.getAxisX().getMin())
                - ((dt>0)?dt:0);
        int nt = g.getIter();
        if (nt < 0) {
            nt = g.getAxisX().getNbins() - 1;
        }
        pointX[0] = x0 - canvas.getAxisX().eval(t0 - g.getTime(nt));
        pointY[0] = 1;
        int nttmp = nt;
        double xmin = 1;
        for (int n = 0; n < g.getAxisX().getNbins(); n++) {
            double t = t0 - g.getTime(nt);
            pointX[n + 1] = x0 - canvas.getAxisX().eval(t );
            if (pointX[n + 1] > 0 && xmin > pointX[n + 1]) {
                xmin = pointX[n + 1];
            }
            pointY[n + 1] = y0 - axisY.eval(g.getPoint(nt));
            nttmp = nt;
            nt--;
            if (nt < 0) {
                nt = g.getAxisX().getNbins() - 1;
            }
        }
        pointX[g.getAxisX().getNbins() + 1] = x0 - canvas.getAxisX().eval(t0 - g.getTime(nttmp));
        pointY[g.getAxisX().getNbins() + 1] = 1;
        for (int i = 0; i < pointX.length; i++) {
            if (pointX[i] < 0) {
                pointX[i] = xmin;
                pointY[i] = 1;
            }
        }
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        update();
        canvas.setLine(getObject().getLine());
        canvas.setFill(getObject().getFill());
        if (histo.getDraw().matches(".*L.*")) {
            canvas.drawPolyline(pointX, pointY);
        }
        if (histo.getDraw().matches(".*P.*")) {
            canvas.drawMarkers(1, pointX.length - 1, pointX, pointY);
        }
    }

}
