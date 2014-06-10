package b2daq.graphics;

import javafx.scene.paint.Color;

public class GLine extends GShape {

    private double x1, y1, x2, y2;

    public GLine() {
        this(0, 0, 1, 1, Color.BLACK);
    }

    public GLine(double x1, double y1, double x2, double y2) {
        this(x1, y1, x2, y2, Color.BLACK);
    }

    public GLine(Color linecolor) {
        this(0, 0, 1, 1, linecolor);
    }

    public GLine(double x1, double y1, double x2, double y2, Color linecolor) {
        super(null, linecolor);
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    public void set(double x1, double y1, double x2, double y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    public void draw(GraphicsDrawer c) {
        c.setLine(getLine());
        c.drawLine(this.x1, this.y1, this.x2, this.y2);
    }

    public double getX1() {
        return this.x1;
    }

    public double getX2() {
        return this.x2;
    }

    public double getY1() {
        return this.y1;
    }

    public double getY2() {
        return this.y2;
    }

    public void setX1(double x1) {
        this.x1 = x1;
    }

    public void setX2(double x2) {
        this.x2 = x2;
    }

    public void setY1(double y1) {
        this.y1 = y1;
    }

    public void setY2(double y2) {
        this.y2 = y2;
    }

}
