package b2daq.graphics;

public class GArrow extends GShape {

    static final public int DIRECT_RIGHT = 0;
    static final public int DIRECT_LEFT = 1;
    static final public int DIRECT_UP = 2;
    static final public int DIRECT_DOWN = 3;

    private double[] _x = new double[8];
    private double[] _y = new double[8];
    private double _baseX = 100. / 400.;
    private double _baseY = 100. / 450.;
    private double _length = 10. / 450.;
    private double _headLength = 10. / 450.;
    private double _headWidth = 40. / 400.;
    private double _width = 20. / 400.;
    private double _direction = DIRECT_RIGHT;

    public GArrow() {

    }

    public GArrow(double direction) {
        _direction = direction;
    }

    public GArrow(double direction, double x, double y) {
        _baseX = x;
        _baseY = y;
        _direction = direction;
    }

    public void setPosition(double x, double y) {
        _baseX = x;
        _baseY = y;
    }

    public void setPositionX(double x) {
        _baseX = x;
    }

    public void setPositionY(double y) {
        _baseY = y;
    }

    public void setDirection(double direction) {
        _direction = direction;
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

    public void draw(GraphicsDrawer canvas) {
        if (_direction == DIRECT_RIGHT) {
            _x[0] = _baseX;
            _x[1] = _baseX;
            _x[2] = _length + _baseX;
            _x[3] = _length + _baseX;
            _x[4] = _length + _headLength + _baseX;
            _x[5] = _length + _baseX;
            _x[6] = _length + _baseX;
            _x[7] = _baseX;
            _y[0] = _baseY;
            _y[1] = _width + _baseY;
            _y[2] = _width + _baseY;
            _y[3] = (_width + _headWidth) * 0.5 + _baseY;
            _y[4] = _width * 0.5 + _baseY;
            _y[5] = (_width - _headWidth) * 0.5 + _baseY;
            _y[6] = _baseY;
            _y[7] = _baseY;
        } else if (_direction == DIRECT_LEFT) {
            _x[0] = _baseX;
            _x[1] = _baseX;
            _x[2] = -_length + _baseX;
            _x[3] = -_length + _baseX;
            _x[4] = -_length - _headLength + _baseX;
            _x[5] = -_length + _baseX;
            _x[6] = -_length + _baseX;
            _x[7] = _baseX;
            _y[0] = _baseY;
            _y[1] = _width + _baseY;
            _y[2] = _width + _baseY;
            _y[3] = (_width + _headWidth) * 0.5 + _baseY;
            _y[4] = _width * 0.5 + _baseY;
            _y[5] = (_width - _headWidth) * 0.5 + _baseY;
            _y[6] = _baseY;
            _y[7] = _baseY;
        } else if (_direction == DIRECT_UP) {
            _x[0] = _baseX;
            _x[1] = _width + _baseX;
            _x[2] = _width + _baseX;
            _x[3] = (_width + _headWidth) * 0.5 + _baseX;
            _x[4] = _width * 0.5 + _baseX;
            _x[5] = (_width - _headWidth) * 0.5 + _baseX;
            _x[6] = _baseX;
            _x[7] = _baseX;
            _y[0] = _baseY;
            _y[1] = _baseY;
            _y[2] = -_length + _baseY;
            _y[3] = -_length + _baseY;
            _y[4] = -_length - _headLength + _baseY;
            _y[5] = -_length + _baseY;
            _y[6] = -_length + _baseY;
            _y[7] = _baseY;
        } else if (_direction == DIRECT_DOWN) {
            _x[0] = _baseX;
            _x[1] = _width + _baseX;
            _x[2] = _width + _baseX;
            _x[3] = (_width + _headWidth) * 0.5 + _baseX;
            _x[4] = _width * 0.5 + _baseX;
            _x[5] = (_width - _headWidth) * 0.5 + _baseX;
            _x[6] = _baseX;
            _x[7] = _baseX;
            _y[0] = _baseY;
            _y[1] = _baseY;
            _y[2] = _length + _baseY;
            _y[3] = _length + _baseY;
            _y[4] = _length + _headLength + _baseY;
            _y[5] = _length + _baseY;
            _y[6] = _length + _baseY;
            _y[7] = _baseY;
        }
        canvas.setFill(getFill());
        canvas.setLine(getLine());
        canvas.drawPolygon(_x, _y);
    }

}
