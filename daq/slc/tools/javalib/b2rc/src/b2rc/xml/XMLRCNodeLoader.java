package b2rc.xml;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import b2rc.core.COPPERNode;
import b2rc.core.RONode;
import b2rc.core.DataSender;
import b2rc.core.FEEModule;
import b2rc.core.FTSW;
import b2rc.core.HSLB;
import b2rc.core.RCHost;
import b2rc.core.RCNode;
import b2rc.core.RCNodeGroup;
import b2rc.core.RCNodeSystem;
import b2rc.core.TTDNode;

public class XMLRCNodeLoader {

	private int _version;
	private String _dir = "";
	private RCNodeSystem _system = new RCNodeSystem();
	private RCNodeGroup _group = new RCNodeGroup();
	private HashMap<String, COPPERNode> _copper_m = new HashMap<String, COPPERNode>();
	private HashMap<String, FEEModule> _module_m = new HashMap<String, FEEModule>();
	private HashMap<Integer, FTSW> _ftsw_m = new HashMap<Integer, FTSW>();
	private HashMap<String, RCHost> _host_m = new HashMap<String, RCHost>();
	private HashMap<String, DataSender> _sender_m = new HashMap<String, DataSender>();
	private HashMap<String, InputStream> _istream_m = new HashMap<String, InputStream>();

	public XMLRCNodeLoader(String dir) {
		_dir = dir;
	}

	public XMLRCNodeLoader() {
		_dir = "";
	}

	public void setVersion(int version) {
		_version = version;
	}

	public int getVersion() {
		return _version;
	}

	public RCNodeSystem getSystem() {
		return _system;
	}

	public HashMap<String, COPPERNode> getCOPPERNodeList() {
		return _copper_m;
	}

	public HashMap<String, FEEModule> getModuleList() {
		return _module_m;
	}

	public HashMap<String, RCHost> getHostList() {
		return _host_m;
	}

	public HashMap<String, DataSender> getSenderList() {
		return _sender_m;
	}

