package b2daq.graphics;

import javafx.scene.paint.Color;

public class GPolygon extends GShape {

    private double[] _x, _y;

    public GPolygon() {
    }

    public GPolygon(double[] x, double[] y) {
        this(x, y, null, Color.BLACK);
    }

    public GPolygon(double[] x, double[] y, Color fill_color, Color line_color) {
        super(fill_color, line_color);
        set(x, y);
    }

    public void set(double[] x, double[] y) {
        _x = x;
        _y = y;
    }

    public void draw(GraphicsDrawer c) {
        if (!isVisible()) return;
        if (_x.length > 0 && _y.length > 0) {
            c.setLine(getLine());
            c.setFill(getFill());
            c.drawPolygon(_x, _y);
        }
    }

}
