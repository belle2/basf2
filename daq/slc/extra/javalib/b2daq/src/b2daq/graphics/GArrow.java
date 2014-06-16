package b2daq.graphics;

public class GArrow extends GShape {

   static enum Direction {
        RIGHT, LEFT, UP, DOWN
    };
    
    private double[] xpoints = new double[8];
    private double[] ypoints = new double[8];
    private double x = 100. / 400.;
    private double y = 100. / 450.;
    private double _length = 10. / 450.;
    private double _headLength = 10. / 450.;
    private double _headWidth = 40. / 400.;
    private double _width = 40. / 400.;
    private Direction direction = Direction.RIGHT;

    public GArrow() {

    }

    public GArrow(Direction direction) {
        this.direction = direction;
    }

    public GArrow(Direction direction, double x, double y) {
        this.x = x;
        this.y = y;
        this.direction = direction;
    }

    public void setPosition(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public void setX(double x) {
        this.x = x;
    }

    public void setY(double y) {
        this.y = y;
    }

    public double  getX() {
        return x;
    }

    public double getY() {
        return y;
    }

    public void setDirection(Direction direction) {
        this.direction = direction;
    }

    public void setLength(double length) {
        _length = length;
    }

    public void setHeadLength(double headLength) {
        _headLength = headLength;
    }

    public void setHeadWidth(double headWidth) {
        _headWidth = headWidth;
    }

    public void setWidth(double width) {
        _width = width;
    }

    public Direction getDirection() {
        return direction;
    }

    public double getLength() {
        return _length;
    }

    public double getHeadLength() {
        return _headLength;
    }

    public double getHeadWidth() {
        return _headWidth;
    }

    public double getWidth() {
        return _width;
    }

    public void draw(GraphicsDrawer canvas) {
        if (!isVisible()) return;
        if (direction == Direction.RIGHT) {
            xpoints[0] = x;
            xpoints[1] = x;
            xpoints[2] = _length + x;
            xpoints[3] = _length + x;
            xpoints[4] = _length + _headLength + x;
            xpoints[5] = _length + x;
            xpoints[6] = _length + x;
            xpoints[7] = x;
            ypoints[0] = y;
            ypoints[1] = _width + y;
            ypoints[2] = _width + y;
            ypoints[3] = _width + _headWidth + y;
            ypoints[4] = _width * 0.5 + y;
            ypoints[5] = y - _headWidth;
            ypoints[6] = y;
            ypoints[7] = y;
        } else if (direction == Direction.LEFT) {
            xpoints[0] = x;
            xpoints[1] = x;
            xpoints[2] = -_length + x;
            xpoints[3] = -_length + x;
            xpoints[4] = -_length - _headLength + x;
            xpoints[5] = -_length + x;
            xpoints[6] = -_length + x;
            xpoints[7] = x;
            ypoints[0] = y;
            ypoints[1] = _width + y;
            ypoints[2] = _width + y;
            ypoints[3] = (_width + _headWidth) * 0.5 + y;
            ypoints[4] = _width * 0.5 + y;
            ypoints[5] = (_width - _headWidth) * 0.5 + y;
            ypoints[6] = y;
            ypoints[7] = y;
        } else if (direction == Direction.UP) {
            xpoints[0] = x;
            xpoints[1] = _width + x;
            xpoints[2] = _width + x;
            xpoints[3] = (_width + _headWidth) * 0.5 + x;
            xpoints[4] = _width * 0.5 + x;
            xpoints[5] = (_width - _headWidth) * 0.5 + x;
            xpoints[6] = x;
            xpoints[7] = x;
            ypoints[0] = y;
            ypoints[1] = y;
            ypoints[2] = -_length + y;
            ypoints[3] = -_length + y;
            ypoints[4] = -_length - _headLength + y;
            ypoints[5] = -_length + y;
            ypoints[6] = -_length + y;
            ypoints[7] = y;
        } else if (direction == Direction.DOWN) {
            xpoints[0] = x;
            xpoints[1] = _width + x;
            xpoints[2] = _width + x;
            xpoints[3] = (_width + _headWidth) * 0.5 + x;
            xpoints[4] = _width * 0.5 + x;
            xpoints[5] = (_width - _headWidth) * 0.5 + x;
            xpoints[6] = x;
            xpoints[7] = x;
            ypoints[0] = y;
            ypoints[1] = y;
            ypoints[2] = _length + y;
            ypoints[3] = _length + y;
            ypoints[4] = _length + _headLength + y;
            ypoints[5] = _length + y;
            ypoints[6] = _length + y;
            ypoints[7] = y;
        }
        canvas.setFill(getFill());
        canvas.setLine(getLine());
        canvas.drawPolygon(xpoints, ypoints);
    }

}
