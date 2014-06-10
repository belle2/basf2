package b2daq.graphics;

import javafx.scene.paint.Color;

public class GCircle extends GShape {

    private double _x, _y, _r_x, _r_y;

    public GCircle() {
        this(1, 1, 1, 1);
    }

    public GCircle(double x, double y, double r_x, double r_y) {
        this(x, y, r_x, r_y, null, Color.BLACK);
    }

    public GCircle(double x, double y, double r_x, double r_y,
            Color fill_color, Color line_color) {
        super(fill_color, line_color);
        set(x, y, r_x, r_y);
    }

    public void set(double x, double y, double r_x, double r_y) {
        _x = x;
        _y = y;
        _r_x = r_x;
        _r_y = r_y;
    }

    public void setPosition(double x, double y) {
        _x = x;
        _y = y;
    }

    public void setRX(double r_x) {
        _r_x = r_x;
    }

    public void setRY(double r_y) {
        _r_y = r_y;
    }

    public void setX(double x) {
        _x = x;
    }

    public void setY(double y) {
        _y = y;
    }

    public void draw(GraphicsDrawer c) {
        c.setLine(getLine());
        c.setFill(getFill());
        c.drawCircle(_x, _y, _r_x, _r_y);
    }

    @Override
    public boolean hit(double x, double y) {
        if (_r_x == 0 || _r_y == 0) {
            return false;
        }
        if ((x - _x) * (x - _x) / (_r_x * _r_x) + (y - _y) * (y - _y) / (_r_y * _r_y) < 1) {
            return true;
        }
        return false;
    }

}
