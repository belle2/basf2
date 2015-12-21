package org.belle2.daq.dqm.graphics;

public class HyperRef {

	private HistogramCanvas canvas = null;
	private int channelX = -1;
	private int channelY = -1;
	
	public HyperRef() {}

	public HyperRef(HistogramCanvas canvas, int channel_x, int channel_y) {
		this.canvas = canvas;
		channelX = channel_x;
		channelY = channel_y;
	}
	
	public void setCanvas(HistogramCanvas canvas) {
		this.canvas = canvas;
	}
	
	public HistogramCanvas getCanvas() {
		return canvas;
	}

	public void setChannelX(int channel_x) {
		channelX = channel_x;
	}

	public void setChannelY(int channel_y) {
		channelY = channel_y;
	}

	public int getChannelX() {
		return channelX;
	}
	
	public int getChannelY() {
		return channelY;
	}
	
}
