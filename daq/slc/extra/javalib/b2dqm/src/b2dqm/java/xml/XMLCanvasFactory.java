package b2dqm.java.xml;

import java.util.ArrayList;

import javax.swing.JComponent;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;
import b2dqm.java.ui.CanvasPanel;
import b2dqm.property.*;


public class XMLCanvasFactory {

	private HistoPackage _pack = null;
	private Canvas _canvas;
	private ArrayList<GHistoProperty> _histo_pro_v = new ArrayList<GHistoProperty>();
	
	public XMLCanvasFactory(HistoPackage pack) {
		_pack = pack;
	}
	
	public JComponent getCanvas(Element el) {
		CanvasPanel panel = new CanvasPanel();
		_canvas = panel.getCanvas();
		GProperty pro = getProperty(el);
		
		if ( pro != null && pro.getType().matches(CanvasProperty.ELEMENT_LABEL)) {
			((CanvasProperty)pro).setup(_pack);
		}
		return panel;
	}
	
	public GProperty getProperty (Element el) {
		GProperty pro = null;
		String node_name = el.getNodeName();
		if ( node_name == CanvasProperty.ELEMENT_LABEL ) {
			pro = new CanvasProperty(_canvas);
		} else if ( node_name == GHistoProperty.ELEMENT_LABEL ) {
			pro = new GHistoProperty(_canvas, _pack, el.getAttribute("name"));
			_histo_pro_v.add((GHistoProperty)pro);
		} else if ( node_name == GMonShapeProperty.ELEMENT_LABEL ) {
			pro = new GMonShapeProperty(_canvas, _pack, el.getAttribute("name"));
		} else if ( node_name == GMonArrowProperty.ELEMENT_LABEL ) {
			pro = new GMonArrowProperty(_canvas, _pack, el.getAttribute("name"));
		} else if ( node_name == GGraph1Property.ELEMENT_LABEL ) {
			pro = new GGraph1Property(_canvas);
		} else if ( node_name == GLegendProperty.ELEMENT_LABEL ) {
			pro = new GLegendProperty(_canvas);
		} else if ( node_name == GPadProperty.ELEMENT_LABEL ) {
			pro = new GPadProperty(_canvas);
		} else if ( node_name == GAxisXProperty.ELEMENT_LABEL ) {
			pro = new GAxisXProperty(_canvas);
		} else if ( node_name == GAxisYProperty.ELEMENT_LABEL ) {
			pro = new GAxisYProperty(_canvas);
		} else if ( node_name == GAxisY2Property.ELEMENT_LABEL ) {
			pro = new GAxisY2Property(_canvas);
		} else if ( node_name == GColorAxisProperty.ELEMENT_LABEL ) {
			pro = new GColorAxisProperty(_canvas);
		} else if ( node_name == GRectProperty.ELEMENT_LABEL ) {
			pro = new GRectProperty(_canvas);
		} else if ( node_name == GTextProperty.ELEMENT_LABEL ) {
			pro = new GTextProperty(_canvas);
		} else if ( node_name == GCircleProperty.ELEMENT_LABEL ) {
			pro = new GCircleProperty(_canvas);
		}
		if ( pro != null ) {
			pro.put("value", el.getTextContent().replaceAll("\t", "").replaceAll("\n", ""));
			NamedNodeMap node_map = el.getAttributes(); 
			for (int m = 0;m < node_map.getLength();m++) {
				Node node = node_map.item(m);
				pro.put(node.getNodeName(), node.getTextContent());
			}
			NodeList node_list = el.getChildNodes(); 
			for (int m = 0;m < node_list.getLength();m++) {
				if ( node_list.item(m).getNodeType() == Node.ELEMENT_NODE ) {
					Element el_ch = (Element)node_list.item(m);
					GProperty pro_ch = getProperty(el_ch);
					if ( pro_ch == null ) {
						pro.put(el_ch.getNodeName(), el_ch.getTextContent());
					} else {
						pro.put(pro_ch);
					}
				}
			}
		}
		return pro;
	}

	public ArrayList<GHistoProperty> getHistoProperties() {
		return _histo_pro_v;
	}
	
}
