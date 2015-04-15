#include "modbustcp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

#define FUNC_COIL_READ        0x01
#define FUNC_INPUT_READ       0x02
#define FUNC_REG_READ         0x03
#define FUNC_INPUTREG_READ    0x04
#define FUNC_COIL_WRITE       0x05
#define FUNC_REG_WRITE        0x06
#define FUNC_MULTI_COIL_WRITE 0x0F
#define FUNC_MULTI_REG_WRITE  0x10
#define FUNC_RECORD_READ      0x14
#define FUNC_RECORD_WRITE     0x15
#define FUNC_REGMASK_WRITE    0x16
#define FUNC_MULTI_REG_RDWR   0x17
#define FUNC_FIFO_READ        0x18
#define FUNC_ERR              0x80

int modbus_connect(const char* hostname, int port)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  int fd = 0;
  if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Failed to create socket\n");
    return -1;
  }
  struct hostent* host = NULL;
  host = gethostbyname(hostname);
  if (host == NULL) {
    unsigned long ip_address = inet_addr(hostname);
    if ((signed long) ip_address < 0) {
      printf("Wrong host name or ip\n");
      return -1;
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  if (host == NULL) {
    close(fd);
    printf("Failed to connect host %s:%d\n", hostname, port);
    return -1;
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(fd);
    printf("Failed to connect host %s:%d\n", hostname, port);
    return -1;
  }
  return fd;
}

int modbus_write_socket(int fd, const void* buf, int count)
{
  int c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = send(fd, ((unsigned char*)buf + c), (count - c), MSG_NOSIGNAL);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          return 0;
      }
    }
    c += ret;
  }
  return c;
}

int modbus_read_socket(int fd, void* buf, int count)
{
  int c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = recv(fd, ((unsigned char*)buf + c), (count - c), 0);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          return 0;
      }
    }
    c += ret;
  }
  return c;
}

int modbus_select(int fd, int timeout)
{
  if (fd <= 0) return -1;
  if (timeout < 0) return 1;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  int ret;
  struct timeval t = {timeout, 0};
  ret = select(FD_SETSIZE, &fds, NULL, NULL, &t);
  if (ret < 0) {
    perror("select");
    return -2;
  }
  if (FD_ISSET(fd, &fds)) {
    return 1;
  } else {
    return 0;
  }
}

int modbus_send(int fd, const modbus_adu_t* data)
{
  const int size = sizeof(modbusap_header_t) - 1 + data->hdr.length;
  return modbus_write_socket(fd, data, size) > 0;
}

int modbus_recv(int fd, modbus_adu_t* data)
{
  int ret = 0, count = 0;
  if ((ret = modbus_read_socket(fd, &(data->hdr), 
				sizeof(modbusap_header_t))) == 0) {
    return 0;
  }
  count += ret;
  const int size = data->hdr.length - 1;
  if ((ret = modbus_read_socket(fd, &(data->pdu), size)) == 0) {
    return 0;
  }
  count += ret;
  return count;
}

int modbus_send_recv(int fd, const modbus_adu_t* data_in,
		modbus_adu_t* data_out, int timeout)
{
  if (modbus_send(fd, data_in) == 0) {
    printf("%s Faield to send\n", __PRETTY_FUNCTION__);
    return 0;
  }
  if (modbus_select(fd, timeout) <= 0) {
    printf("%s Faield to select\n", __PRETTY_FUNCTION__);
    return 0;
  }
  if (modbus_recv(fd, data_out) == 0) {
    printf("%s Faield to recv\n", __PRETTY_FUNCTION__);
    return 0;
  }
  if (data_out->pdu.func_code > 0x80) {
    printf("Response error : 0x%x", data_out->pdu.func_code);
    switch (data_out->pdu.data[0]) {
    case 0x01: printf("Function code not exists : %d", 
		      data_in->pdu.func_code);
      break;
    case 0x02: printf("Out of address range");
      break;
    case 0x03: printf("Out of range");
      break;
    case 0x04: printf("Write protected");
      break;
    case 0x0a: printf("Illigal unit id : %d", data_out->hdr.unit_id);
      break;
    case 0x0b: printf("cpu not exsists for unit id : %d", 
		      data_out->hdr.unit_id);
      break;
    }
    return -1;
  }
  return 1;
}

