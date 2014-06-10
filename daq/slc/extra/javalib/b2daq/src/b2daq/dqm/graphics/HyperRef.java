package b2daq.dqm.graphics;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;

public class HyperRef {

	private HistogramCanvas canvas = null;
	private final IntegerProperty channelX = new SimpleIntegerProperty(-1);
	private final IntegerProperty channelY = new SimpleIntegerProperty(-1);
	
	public HyperRef() {}

	public HyperRef(HistogramCanvas canvas, int channel_x, int channel_y) {
		this.canvas = canvas;
		channelX.set(channel_x);
		channelY.set(channel_y);
	}
	
	public void setCanvas(HistogramCanvas canvas) {
		this.canvas = canvas;
	}
	
	public HistogramCanvas getCanvas() {
		return canvas;
	}

	public void setChannelX(int channel_x) {
		channelX.set(channel_x);
	}

	public void setChannelY(int channel_y) {
		channelY.set(channel_y);
	}

	public int getChannelX() {
		return channelX.get();
	}
	
	public int getChannelY() {
		return channelY.get();
	}
	
}
