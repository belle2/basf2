package b2dqm.java.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.ScrollPaneConstants;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import b2daq.java.ui.LogViewPanel;
import b2daq.java.ui.UScrollPanel;
import b2daq.java.ui.UTabPanel;
import b2daq.java.ui.UTablePanel;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;
import b2dqm.java.ui.CanvasPanel;
import b2dqm.property.GHistoProperty;

public class XMLPanelFactory {
	
	static public String parseUnicode(String str) {
		String str_out = str.replace("&lt;", "<");
	    str_out = str_out.replace("&gt;", ">");
	    str_out = str_out.replace("&quot;", "\"");
	    str_out = str_out.replace("&apos;", "'");
	    str_out = str_out.replace("&amp;", "&");
		return str_out;
	}
	
	private HistoPackage _pack = null;
	private HashMap<String, Canvas> _canvas_map = new HashMap<String, Canvas>(); 
	private ArrayList<XMLCanvasFactory> _canvas_fact_v = new ArrayList<XMLCanvasFactory>(); 
	
	public void setPackage(HistoPackage pack) {
		_pack = pack;
	}
	
	public JComponent createPanel(Element root) {
		JComponent panel = null;
		try {
			panel = getPanel((Element)root.getChildNodes().item(1));
			if (root.hasAttribute("title")) {
				panel.setName(root.getAttribute("title"));
			}
			Iterator<XMLCanvasFactory> fact_it = _canvas_fact_v.iterator();
			while (fact_it.hasNext()) {
				XMLCanvasFactory fact = fact_it.next();
				Iterator<GHistoProperty> histo_pro_it = fact.getHistoProperties().iterator();
				while (histo_pro_it.hasNext()) {
					GHistoProperty pro = histo_pro_it.next();
					pro.setHyperLink(_canvas_map);
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		panel.repaint();
		return panel;
	}
	
	private JComponent getPanel(Element el) {
		JComponent panel = null;
		if (el.getNodeName()=="tabbedPanel") {
			panel = getTabbedPanel(el); 
		} else if (el.getNodeName()=="canvas") {
			panel = getCanvas(el); 
		} else if (el.getNodeName()=="table") {
			panel = getTable(el); 
		} else if (el.getNodeName()=="scrollPanel") {
			panel = getScrollPanel(el); 
		} else if (el.getNodeName()=="logPanel") {
			panel = getLogViewPanel(el); 
		}
		return panel;
	}
	
	private JComponent getTable(Element el) {
		int col = Integer.parseInt(el.getAttribute("col"));
		int row = Integer.parseInt(el.getAttribute("row"));
		int nel = 0;
		NodeList node_list = el.getChildNodes();
		UTablePanel table = new UTablePanel(col, row);
		if (el.hasAttribute("width-ratio")) {
			table.setWidthRatio(el.getAttribute("width-ratio"));
		}
		if (el.hasAttribute("height-ratio")) {
			table.setHeightRatio(el.getAttribute("height-ratio"));
		}
		if (el.hasAttribute("cel-width")) {
			table.setCelWidthRatio(Double.parseDouble(el.getAttribute("cel-width")));
		}
		if (el.hasAttribute("cel-height")) {
			table.setCelHeightRatio(Double.parseDouble(el.getAttribute("cel-height")));
		}
		for (int nc = 0; nc < col; nc++) {
			for (int nr = 0; nr < row; nr++) {
				for (; nel < node_list.getLength(); nel++) {
					Node node = node_list.item(nel);
					if ( node.getNodeType() == Node.ELEMENT_NODE ) {
						JComponent widget = getPanel((Element)node);
						table.add(widget);
						nel++;
						break;
					}
				}
			}
		}
		return table;
	}

	private JComponent getCanvas(Element el) {
		XMLCanvasFactory c_fact = new XMLCanvasFactory(_pack);
		_canvas_fact_v.add(c_fact);
		CanvasPanel canvas = (CanvasPanel)c_fact.getCanvas(el);
		_canvas_map.put(canvas.getCanvas().getName(), canvas.getCanvas());
		return canvas;
	}

	private JComponent getTabbedPanel(Element el) {
		UTabPanel tabpanel = new UTabPanel();
		NodeList node_list = el.getChildNodes();
		for (int n = 0; n < node_list.getLength(); n++) {
			Node node = node_list.item(n);
			if ( node.getNodeType() == Node.ELEMENT_NODE ) {
				Element ch = (Element)node;
				if (ch.getNodeName()=="tab") {
					if (ch.getChildNodes().getLength() > 2) {
						tabpanel.addTab(ch.getAttribute("title"), 
								getPanel((Element)ch.getChildNodes().item(1)));
					} else {
						tabpanel.addTab(ch.getAttribute("title"), new JPanel());
					}
				}
			}
		}
		return tabpanel;
	}

	private JComponent getScrollPanel(Element el) {
		UScrollPanel panel = new UScrollPanel();
		panel.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS );
		for (int n = 0; n < el.getChildNodes().getLength(); n++) {
			try {
				Element ch = (Element)el.getChildNodes().item(n);
				JComponent ch_panel = getPanel(ch);
				if (ch.getNodeName()=="table") {
					((UTablePanel)ch_panel).setUseMinimum(true);
				}
				panel.setViewportView(ch_panel);
				break;
			} catch(Exception e) {}
		}
		return panel;
	}

	private JComponent getLogViewPanel(Element el) {
		return new LogViewPanel();
	}

	public HistoPackage getPackage() {
		return _pack;
	}

}
