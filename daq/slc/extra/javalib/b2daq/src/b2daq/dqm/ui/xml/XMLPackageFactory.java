package b2daq.dqm.ui.xml;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoFactory;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.core.MonObject;
import java.util.Date;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class XMLPackageFactory {

    static public HistoPackage readConfig(Element root) {
        HistoPackage pack = new HistoPackage();
        pack.setName(root.getAttribute("name"));
        NodeList histo_list = root.getElementsByTagName("histo");
        for (int nhisto = 0; nhisto < histo_list.getLength(); nhisto++) {
            Node node = histo_list.item(nhisto);
            if (node.getNodeType() == Node.ELEMENT_NODE) {
                Element el_h = (Element) node;
                MonObject histo = null;
                histo = HistoFactory.create(el_h.getAttribute("type"));
                if (histo != null) {
                    try {
                        XMLHistoFactory.readConfig((Histo) histo, el_h);
                    } catch (Exception e) {
                    }
                    pack.addMonObject(histo);
                }
            }
        }
        return pack;
    }

    static public void readUpdate(HistoPackage pack, Element root) {
        NodeList histo_list = root.getElementsByTagName("histo");
        long update_time = (new Date()).getTime();
        if (root.hasAttribute("update_time")) {
            update_time = Long.parseLong(root.getAttribute("update_time"));
        }
        for (int nhisto = 0; nhisto < histo_list.getLength(); nhisto++) {
            Node node = histo_list.item(nhisto);
            if (node.getNodeType() == Node.ELEMENT_NODE) {
                Element el_h = (Element) node;
                try {
                    Histo histo = (Histo) pack.getHisto(nhisto);
                    if (histo != null) {
                        XMLHistoFactory.readUpdate(histo, el_h);
                        histo.setTime(update_time);
                    }
                } catch (Exception e) {
                }
            }
        }
    }
}
