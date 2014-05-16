package b2daq.ui;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;
import javax.swing.text.BadLocationException;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;

import b2daq.graphics.FontProperty;
import b2daq.graphics.HtmlColor;


public class PopupWindow extends JFrame {

	static public SimpleAttributeSet createAttribute(FontProperty font) {
		SimpleAttributeSet sas = new SimpleAttributeSet();
		HtmlColor fcolor = font.getColor();
		HtmlColor bcolor = font.getBackgroundColor();
		StyleConstants.setFontSize(sas, 14);
		if ( fcolor != null) {
			if ( !fcolor.isNull())
				StyleConstants.setForeground(sas, new Color(fcolor.getRed(),
					fcolor.getGreen(), fcolor.getBlue()));
		}  
		if ( bcolor != null) {
			if ( !bcolor.isNull())
				StyleConstants.setBackground(sas, new Color(bcolor.getRed(),
					bcolor.getGreen(), bcolor.getBlue()));
		}  
		return sas;
	}

	private static final long serialVersionUID = 1L;
	private String _message;
	private int _fadeout_time;
	private FontProperty _font;
	private String _title;
	private JButton _ok_button;
	private JButton _detail_button;
	
	public PopupWindow() {
		this("TEST", 0);
	}
	
	public PopupWindow(String title, String msg, int time, FontProperty font) {
		_title = title;
		_fadeout_time = time;
		_message = msg;
		_font = font;
		init();
	}
	
	public PopupWindow(String msg, int time, FontProperty font) {
		this(msg, msg, time, font);
	}
	
	public PopupWindow(String msg, int time) {
		this(msg, time, new FontProperty());
	}
	
	public PopupWindow(String title, String msg) {
		this(title, msg, 5000, new FontProperty());
	}

	public PopupWindow(String msg) {
		this(msg, 5000);
	}
	
	public String getMessage() { return _message; }
	public int getFadeoutTime() { return _fadeout_time; }
	public void setMessage(String msg) { _message = msg; }
	public void setFadeoutTime(int ft) { _fadeout_time = ft; }
	
	public void init() {
		setResizable(false);
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		setTitle(_title);
		setSize(330, 240);
		setLocationRelativeTo(null);
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		JPanel middlePanel = new JPanel();
		middlePanel.setLayout(new BoxLayout(middlePanel, BoxLayout.X_AXIS));
		
		JScrollPane scrollpanel = new JScrollPane();
		JTextPane messagePanel = new JTextPane();
		messagePanel.setEditable(false);
		messagePanel.setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
		HtmlColor bcolor = _font.getBackgroundColor();
		if (!bcolor.isNull()) {
			messagePanel.setBackground(new Color(bcolor.getRed(), bcolor.getGreen(), bcolor.getBlue()));
		} else { 
			messagePanel.setBackground(Color.WHITE);
		}
		scrollpanel.setViewportView(messagePanel);
		panel.add(scrollpanel);
		middlePanel.add(Box.createRigidArea(new Dimension(10, 0)));
		middlePanel.add(scrollpanel);
		middlePanel.add(Box.createRigidArea(new Dimension(10, 0)));

		JPanel bottomPanel = new JPanel();
		bottomPanel.setLayout(new BoxLayout(bottomPanel, BoxLayout.X_AXIS));
		_ok_button = new JButton("OK");
		_ok_button.setSize(80, 30);
		_detail_button = new JButton("Detail");
		_detail_button.setSize(80, 30);
		_ok_button.addActionListener(new ActionListener(){
			public void actionPerformed(ActionEvent arg0) {
				((JFrame)SwingUtilities.getWindowAncestor(_ok_button)).dispose();
			}
		});
		_detail_button.addActionListener(new ActionListener(){
			public void actionPerformed(ActionEvent arg0) {
				((PopupWindow)SwingUtilities.getWindowAncestor(_detail_button)).openDetail();
			}
		});
		bottomPanel.add(_ok_button);
		bottomPanel.add(Box.createRigidArea(new Dimension(20, 50)));
		bottomPanel.add(_detail_button);
		
		panel.add(Box.createRigidArea(new Dimension(10, 10)));
		panel.add(middlePanel);
		panel.add(bottomPanel);

		add(panel);
		try {
			messagePanel.getDocument().insertString(0, _message, createAttribute(_font));
		} catch (BadLocationException e) {}
		add(panel);
		setVisible(true);
	}
	
	public void openDetail() {
		setSize(480, 360);
		setLocationRelativeTo(null);
		_detail_button.setEnabled(false);
	}
	
}
