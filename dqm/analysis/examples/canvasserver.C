{
  TServerSocket *ss = new TServerSocket(9192, kTRUE);
  //THttpServer *m_serv = new THttpServer("http:8081");
  THttpServer *m_serv = new THttpServer("http:8082");
  //m_serv->SetDefaultPage("custom.htm");
  TMonitor *fMon = new TMonitor;
  TList    *fSockets = new TList;   // list of open spy sockets
  
  fMon->Add(ss);
  TCanvas *h = 0;
  while (1) {
    TMessage *mess;
    TSocket  *s;
    
    m_serv->ProcessRequests();      
    s = fMon->Select(50); // 20 Hz
    if(s==0) break; // Error
    if(s==(TSocket *)-1) continue; // timeout
    if (s->IsA() == TServerSocket::Class()) {
      TSocket *sock = ((TServerSocket*)s)->Accept();
      fMon->Add(sock);
      fSockets->Add(sock);
      printf("accepted connection from %sn", sock->GetInetAddress().GetHostName());
      continue;
    }
    if (s->Recv(mess) <= 0) {
      fMon->Remove(s);
      fSockets->Remove(s);
      printf("closed connection from %sn", s->GetInetAddress().GetHostName());
      delete s;
      continue;
    }
    if (mess->What() == kMESS_STRING) {
      char str[64];
      mess->ReadString(str, 64);
      printf("Client: %s - %s\n", s->GetInetAddress().GetHostName(), str);
      fMon->Remove(s);
      if (fMon->GetActive() == 0) {
	printf("No more active clients... stopping\n");
	break;
      }
    } else if (mess->What() == kMESS_OBJECT) {
      auto a = mess->ReadObject(mess->GetClass());
      if(mess->GetClass()->InheritsFrom("TCanvas")){
	h=(TCanvas *)a;
	if (h) {           
	  printf("hname = %s\n", h->GetName());
	  std::string name = h->GetName();
	  std::vector<std::string> v;
	  std::stringstream sin(name);
	  std::string buffer;
	  while( std::getline(sin, buffer, '/') ) {
	    v.push_back(buffer);
	  }
	  std::vector<std::string> s = v;
	  if (s.size() > 1) {
	    h->SetName(s[1].c_str());
	    s[0] = "/" + s[0];
	    std::cout << s[0] << std::endl;
	    m_serv->Register(s[0].c_str(), h);
	  }
	  h->Draw();  // draw canvas
	}
        }
    } else {
      printf("*** Unexpected message ***\n");
    }
    delete mess;
  }
  
  fSockets->Delete();
  delete fSockets;
  delete ss;
  delete fMon;
}