int modbus_coil_read(int fd, modbus_adu_t* data_in, 
		     unsigned short top_coilid, 
		     unsigned short nread,
		     unsigned char* ndata, 
		     unsigned char* pdata, 
		     int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_COIL_READ;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = top_coilid;
  data[1] = nread;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    *ndata = 0;
    return err;
  }
  *ndata = data_out.pdu.data[0];
  int i = 0;
  for (; i < *ndata; i++) {
    pdata[i] = data_out.pdu.data[i+1];
  }
  return 1;
}

int modbus_input_read(int fd, modbus_adu_t* data_in, 
		     unsigned short top_coilid, 
		     unsigned short nread,
		     unsigned char* ndata, 
		     unsigned char* pdata, 
		     int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_INPUT_READ;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = top_coilid;
  data[1] = nread;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    *ndata = 0;
    return err;
  }
  *ndata = data_out.pdu.data[0];
  int i = 0;
  for (; i < *ndata; i++) {
    pdata[i] = data_out.pdu.data[i+1];
  }
  return 1;
}

int modbus_reg_read(int fd, modbus_adu_t* data_in, 
		    unsigned short regid, 
		    unsigned short nread,
		    unsigned char* ndata, 
		    unsigned short* pdata, 
		    int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_REG_READ;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = regid;
  data[1] = nread;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    *ndata = 0;
    return err;
  }
  *ndata = data_out.pdu.data[0];
  data = (unsigned short*)(data_out.pdu.data + 1);
  int i = 0;
  for (; i < *ndata / 2; i++) {
    pdata[i] = data[i];
  }
  return 1;
}

int modbus_inputreg_read(int fd, modbus_adu_t* data_in, 
			 unsigned short regid, 
			 unsigned short nread,
			 unsigned char* ndata, 
			 unsigned short* pdata, 
			 int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_INPUTREG_READ;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = regid;
  data[1] = nread;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    *ndata = 0;
    return err;
  }
  *ndata = data_out.pdu.data[0];
  data = (unsigned short*)(data_out.pdu.data+1);
  int i = 0;
  for (; i < *ndata / 2; i++) {
    pdata[i] = data[i];
  }
  return 1;
}

int modbus_coil_write(int fd, modbus_adu_t* data_in, 
		      unsigned short coilid, 
		      unsigned short enabled, 
		      int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_COIL_WRITE;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = coilid;
  enabled = (enabled > 0)?0xFF00:0x0000;
  data[1] = enabled;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned short*)(data_out.pdu.data);
  if (data[0] != coilid) {
    printf("%s inconsitent coilid : %d!=%d", __FUNCTION__,
	   (int)coilid, (int)data[0]);
    return -2;
  }
  if (data[1] != enabled) {
    printf("%s inconsitent enabled : 0x%04x!=0x%04x", __FUNCTION__,
	   (int)enabled, (int)data[0]);
    return -3;
  }
  return 1;
}

int modbus_reg_write(int fd, modbus_adu_t* data_in, 
		     unsigned short regid, 
		     unsigned short wdata, 
		     int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 5;
  data_in->pdu.func_code = FUNC_REG_WRITE;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = regid;
  data[1] = wdata;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned short*)(data_out.pdu.data);
  if (data[0] != regid) {
    printf("%s inconsitent regid : %d!=%d", __FUNCTION__,
	   (int)regid, (int)data[0]);
    return -2;
  }
  if (data[1] != wdata) {
    printf("%s inconsitent wdata : 0x%04x!=0x%04x", __FUNCTION__,
	   (int)wdata, (int)data[0]);
    return -3;
  }
  return 1;
}