	private Element parse(String path) {
		Element root = null;
		try {
			DocumentBuilderFactory dbfactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder builder = dbfactory.newDocumentBuilder();
			//Document doc = builder.parse(getClass().getResourceAsStream(_dir_+"/"+path));
			Document doc = builder.parse(_istream_m.get(path));
			root = doc.getDocumentElement();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return root;
	}

	public void load(String entry) throws ParserConfigurationException,
			FileNotFoundException, SAXException, IOException {
		Element el_system = parse(entry + ".xml");
		loadHosts(el_system);
		loadFTSWs(el_system);
		loadCOPPERNodes(el_system);
		loadNodes(el_system);
		ArrayList<FEEModule> module_v = _system.getModules();
		for (FEEModule module : module_v) {
			String type = module.getType();
			if ( type != null ) {
				if ( !_system.hasModuleClass(type)) {
					_system.addModules(type, new ArrayList<FEEModule>());
				}
				module.setId(_system.getModules(type).size());
				_system.getModules(type).add(module);
			}
		}
	}

	public void loadHosts(Element el) throws ParserConfigurationException,
			FileNotFoundException, SAXException, IOException {
		final String parent_tag = "hosts";
		final String tag = "host";
		if (el.hasAttribute(parent_tag)) {
			String network = el.getAttribute(parent_tag);
			String path = network + ".xml";
			Element el_network = parse(path);
			int version = 0;
			try {
				version = parseInt(el_network.getAttribute("version"));
			} catch (Exception e) {
			}
			NodeList el_v = el_network.getChildNodes();
			int id = 0;
			for (int i = 0; i < el_v.getLength(); i++) {
				if ( el_v.item(i) instanceof Element) {
					Element elc = (Element) el_v.item(i);
					if (elc.getTagName() == tag) {
						RCHost host = new RCHost();
						host.setVersion(version);
						host.setId(id);id++;
						host.setName(elc.getAttribute("name"));
						host.setType(elc.getAttribute("type"));
						host.setProductID(parseInt(elc.getAttribute("produt_id")));
						host.setLocation(elc.getAttribute("location"));
						_system.addHost(host);
						_host_m.put(host.getName(), host);
					}
				}
			}
		}

	}

	public void loadFTSWs(Element el) {
		if (el.hasAttribute("ftswset")) {
			String ftswset = el.getAttribute("ftswset");
			String path = ftswset + ".xml";
			Element el_ftswset = parse(path);
			int version = 0;
			NodeList el_v = el_ftswset.getChildNodes();
			int id = 0;
			for (int i = 0; i < el_v.getLength(); i++) {
				if ( el_v.item(i) instanceof Element) {
					Element elc = (Element) el_v.item(i);
					if (elc.getTagName() == "ftsw") {
						FTSW ftsw = new FTSW();
						ftsw.setVersion(version);
						ftsw.setId(id++);
						ftsw.setChannel(parseInt(elc.getAttribute("channel")));
						ftsw.setProductID(parseInt(elc.getAttribute("product_id")));
						ftsw.setLocation(elc.getAttribute("location"));
						ftsw.setFirmware(elc.getAttribute("firmware"));
						int mode = FTSW.TRIG_NORMAL;
						String mode_s = elc.getAttribute("trigger_mode").toUpperCase();
						if ( mode_s == "IN" ) mode = FTSW.TRIG_IN;
						else if ( mode_s == "TLU" ) mode = FTSW.TRIG_TLU;
						else if ( mode_s == "PULSE" ) mode = FTSW.TRIG_PULSE;
						else if ( mode_s == "REVO" ) mode = FTSW.TRIG_REVO;
						else if ( mode_s == "RANDOM" ) mode = FTSW.TRIG_RANDOM;
						else if ( mode_s == "POSSION" ) mode = FTSW.TRIG_POSSION;
						else if ( mode_s == "ONCE" ) mode = FTSW.TRIG_ONCE;
						else if ( mode_s == "STOP" ) mode = FTSW.TRIG_STOP;
						ftsw.setTriggerMode(mode);
						_system.addFTSW(ftsw);
						_ftsw_m.put(ftsw.getProductID(), ftsw);
					}
				}
			}
		}

	}

	public void loadNodes(Element el) {
		RCNode node = null;
		String name = el.getAttribute("name");
		String tag = el.getTagName();
		if ( tag == COPPERNode.TAG ) {
			COPPERNode copper = new COPPERNode();
			node = copper;
			copper.setId(_system.getCOPPERNodes().size());
			_system.addCOPPERNode(copper);
			String hostname = el.getAttribute("host");
			if (_host_m.containsKey(hostname)) {
				copper.setHost(_host_m.get(hostname));
			} else {
				copper.setHost(null);
			}
			String module_class = el.getAttribute("module_class");
			copper.setModuleClass(module_class);
			if (_copper_m.containsKey(module_class)) {
				for (int slot = 0; slot < 4; slot++) {
					HSLB hslb = _copper_m.get(module_class).getHSLB(slot);
					if ( hslb != null ) {
						HSLB hslb_new = new HSLB();
						hslb_new.setFirmware(hslb.getFirmware());
						hslb_new.setLocation(hslb.getLocation());
						hslb_new.setProductID(hslb.getProductID());
						hslb_new.setTriggerMode(hslb.getTriggerMode());
						hslb_new.setUsed(hslb.isUsed());
						hslb_new.setId(_system.getHSLBs().size());
						hslb_new.setVersion(_version);
						_system.addHSLB(hslb_new);
						copper.setHSLB(slot, hslb_new);
						if (hslb.getFEEModule() != null) {
							FEEModule module = new FEEModule();
							module.setType(hslb.getFEEModule().getType());
							module.setLocation(hslb.getFEEModule().getLocation());
							module.setProductID(hslb.getFEEModule().getProductID());
							module.setRegisters(hslb.getFEEModule().getRegisters());
							module.setVersion(_version);
							hslb_new.setFEEModule(module);
							_system.addModule(module);
						}
					}
				}
			}
			copper.getSender().setScript(el.getAttribute("script"));
			short port = (el.hasAttribute("port")) ? parseShort(el.getAttribute("port")) : -1;
			copper.getSender().setPort(port);
			int event_size = (el.hasAttribute("event_size")) ? parseInt(el.getAttribute("event_size")) : -1;
			copper.getSender().setEventSize(event_size);
			copper.getSender().setId(copper.getId());
		} else if ( tag == TTDNode.TAG ) {
			TTDNode ttd = new TTDNode();
			node = ttd;
			ttd.setId(_system.getTTDNodes().size());
			_system.addTTDNode(ttd);
			String hostname = el.getAttribute("host");
			if (_host_m.containsKey(hostname)) {
				ttd.setHost(_host_m.get(hostname));
			} else {
				ttd.setHost(null);
			}
			String[] ftsw_v = el.getAttribute("ftsw").split(",");
			for (int ich = 0; ich < ftsw_v.length; ich++) {
				int product_id = parseInt(ftsw_v[ich]);
				if (_ftsw_m.containsKey(product_id)) {
					ttd.addFTSW(_ftsw_m.get(product_id));
				}
			}
		} else if ( tag == RONode.TAG ) {
			RONode recv = new RONode();
			node = recv;
			recv.setId(_system.getReceiverNodes().size());
			recv.setScript(el.getAttribute("script"));
			_system.addReceiverNode(recv);
			String hostname = el.getAttribute("host");
			if (_host_m.containsKey(hostname)) {
				recv.setHost(_host_m.get(hostname));
			} else {
				recv.setHost(null);
			}
			String [] hostname_v = el.getAttribute("senders").split(",");
			recv.clearSenders();
			for (String hostname_s : hostname_v) {
				recv.addSender(hostname_s);
			}
		} else if (el.hasChildNodes()) {
			if (tag == "group") {
				RCNodeGroup group = new RCNodeGroup();
				group.setLabel(name);
				group.setCols(parseInt(el.getAttribute("cols")));
				group.setRows(parseInt(el.getAttribute("rows")));
				_system.addNodeGroup(group);
				_group = group;
			}
			NodeList el_v = el.getChildNodes();
			for (int i = 0; i < el_v.getLength(); i++) {
				if ( el_v.item(i) instanceof Element) {
					loadNodes((Element) el_v.item(i));
				}
			}
		}

		if (node != null) {
			node.setVersion(_system.getVersion());
			node.setUsed(!(el.getAttribute("used").matches("false")));
			node.setIndex(_system.getNodes().size());
			node.setName(name);
			_system.addNode(node);
			if (_group != null) {
				_group.addNode(node);
			}
		}
	}

	void loadCOPPERNodes(Element el) {
		HashMap<String, Element> el_m = new HashMap<String, Element>();
		search(el_m, el, "module_class");
		HashMap<String, Element> elc_m = new HashMap<String, Element>();
		for (String label : el_m.keySet()) {
			search(elc_m, el_m.get(label), "type");
		}

		for (String label : elc_m.keySet()) {
			NodeList el_reg_v = elc_m.get(label).getChildNodes();
			FEEModule module = new FEEModule();
			module.setType(label);
			module.setVersion(_version);
			for (int i = 0; i < el_reg_v.getLength(); i++) {
				if ( el_reg_v.item(i) instanceof Element) {
					Element el_reg = (Element) el_reg_v.item(i);
					FEEModule.Register reg = module.createRegister();
					reg.setName(el_reg.getAttribute("name"));
					reg.setAddress(parseInt(el_reg.getAttribute("address")));
					reg.setLength(parseInt(el_reg.getAttribute("length")));
					if ( reg.length() <= 0 ) reg.setLength(1);
					reg.setSize(parseInt(el_reg.getAttribute("size")));
					reg.setValues(parseInt(el_reg.getAttribute("value")));
					module.addRegister(reg);
				}
			}
			_module_m.put(label, module);
		}
		for (String label : el_m.keySet()) {
			NodeList el_v = el_m.get(label).getChildNodes();
			COPPERNode copper = new COPPERNode();
			for (int i = 0; i < el_v.getLength(); i++) {
				if ( el_v.item(i) instanceof Element) {
					Element elc = (Element) el_v.item(i);
					if (elc.getTagName() == "hslb") {
						HSLB hslb = new HSLB();
						hslb.setUsed(elc.getAttribute("used")!="false");
						hslb.setFirmware(elc.getAttribute("firmware"));
						hslb.setFEEModule(_module_m.get(elc.getAttribute("type")));
						copper.setHSLB((elc.getAttribute("slot").codePointAt(0) - 'a'),hslb);
					}
				}
			}
			_copper_m.put(label, copper);
		}
	}

	private void search(HashMap<String, Element> el_m, Element el, String type) {
		NodeList el_v = el.getChildNodes();
		for (int i = 0; i < el_v.getLength(); i++) {
			if ( el_v.item(i) instanceof Element ) {
				Element elc = (Element) el_v.item(i);
				if ( elc.hasAttribute(type) ) {
					String value = elc.getAttribute(type);
					if ( !el_m.containsKey(value)) {
						String path = value + ".xml";
						el_m.put(value, parse(path));
					}
				} else {
					search(el_m, elc, type);
				}
			}
		}
	}

	private int parseInt(String arg) {
		try {
			return Integer.parseInt(arg);
		} catch (Exception e) {
			return 0;
		}
	}

	private short parseShort(String arg) {
		try {
			return Short.parseShort(arg);
		} catch (Exception e) {
			return 0;
		}
	}

	public HashMap<String, InputStream> getInputstreams() {
		return _istream_m;
	}

	public void setInputstreams(HashMap<String, InputStream> istream_m) {
		_istream_m = istream_m;
	}

	public void addInputstream(String label, InputStream istream) {
		_istream_m.put(label, istream);
	}

}
