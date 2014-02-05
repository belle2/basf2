package b2dqm.java.xml;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import b2dqm.core.Axis;
import b2dqm.core.Histo;


public class XMLHistoFactory {
	
	static public void readConfig(Histo histo, Element el) {
		histo.setTabId((char)Integer.parseInt(el.getAttribute("tab")));
		histo.setPositionId((char)Integer.parseInt(el.getAttribute("position")));
		histo.setName(el.getAttribute("name"));
		histo.setTitle(el.getAttribute("title"));
		NodeList list = el.getElementsByTagName("axis-x");
		if ( list.getLength() > 0 && list.item(0).getNodeType() == Node.ELEMENT_NODE ) {
			histo.setAxisX(readAxis((Element)list.item(0)));
		}
		list = el.getElementsByTagName("axis-y");
		if ( list.getLength() > 0 && list.item(0).getNodeType() == Node.ELEMENT_NODE ) {
			histo.setAxisY(readAxis((Element)list.item(0)));
		}
		if (histo.getDataType().matches("H1.")) {
			histo.getData().resize(histo.getAxisX().getNbins()+2);
		} else if (histo.getDataType().matches("H2.")) {
			histo.getData().resize((histo.getAxisX().getNbins()+2)*(histo.getAxisY().getNbins()+2));
		} else if (histo.getDataType().matches("G1.")) {
			histo.getData().resize((histo.getAxisX().getNbins()+2)*2);
		} else if (histo.getDataType().matches("IDM")) {
			histo.getData().resize((histo.getAxisX().getNbins()+2));
		} else if (histo.getDataType().matches("IVM")) {
			histo.getData().resize((histo.getAxisX().getNbins()+2));
		}
	}
	
	public static Axis readAxis(Element el) {
		int nbins = Integer.parseInt(el.getAttribute("nbins"));
		double min = Double.parseDouble(el.getAttribute("min"));
		double max = Double.parseDouble(el.getAttribute("max"));
		String title = el.getAttribute("title");
		return new Axis (nbins, min ,max, title);
	}

	public static void readUpdate(Histo histo, Element el_h) {
		if (el_h.hasAttribute("data")) {
			String [] value_v = el_h.getAttribute("data").split(",");
			for (int n = 0; n < value_v.length && n < histo.getData().length(); n++ ) {
				histo.getData().set(n ,Double.parseDouble(value_v[n]));
			}
		}
		histo.setMaxAndMin();
	}
	
}