int modbus_muticoil_write(int fd, modbus_adu_t* data_in, 
			  unsigned short topcoilid, 
			  unsigned short nwdata,
			  unsigned char* wdata, 
			  int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->pdu.func_code = FUNC_MULTI_COIL_WRITE;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = topcoilid;
  data[1] = nwdata;
  unsigned int size = (nwdata % 8 == 0)? nwdata / 8: nwdata / 8 + 1;
  data_in->pdu.data[4] = size;
  memcpy(&(data_in->pdu.data[5]), wdata, size);
  data_in->hdr.length= 1 + 5 + size;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned short*)(data_out.pdu.data);
  if (data[0] != topcoilid) {
    printf("%s inconsitent regid : %d!=%d", __FUNCTION__,
	   (int)topcoilid, (int)data[0]);
    return -2;
  }
  if (data[1] != nwdata) {
    printf("%s inconsitent nwdata : 0x%04x!=0x%04x", __FUNCTION__,
	   (int)nwdata, (int)data[0]);
    return -3;
  }
  return 1;
}

int modbus_mutireg_write(int fd, modbus_adu_t* data_in, 
			 unsigned short topcoilid, 
			 unsigned short nwdata,
			 unsigned short* wdata, 
			  int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->pdu.func_code = FUNC_MULTI_REG_WRITE;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = topcoilid;
  data[1] = nwdata;
  unsigned int size = nwdata * 2;
  data_in->pdu.data[4] = size;
  memcpy(&(data_in->pdu.data[5]), wdata, size);
  data_in->hdr.length = 1 + 5 + size;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned short*)(data_out.pdu.data);
  if (data[0] != topcoilid) {
    printf("%s inconsitent regid : %d!=%d", __FUNCTION__,
	   (int)topcoilid, (int)data[0]);
    return -2;
  }
  if (data[1] != nwdata) {
    printf("%s inconsitent nwdata : 0x%04x!=0x%04x", __FUNCTION__,
	   (int)nwdata, (int)data[0]);
    return -3;
  }
  return 1;
}

int modbus_record_read(int fd, modbus_adu_t* data_in, 
		       int nreq,
		       modbus_req_record_t* req,
		       unsigned int* nrecord,
		       modbus_rrecord_t* record,
		       int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->pdu.func_code = FUNC_RECORD_READ;
  unsigned char* data = (unsigned char*)data_in->pdu.data;
  unsigned int size = nreq *sizeof(modbus_req_record_t);
  data[0] = size;
  int i = 0;
  for (i = 0; i < nreq; i++) {
    memcpy(data+1+i*sizeof(modbus_req_record_t), &req[i], 
	   sizeof(modbus_req_record_t));
  }
  data_in->hdr.length = 1 + 2 + size;
  modbus_adu_t data_out;
  int err;
  *nrecord = 0;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned char*)(data_out.pdu.data);
  int nbyte = data[0];
  for (i = 1; i < nbyte;) {
    record[*nrecord].nbyte = data[i++];
    record[*nrecord].ref = data[i++];
    memcpy(record[*nrecord].data, &(data[i]), record[*nrecord].nbyte -1);
    i += record[*nrecord].nbyte - 1;
    (*nrecord)++;
  }
  return *nrecord;
}

