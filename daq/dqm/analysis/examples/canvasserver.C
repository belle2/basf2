{
   // Open a server socket looking for connections on a named service or
   // on a specified port.
   //TServerSocket *ss = new TServerSocket("rootserv", kTRUE);
   TServerSocket *ss = new TServerSocket(9191, kTRUE);
   THttpServer *m_serv = new THttpServer("http:8081");
   TMonitor *fMon = new TMonitor;
   TList    *fSockets = new TList;   // list of open spy sockets
   
   fMon->Add(ss);
   TCanvas *h = 0;
   TH1 * hh;
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
        // delete canvas .... seems to be done automatically .. TODO check for memory leak!!!
        auto a=mess->ReadObject(mess->GetClass());
        if(mess->GetClass()->InheritsFrom("TCanvas")){
          h=(TCanvas *)a;
          if(h) printf("===== recv object %s ====\n",h->GetName());
          gROOT->GetListOfCanvases()->ls();
          if (h) {           
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
   delete fMon
}
