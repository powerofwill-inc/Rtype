#include <sstream>
#include "WSocket.hpp"
#include "WServerSocket.hpp"

int CALLBACK conditionAcceptFunc(
    LPWSABUF lpCallerId,
    LPWSABUF lpCallerData,
    LPQOS pQos,
    LPQOS lpGQOS,
    LPWSABUF lpCalleeId,
    LPWSABUF lpCalleeData,
    GROUP FAR * g,
    DWORD_PTR dwCallbackData
    )
{
	return CF_ACCEPT;
}

void	WServerSocket::addNewPeer(void * peer)
{
	std::cout << "new Client !" << std::endl;
	static int id = 0;
	ISocket * acc = this->myaccept(peer);

	ISocket * sockUdp = new WSocket();
	sockUdp->setUDP(true);
	std::ostringstream convert;
	convert << this->_portUdpPeer;

	// TODO: a decommenter
	sockUdp->connectToServer(acc->getHost(), "4246");
	
//	sockUdp->connectToServer(acc->getHost(), convert.str());
//	int * header = new int[2];
//	Protocol::portUdp portNo;
//	portNo.port = this->_portUdpPeer;
//	header[0] = Protocol::PORT_UDP;
//	header[1] = sizeof(portNo);
//	void * pckg = new char[(2 * sizeof(int)) + sizeof(portNo)];
//	memcpy(pckg, header, (2 * sizeof(int)));
//	memcpy(((char *)pckg) + (2 * sizeof(int)), &portNo, sizeof(portNo));
//	acc->sendv((2 * sizeof(int)) + sizeof(portNo), pckg);
//	this->_portUdpPeer++;

	this->_server->createPlayerWaiting(id, acc->getHost(), acc, sockUdp);

	this->_clientsList.push_back(((WSocket *)(acc))->getSocket());
	this->_clientsSocksMap[((WSocket *)(acc))->getSocket()] = acc;

	// TODO : voir si select sur l'udp est clean
	this->_clientsList.push_back(((WSocket *)(sockUdp))->getSocket());
	this->_clientsSocksMap[((WSocket *)(sockUdp))->getSocket()] = sockUdp;
	this->_clientsSocksUdpMap[sockUdp->getHost()] = sockUdp;
	id++;
}

int		WServerSocket::selectSockets()
{
	int		nbSocksReady;
	int		maxFd = 0;

	FD_ZERO(&_readFd);
	for (std::list<SOCKET>::iterator it = _clientsList.begin(); it != _clientsList.end(); it++)
		FD_SET((*it), &_readFd);
	if ((nbSocksReady = select(maxFd + 1, &_readFd, NULL, NULL, NULL)) == -1)
	{
		std::cerr << "Server select() error: " << WSAGetLastError() << std::endl;
		return -1;
	}
	return nbSocksReady;
}

void	WServerSocket::callBack(std::list<SOCKET>::iterator & it)
{
  ISocket * tmp = _clientsSocksMap[*it];
  void	* data = 0;
  void	* header = 0;

  if (tmp->isUDP() == true)
  {
	  header = new int[2];
	  ((int*)header)[1] = sizeof(Protocol::cmd_client);
  }
  if (!tmp->recv(&header, &data))
	{
		  std::cout << "client disconnected" << std::endl;
		  this->_interPckg->executeCmd(0, 0, tmp);
		  it = _clientsList.erase(it);
		  _clientsSocksUdpMap.erase(tmp->getHost());
		  _clientsSocksMap.erase(((WSocket*)(tmp))->getSocket());
		  delete tmp;
		  it--;
		  return ;
	}

	if (tmp->isUDP() == true)
		tmp = this->_clientsSocksUdpMap[tmp->getHost()];
  this->_interPckg->executeCmd(header, data, tmp);
}

