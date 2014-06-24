package b2daq.dqm.graphics;

import b2daq.dqm.core.Histo2;
import b2daq.graphics.GraphicsDrawer;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;

public class GHisto2 extends GHisto {

    public GHisto2() {
        super();
    }

    public GHisto2(HistogramCanvas canvas, Histo2 histo) {
        super(canvas, histo);
    }

    @Override
    public void update() {
        super.update();
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        //if (canvas == null) return;
        update();
        axisY = this.canvas.getColorAxis();
        ObservableList<Paint> color_pattern = ((GColorAxis) this.canvas.getColorAxis()).getColorPattern();
        if (color_pattern.size() == 0) {
                ((GColorAxis)this.canvas.getColorAxis()).setColorPattern(GColorAxis.COLOR_LIST);
                color_pattern = ((GColorAxis) this.canvas.getColorAxis()).getColorPattern();
        }
        double x0 = this.canvas.getPad().getX();
        double y0 = this.canvas.getPad().getY() + this.canvas.getPad().getHeight();
        double xmin = histo.getAxisX().getMin(), xmax = histo.getAxisX().getMax();
        double dbinx = (xmax - xmin) / histo.getAxisX().getNbins();
        double ymin = histo.getAxisY().getMin(), ymax = histo.getAxisY().getMax();
        double dbiny = (ymax - ymin) / histo.getAxisY().getNbins();
        double max = axisY.get().getMax();
        double min = axisY.get().getMin();
        double X, Y, width, height;
        for (int ny = 0; ny < histo.getAxisY().getNbins(); ny++) {
            for (int nx = 0; nx < histo.getAxisX().getNbins(); nx++) {
                X = x0 + this.canvas.getAxisX().eval(dbinx * nx + xmin);
                Y = y0 - this.canvas.getAxisY().eval(dbiny * ny + ymin);
                width = x0 + this.canvas.getAxisX().eval(dbinx * (nx + 1) + xmin) - X;
                height = Y - (y0 - this.canvas.getAxisY().eval(dbiny * (ny + 1) + ymin));
                Paint color = color_pattern.get(0);
                double data = histo.getBinContent(nx, ny);
                if (data < max && data > min) {
                    color = color_pattern.get((int) (color_pattern.size()
                            * ((data - min) / (max - min))));
                }
                if (data >= max) {
                    color = color_pattern.get(color_pattern.size() - 1);
                }
                if (data == 0 && min >= 0) {
                    color = Color.WHITE;
                }
                if (color != null) {
                    Paint c = canvas.getLine().getColor();
                    canvas.getFill().setColor(color);
                    canvas.getLine().setColor(null);
                    canvas.drawRect(X, Y - height * 1.2, width * 1.2, height * 1.2);
                    canvas.getLine().setColor(c);
                }
                if (histo.getLineColor() != null) {
                    Paint c = canvas.getFill().getColor();
                    canvas.getFill().setColor(null);
                    canvas.getLine().setColor(histo.getLineColor());
                    canvas.getLine().setWidth(histo.getLineWidth());
                    canvas.drawRect(X, Y - height * 1.2, width * 1.2, height * 1.2);
                    canvas.getFill().setColor(c);
                }
            }
        }
    }

}
