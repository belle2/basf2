package b2daq.graphics;

import b2daq.graphics.FillProperty;
import b2daq.graphics.FontProperty;
import b2daq.graphics.HtmlColor;
import b2daq.graphics.LineProperty;

public class GActiveLabel extends GShape {

	protected FillProperty _activeFill = null;
	protected LineProperty _activeLine = null;
	protected FontProperty _activeFont = null;
	protected FillProperty _unactiveFill = null;
	protected LineProperty _unactiveLine = null;
	protected FontProperty _unactiveFont = null;

	protected boolean _active = false;
	protected double _baseX = 100./400.;
	protected double _baseY = 50./450.;
	protected double _height = 30./450.;
	protected double _width = 100./400.;
	private String _text;

	public GActiveLabel() {
		_activeFill = new FillProperty(HtmlColor.BLUE, 2);
		_activeLine = new LineProperty(HtmlColor.CYAN, 1);
		_activeFont = new FontProperty(HtmlColor.WHITE, HtmlColor.NULL);
		_unactiveFill = new FillProperty(HtmlColor.LIGHT_GRAY, 2);
		_unactiveLine = new LineProperty(HtmlColor.GRAY, 2);
		_unactiveFont = new FontProperty(HtmlColor.GRAY, HtmlColor.NULL);
	}

	public GActiveLabel(String text) {
		this();
	}
	
	public GActiveLabel(String text, double x, double y) {
		this(text);
		_baseX = x;
		_baseY = y;
	}
	public GActiveLabel(String text, double x, double y, double width, double height) {
		this(text, x, y);
		_width = width;
		_height = height;
	}

	public GActiveLabel(String text, double x, double y, double width, double height, boolean active) {
		this(text, x, y, width, height);
		_active = active;
	}

	public void draw(GCanvas canvas) {
		canvas.store();
		if ( _active ) {
			canvas.setFill(_activeFill);
			canvas.setLine(_activeLine);
			canvas.setFont(_activeFont);
		} else {
			canvas.setFill(_unactiveFill);
			canvas.setLine(_unactiveLine);
			canvas.setFont(_unactiveFont);
		}
		canvas.drawRect(_baseX, _baseY, _width, _height);
		canvas.drawString(_text, _baseX + _width * 0.5, _baseY + _height * 0.5, "center middle");
		canvas.restore();
	}
	
	public void setActive(boolean active) {
		_active = active;
	}
	
	public void setText(String text) {
		_text = text;
	}

}
