package b2daq.java.ui;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;

import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.border.BevelBorder;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import javax.swing.text.html.HTMLDocument;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.java.io.FileDataWriter;


public class LogPanel extends JScrollPane {

	private static final long serialVersionUID = 1L;
	private JEditorPane _text_panel= new JEditorPane("text/html", "");
	private LogLevel _threshold = LogLevel.DEBUG;
	private ArrayList<Log> _log_v = new ArrayList<Log>();
	private int _max_length = 200;
	
	public LogPanel(String name) {
		this();
		setName(name);
	}
	
	public LogPanel() {
		setName("log_panel");
		setPriorityThreshold(LogLevel.DEBUG);
		_text_panel.setBackground(Color.WHITE);
		_text_panel.setEditable(false);
		_text_panel.addMouseListener(new MouseAdapter(){
			@Override
			public void mouseClicked(MouseEvent arg0) {
				if (!SwingUtilities.isRightMouseButton(arg0)) return;
				JPopupMenu pop_menu = new JPopupMenu();
				JMenu menu = new JMenu("Log history");
				JMenuItem item = new JMenuItem("Clear");
				item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						_text_panel.setText("");
						add(new Log("Cleared the logs.", LogLevel.DEBUG));
					}
				});
				menu.add(item);
				JMenu save_menu = new JMenu("Save as");
				JMenuItem item_html = new JMenuItem("HTML");
				save_menu.add(item_html);
				item_html.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						JFileChooser filechooser = new JFileChooser();
						filechooser.setDialogTitle("Save as a HTML file");
						FileFilter filter = new FileNameExtensionFilter("HTML file(*.html)", "html");
						filechooser.setFileFilter(filter);
						if (filechooser.showSaveDialog(_text_panel) == JFileChooser.APPROVE_OPTION) {
							File file = filechooser.getSelectedFile();
							try {
								FileDataWriter writer = new FileDataWriter(file);
								add(new Log("Saved logs to \""+file.getPath()+"\"", LogLevel.DEBUG));
								writer.writeString(toHTML());
							} catch ( IOException e0 ) {
								e0.printStackTrace();
							} catch ( Exception e1 ) {
								e1.printStackTrace();
							}
						}
					}
				});
				JMenuItem item_xml = new JMenuItem("XML");
				save_menu.add(item_xml);
				item_xml.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						JFileChooser filechooser = new JFileChooser();
						filechooser.setDialogTitle("Save as a XML file");
						FileFilter filter = new FileNameExtensionFilter("XML file(*.xml)", "xml");
						filechooser.setFileFilter(filter);
						if (filechooser.showSaveDialog(_text_panel) == JFileChooser.APPROVE_OPTION) {
							File file = filechooser.getSelectedFile();
							try {
								FileDataWriter writer = new FileDataWriter(file);
								add(new Log("Saved logs to \""+file.getPath()+"\"", LogLevel.DEBUG));
								writer.writeString(toXML());
							} catch (IOException ex) {
								ex.printStackTrace();
							} catch (Exception e1) {
								e1.printStackTrace();
							}
						}
					}
				});
				menu.add(save_menu);
				pop_menu.add(menu);
				pop_menu.show(arg0.getComponent(), arg0.getX(), arg0.getY());
			}
		});
		setBorder(new BevelBorder(BevelBorder.LOWERED));
		setViewportView(_text_panel);
		setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
		add(new Log("Log open.", LogLevel.DEBUG));
		_text_panel.setFont(new Font("Sans", Font.PLAIN, 15));
		_text_panel.setText("");
	}
	
	public void setPriorityThreshold(LogLevel level){
		_threshold = level;
	}
	
	public LogLevel getPriorityThreshold(){
		return _threshold;
	}
	
	public void setMaxLength(int max_length) {
		_max_length = max_length;
	}
	
	synchronized public void add(Log log) {
		if ( _threshold.hiegher(log.getLevel()) ) return;
		HTMLDocument html = ((HTMLDocument)_text_panel.getDocument());
		_log_v.add(log);
		while ( _log_v.size() > _max_length  ) {
			_log_v.remove(0);
		}
		try {
			Element el = html.getRootElements()[0].getElement(0);
			if ( el.getElementCount() > _max_length ) {
				html.remove(0, el.getElement(0).getEndOffset());
			}
			html.insertAfterEnd(html.getCharacterElement(html.getLength()-1),
						createHTML(log));
			_text_panel.setCaretPosition(html.getLength() - 1);
		} catch (BadLocationException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public String createHTML(Log log) {
		return "<span style='color:"+log.getLevel().getFontColor()+";background:"+
		log.getLevel().getFillColor()+";font-family:Arial;font-size:90%;'>"+log.toString()+"</span><br />";
	}
	
	public void update() {
		_text_panel.setText(toString());
	}
	
	public String toHTML() {
		StringBuffer buf = new StringBuffer();
		buf.append("<!doctype html>\n");
		buf.append("<html>\n");
		buf.append("<head>\n");
		buf.append("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		buf.append("<title> log from "+getName()+" recorded at "+ new Date() +"</title>\n");
		buf.append("<style type=\"text/css\">\n");
		for ( LogLevel level : LogLevel.LIST ) {
			buf.append("span."+level.getLabel() + "{" +
					"color:" + level.getFontColor().toString() + ";" +
					"background:" + level.getFillColor().toString() + ";}\n");
		}
		buf.append("</style>");
		buf.append("</head>\n");
		buf.append("<body>\n");
		for ( Log log : _log_v ) buf.append(log.toHTML()+"<br/>\n");
		buf.append("</body>\n");
		return buf.toString();
	}

	public String toXML() {
		StringBuffer buf = new StringBuffer();
		buf.append("<?xml version='1.0'?>\n");
		buf.append("<log_history from='"+getName()+"' date='"+new Date()+"'>\n");
		for ( Log log : _log_v ) buf.append(log.toXML()+"\n");
		buf.append("</log_history>\n");
		return buf.toString();
	}

}
