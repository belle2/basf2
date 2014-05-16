package b2daq.dqm.graphics;

public class HyperRef {

	private Canvas _canvas = null;
	private int _channel_x = -1;
	private int _channel_y = -1;
	
	public HyperRef() {}

	public HyperRef(Canvas canvas, int channel_x, int channel_y) {
		_canvas = canvas;
		_channel_x = channel_x;
		_channel_y = channel_y;
	}
	
	public void setCanvas(Canvas canvas) {
		_canvas = canvas;
	}
	
	public Canvas getCanvas() {
		return _canvas;
	}

	public void setChannelX(int channel_x) {
		_channel_x = channel_x;
	}

	public void setChannelY(int channel_y) {
		_channel_y = channel_y;
	}

	public int getChannelX() {
		return _channel_x;
	}
	
	public int getChannelY() {
		return _channel_y;
	}
	
}