int modbus_record_write(int fd, modbus_adu_t* data_in, 
			int nreq, modbus_req_record_t* req,
			unsigned short** wdata,
			int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->pdu.func_code = FUNC_RECORD_WRITE;
  unsigned char* data = (unsigned char*)data_in->pdu.data;
  unsigned int size = 0;
  data++;
  int i = 0;
  for (i = 0; i < nreq; i++) {
    memcpy(data, &req[i], sizeof(modbus_req_record_t));
    data += sizeof(modbus_req_record_t);
    size += sizeof(modbus_req_record_t);
    memcpy(data, &wdata[i], req[i].nrecords * sizeof(unsigned short));
    data += req[i].nrecords * sizeof(unsigned short);
    size += req[i].nrecords * sizeof(unsigned short);
  }
  data = (unsigned char*)data_in->pdu.data;
  data[0] = size;
  data_in->hdr.length = 1 + 2 + size;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  data = (unsigned char*)(data_out.pdu.data);
  int nbyte = data[0];
  int nreq_out = 0;
  modbus_req_record_t req_out;
  size = 0;
  while (size < nbyte) {
    memcpy(&req_out, data, sizeof(modbus_req_record_t));
    data += sizeof(modbus_req_record_t);
    size += sizeof(modbus_req_record_t);
    data += req_out.nrecords * sizeof(unsigned short);
    size += req_out.nrecords * sizeof(unsigned short);
    nreq_out++;
  }
  return nreq_out;
}

int modbus_regmask_write(int fd, modbus_adu_t* data_in, 
			 modbus_regmask_t mask, 
			 int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + sizeof(mask) + 1;
  data_in->pdu.func_code = FUNC_REG_WRITE;
  memcpy(data_in->pdu.data, &mask, sizeof(mask));
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  modbus_regmask_t mask_out;
  memcpy(&mask_out, data_out.pdu.data, sizeof(modbus_regmask_t));
  if (mask.topregid != mask_out.topregid) {
    printf("%s inconsitent regid : %d!=%d", __FUNCTION__,
	   (int)mask.topregid, (int)mask_out.topregid);
    return -2;
  }
  if (mask.and_mask != mask_out.and_mask) {
    printf("%s inconsitent and_mask : %d!=%d", __FUNCTION__,
	   (int)mask.and_mask, (int)mask_out.and_mask);
    return -2;
  }
  if (mask.or_mask != mask_out.or_mask) {
    printf("%s inconsitent or_mask : %d!=%d", __FUNCTION__,
	   (int)mask.or_mask, (int)mask_out.or_mask);
    return -2;
  }
  return 1;
}

int modbus_multireg_read_write(int fd, modbus_adu_t* data_in, 
			       unsigned short rregid, 
			       unsigned short nrreg, 
			       unsigned short* rdata, 
			       unsigned short wregid, 
			       unsigned short nwreg, 
			       unsigned char  nwdata, 
			       unsigned short* wdata, 
			       int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->pdu.func_code = FUNC_MULTI_REG_RDWR;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = rregid;
  data[1] = nrreg;
  data[2] = wregid;
  data[3] = nwreg;
  data_in->pdu.data[8] = nwdata;
  data = (unsigned short*)(data_in->pdu.data+9);
  memcpy(data, wdata, nwdata);
  data_in->hdr.length= 1 + 1 + sizeof(unsigned short) * 4 + nwdata;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    return err;
  }
  if (nrreg != data_in->pdu.data[0]/2) {
    printf("%s inconsitent nrreg : %d!=%d", __FUNCTION__,
	   (int)nrreg, (int)data_in->pdu.data[0]/2);
    return -2;
  }
  data = (unsigned short*)(data_in->pdu.data+1);
  memcpy(rdata, data, nrreg*2);
  
  return 1;
}

int modbus_fifo_read(int fd, modbus_adu_t* data_in, 
		    unsigned short fifoid, 
		    unsigned short* ndata, 
		    unsigned short* pdata, 
		    int timeout)
{
  data_in->hdr.protocol= 0;
  data_in->hdr.length= 1 + 3;
  data_in->pdu.func_code = FUNC_FIFO_READ;
  unsigned short* data = (unsigned short*)data_in->pdu.data;
  data[0] = fifoid;
  modbus_adu_t data_out;
  int err;
  if ((err = modbus_send_recv(fd, data_in, &data_out, timeout)) <= 0) {
    *ndata = 0;
    return err;
  }
  data = (unsigned short*)data_out.pdu.data;
  *ndata = *data;
  data++;
  int i;
  for (i = 0; i < *ndata / 2; i++) {
    pdata[i] = data[i];
  }
  return 1;
}