void	WServerSocket::launch()
{
	std::cout << "server launched !" << std::endl;
	_clientsList.push_back(_listenSocketTcp);
	_clientsList.push_back(_listenSocketUdp);

	while (true)
	{
		int nbSocksReady = this->selectSockets();
		for (std::list<SOCKET>::iterator it = _clientsList.begin(); nbSocksReady > 0 && it != _clientsList.end(); it++)
			if (FD_ISSET((*it), &_readFd))
			{
				nbSocksReady--;
				if ((*it) == _listenSocketTcp)
					this->addNewPeer(&(*it));
				else
					this->callBack(it);
			}
	}
}

ISocket	* WServerSocket::myaccept(void * sockType)
{
	SOCKET				acceptSock;
	sockaddr_in			saClient;
	int					clientSize = sizeof(saClient);


	acceptSock = WSAAccept(
		(*((SOCKET*)sockType)),
		(SOCKADDR*) &saClient,
		&clientSize,
		&conditionAcceptFunc,
		NULL
	);
    if (acceptSock == INVALID_SOCKET) {
        std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
        return NULL;
    }

	ISocket * ret = new WSocket();
	ret->connectFromAcceptedFd(&acceptSock);
	ret->setHost(inet_ntoa(saClient.sin_addr));
	return ret;
}

bool	WServerSocket::init(std::string const & listenHost, std::string const & listenPort)
{
	int					i;
	std::stringstream	ss;
	int					port;
	WSADATA				wsd;

	if ((i = WSAStartup(MAKEWORD(2, 2), &wsd)))
	{
        std::cerr << "Unable to load Winsock: " << i << std::endl;
        return false;
	}
	ss << listenPort;
	ss >> port;
	this->_portUdpPeer = port + 1;
	ss.str("");
	ss.clear();
	// ========= TCP =========
  memset((char *) &(this->_servAddr), 0, sizeof(this->_servAddr));
  this->_servAddr.sin_family = AF_INET;
  this->_servAddr.sin_addr.s_addr = INADDR_ANY;
  this->_servAddr.sin_port = htons(port);
  this->_listenSocketTcp = socket(AF_INET, SOCK_STREAM, 0);
  if (this->_listenSocketTcp <= 0)
    {
      std::cerr << "socket failed with error (TCP)" << std::endl;
      return false;
    }
  i = bind( this->_listenSocketTcp, (struct sockaddr *) &this->_servAddr, sizeof(this->_servAddr));
  if (i < 0)
    {
      std::cerr << "bind failed with error (TCP): " << std::endl;
      closesocket(this->_listenSocketTcp);
      return false;
    }
  if (listen(_listenSocketTcp, 5) < 0)
    {
      std::cerr << "listen failed with error (TCP): " << std::endl;
      return false;
    }


	this->_servAddr.sin_port = htons(4245);
//  this->_servAddr.sin_port = htons(this->_portUdpPeer);
 // this->_portUdpPeer++;
	this->_listenSocketUdp = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->_listenSocketUdp <= 0)
    {
      std::cerr << "socket failed with error (TCP)" << std::endl;
      return false;
    }
  i = bind( this->_listenSocketUdp, (struct sockaddr *) &this->_servAddr, sizeof(this->_servAddr));
  if (i < 0)
    {
      std::cerr << "bind failed with error (TCP): " << std::endl;
      closesocket(this->_listenSocketUdp);
      return false;
    }
  ISocket *udp = new WSocket();
  udp->setUDP(true);
  udp->connectFromAcceptedFd(&_listenSocketUdp);
  this->_clientsSocksMap[((WSocket *)(udp))->getSocket()] = udp;
	return true;
}

WServerSocket::~WServerSocket()
{
	if (this->_listenSocketTcp != INVALID_SOCKET)
		closesocket(this->_listenSocketTcp);
	WSACleanup();
}

WServerSocket::WServerSocket(Server * s)
{
	this->_interPckg = new InterpretPackage(s);
	this->_listenSocketTcp = INVALID_SOCKET;
	this->_listenSocketUdp = INVALID_SOCKET;
	this->_server = s;
}
