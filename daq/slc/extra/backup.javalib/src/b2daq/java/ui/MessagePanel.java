package b2daq.java.ui;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.Desktop;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Date;

import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.border.BevelBorder;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

import b2daq.java.io.FileDataWriter;

public class MessagePanel extends JScrollPane {

	private static final long serialVersionUID = 1L;
	private JEditorPane _text_panel= new JEditorPane("text/html", "");
	private ArrayList<String> _message_v = new ArrayList<String>();
	private int _max_length = 200;
	
	public MessagePanel(String name, int fontsize) {
		this(fontsize);
		setName(name);
	}

	public void setFontSize(int fontsize) {
		_text_panel.setFont(new Font("Sans", Font.PLAIN, fontsize));
	}
	
	public MessagePanel(int fontsize) {
		setName("log_panel");
		_text_panel.setFont(new Font("Sans", Font.PLAIN, fontsize));
		_text_panel.setBackground(Color.WHITE);
		_text_panel.setEditable(false);
		_text_panel.addMouseListener(new MouseAdapter(){
			@Override
			public void mouseClicked(MouseEvent arg0) {
				if (!SwingUtilities.isRightMouseButton(arg0)) return;
				JPopupMenu pop_menu = new JPopupMenu();
				JMenu menu = new JMenu("Option");
				JMenuItem item = new JMenuItem("Font size");
				item.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						String text = JOptionPane.showInputDialog("Input new font size", ""+_text_panel.getFont().getSize());
						try {
							setFontSize(Integer.parseInt(text));
						} catch (Exception e) {
							
						}
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
								byte [] text = toHTML().getBytes();
								for (byte c : text) {
									writer.writeByte(c);
								}
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
								add("Saved logs to \""+file.getPath()+"\"");
								byte [] text = toXML().getBytes();
								for (byte c : text) {
									writer.writeByte(c);
								}
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
		_text_panel.addHyperlinkListener(new HyperlinkListener() {
			@Override
			public void hyperlinkUpdate(HyperlinkEvent e) {
				if ( e.getEventType() == HyperlinkEvent.EventType.ACTIVATED ) {
					try {
						Desktop.getDesktop().browse(e.getURL().toURI());
					} catch (IOException e1) {
						e1.printStackTrace();
					} catch (URISyntaxException e1) {
						e1.printStackTrace();
					}
				}
			}
		});
		setBorder(new BevelBorder(BevelBorder.LOWERED));
		setViewportView(_text_panel);
		setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
		_text_panel.setText("");
	}
	
	public void setMaxLength(int max_length) {
		_max_length = max_length;
	}
	
	synchronized public void add(String message) {
		_message_v.add(message);
		while ( _message_v.size() > _max_length  ) {
			_message_v.remove(0);
		}
	}
	
	public void update() {
		StringBuffer ss = new StringBuffer();
		for (String message : _message_v) {
			ss.append(message + "\n");
		}
		_text_panel.setText(ss.toString());
	}

	public void reset() {
		_text_panel.setText("");
		_message_v.clear();
	}
	
	public String toHTML() {
		StringBuffer buf = new StringBuffer();
		buf.append("<!doctype html>\n");
		buf.append("<html>\n");
		buf.append("<head>\n");
		buf.append("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		buf.append("<title> log from "+getName()+" recorded at "+ new Date() +"</title>\n");
		buf.append("<style type=\"text/css\">\n");
		buf.append("</style>");
		buf.append("</head>\n");
		buf.append("<body>\n");
		for ( String message : _message_v ) buf.append(message+"\n");
		buf.append("</body>\n");
		return buf.toString();
	}

	public String toXML() {
		StringBuffer buf = new StringBuffer();
		buf.append("<?xml version='1.0'?>\n");
		buf.append("<log_history from='"+getName()+"' date='"+new Date()+"'>\n");
		for ( String message : _message_v ) buf.append(message+"\n");
		buf.append("</log_history>\n");
		return buf.toString();
	}

}
