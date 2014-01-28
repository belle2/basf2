package b2dqm.java.ui;

import java.awt.Dimension;

import b2daq.graphics.GRect;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2daq.java.graphics.JavaGCanvasPanel;
import b2dqm.core.DQMState;

public class DQMStateLabel extends JavaGCanvasPanel {

	private static final long serialVersionUID = 1L;
	private DQMState _state = new DQMState();
	private GRect _rect = new GRect(0.02, 0.02, 0.96, 0.96);
	private GText _text = new GText("", 0.5, 0.5, "center middle");
	
	public DQMStateLabel() {
		super();
		setPreferredSize(new Dimension(150, 30));
		setMaximumSize(new Dimension(150, 30));
		setMinimumSize(new Dimension(150, 30));
		_text.setFontSize(1.5);
		_rect.setLineWidth(2);
		add(_rect);
		add(_text);
		setOpaque(false);
		update();
	}

	public void setEnabled(boolean enabled) {
		super.setEnabled(enabled);
		update();
	}
	
	public void setState(int state) {
		_state.copy(state);
	}
	
	public void update() {
		if ( !isEnabled() ) {
			_rect.setLineColor(HtmlColor.GRAY);
			_rect.setFillColor(HtmlColor.LIGHT_GRAY);
			_text.setFontColor(HtmlColor.GRAY);
			_text.setText("");
		} else {
			_text.setFontSize(1.5);
			if ( _state.equals(DQMState.RUNNING_S)) {
				_rect.setLineColor(HtmlColor.GREEN);
				_rect.setFillColor(HtmlColor.RUNNING_GREEN);
				_text.setFontColor(HtmlColor.WHITE);
			} else if (_state.getLabel().endsWith("_S")) {
				_rect.setLineColor(HtmlColor.CYAN);
				_rect.setFillColor(HtmlColor.BLUE);
				_text.setFontColor(HtmlColor.WHITE);
			} else if (_state.getLabel().endsWith("_TS")) {
				_rect.setLineColor(HtmlColor.YELLOW);
				_rect.setFillColor(HtmlColor.GOLD);
				_text.setFontColor(HtmlColor.WHITE);
			} else if (_state.getLabel().endsWith("_ES")) {
				_rect.setLineColor(new HtmlColor("#BB0000"));
				_rect.setFillColor(HtmlColor.RED);
				_text.setFontColor(HtmlColor.WHITE);
			} else if (_state.getLabel().endsWith("_RS")) {
				_rect.setLineColor(new HtmlColor("#DDA0DD"));
				_rect.setFillColor(new HtmlColor("#9932CC"));
				_text.setFontColor(HtmlColor.WHITE);
				_text.setText("ABORTING");
			} else {
				_rect.setLineColor(HtmlColor.RED);
				_rect.setFillColor(HtmlColor.WHITE);
				_text.setFontColor(HtmlColor.RED);
			}
			_text.setText(_state.getAlias());
		}
		this.repaint();
	}
}
