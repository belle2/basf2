package b2daq.graphics;

import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;

public class GRect extends GShape {

    protected double x, y, width, height;

    public GRect() {
        this(1, 1, 1, 1);
    }

    public GRect(double x, double y, double width, double height) {
        this(x, y, width, height, null, Color.BLACK);
    }

    public GRect(double x, double y, double width, double height,
            Paint fill_color, Paint line_color) {
        super(fill_color, line_color);
        set(x, y, width, height);
    }

    public void set(double x, double y, double width, double height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    public void setPosition(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public void setBounds(double width, double height) {
        this.width = width;
        this.height = height;
    }

    @Override
    public void draw(GraphicsDrawer c) {
        c.setLine(getLine());
        c.setFill(getFill());
        c.drawRect(x, y, width, height);
    }

    @Override
    public boolean hit(double x, double y) {
        return x >= this.x && x <= this.x + width && y >= this.y && y <= this.y + height;
    }

    public void setWidth(double w) {
        width = w;
    }

    public void setHeight(double h) {
        height = h;
    }

    public void setX(double x) {
        this.x = x;
    }

    public void setY(double y) {
        this.y = y;
    }

    public double getWidth() {
        return width;
    }

    public double getHeight() {
        return height;
    }

    public double getX() {
        return x;
    }

    public double getY() {
        return y;
    }

}
