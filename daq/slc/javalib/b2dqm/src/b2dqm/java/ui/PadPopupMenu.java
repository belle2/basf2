package b2dqm.java.ui;

import javax.swing.JMenu;

import b2dqm.graphics.GPad;


public class PadPopupMenu extends JMenu {

	public PadPopupMenu(String name, GPad pad, CanvasPanel panel) {
		super(name);
		JMenu menu = new PadBoundsSettingMenu("Set Bounds", pad, panel);
		add(menu);
		menu = new LineSettingMenu("Set Line", pad.getLine(), panel);
		add(menu);
		menu = new FillSettingMenu("Set Fill", pad.getFill(), panel);
		add(menu);
	}

	private static final long serialVersionUID = 1L;

}
