//
// InterpretPackage.cpp for  in /home/marche_m/workspace/tek3/cpp/r-type/Rtype/server/src
// 
// Made by marche_m (Maxime Marchès)
// Login   <marche_m@epitech.net>
// 
// Started on  Wed Jan  9 10:54:24 2013 marche_m (Maxime Marchès)
// Last update Tue Jan 15 22:25:26 2013 mathieu leurquin
//

#include "../include/InterpretPackage.hpp"

InterpretPackage::InterpretPackage(Server *s)
{
  _funcMap[Protocol::REGISTER] = &InterpretPackage::execRegister;
  _funcMap[Protocol::LOGIN] = &InterpretPackage::execLogin;
  _funcMap[Protocol::GET_GAME_LIST] = &InterpretPackage::execGetGameList;
  _funcMap[Protocol::JOIN_GAME] = &InterpretPackage::execJoinGame;
  _funcMap[Protocol::CREATE_GAME] = &InterpretPackage::execCreateGame;
  //_funcMap[Protocol::MOVE] = &InterpretPackage::execMove;
  //_funcMap[Protocol::FIRE] = &InterpretPackage::execFire;
  _server = s;
}

void	InterpretPackage::executeCmd(void * header, void * data, ISocket * sock)
{
  if (sock->isUDP() == true)
    {
      std::cout<<"receive something"<<std::endl;
      Protocol::cmd_client * cmd = ((Protocol::cmd_client*)data);
      if (cmd->fire == true)
	this->execFire(data, sock);
      else
	{
	  std::cout<<"move"<<std::endl;
	  Protocol::move * m = new Protocol::move;
	  if (cmd->top == true)
	    m->top = 1;
	  else if (cmd->down == true)
	    m->down = 1;
	  else if (cmd->left == true)
	    m->left = 1;
	  else if (cmd->right == true)
	    m->right = 1;
	  this->execMove(m, sock);
	}
      return ;
    }
  std::cout << "executeCmd" << std::endl;
  int hdTmp[2];
  
  memcpy(hdTmp, header, 2 * sizeof(int));
  std::cout << "id:" << hdTmp[0] << std::endl;
  std::cout << "size:" << hdTmp[1] << std::endl;
  void (InterpretPackage::*pMethod)(void *, ISocket *) = (this->_funcMap[((Protocol::type_cmd)hdTmp[0])]);
  (this->*pMethod)(data, sock);
}

void	InterpretPackage::execRegister(void * data, ISocket * sock)
{

}

void	InterpretPackage::execLogin(void * data, ISocket * sock)
{
  std::cout << "execLogin" << std::endl;
  Protocol::login * dataTmp = new Protocol::login();
  memset(dataTmp, 0, sizeof(*dataTmp));
  memcpy(dataTmp, data, sizeof(*dataTmp));
  std::cout << dataTmp->login << std::endl;
  std::cout << dataTmp->passwd << std::endl;
}

void	InterpretPackage::execGetGameList(void * data, ISocket * sock)
{
  void	*res;
  int	size = 0;
  int position = 0;

  Protocol::package *p = new Protocol::package();
  p->id = Protocol::GET_GAME_LIST;
  
  size = sizeof(Protocol::parties*) + (_server->getNbGame() * sizeof (Protocol::party*));
  p->size = size;

  memset(res, 0, size);
  memcpy(&res, p, sizeof(Protocol::package*));
  position += sizeof(Protocol::package*);
  
  Protocol::parties *pa = new Protocol::parties();
  pa->nb_parties = _server->getNbGame();
  memcpy(&res + position, pa, sizeof(Protocol::parties*));
  position = sizeof(Protocol::parties*);

  std::list <Game*> listGameCpy = _server->gameList;
  for (std::list<Game*>::iterator it = listGameCpy.begin(); it != listGameCpy.end(); it++)
    {
      Protocol::party *pa = new Protocol::party();
      pa->nb_players = (*it)->getNbPlayer();
      memcpy(&res + position, pa, sizeof(Protocol::party*));
      position += sizeof(Protocol::party*);
    }
  sock->sendv(p->size, res);
}

void	InterpretPackage::execJoinGame(void * data, ISocket * sock)
{
  std::list <Game*> listGameCpy = _server->gameList;
  Protocol::join_game *game = new Protocol::join_game();

  memset(game, 0, sizeof(Protocol::join_game *));
  memcpy(game, data, sizeof(Protocol::join_game *));
  for (std::list<Game*>::iterator it = listGameCpy.begin(); it != listGameCpy.end(); it++)
    {
      Protocol::response *rep = new Protocol::response();
      if ((*it)->getId() == game->id && (*it)->getNbPlayer() < 4)
	{
	  Player *p = (*it)->getPlayerBySockUdp(sock);
	  
	  (*it)->addHumainUnitByPlayer(p);
	  _server->erasePlayerWaiting(p->getId());
	  rep->response = Protocol::VALIDE;
	  sock->sendv(sizeof(Protocol::response*), (void*)rep);
	}
      else if ((*it)->getId() == game->id && (*it)->getNbPlayer() >= 4)
 	{
	  rep->response = Protocol::CANT_JOIN_GAME;
	  sock->sendv(sizeof(Protocol::response*), (void*)rep);
	}
    }
}

void	InterpretPackage::execCreateGame(void * data, ISocket * sock)
{

}

void	InterpretPackage::execMove(void * data, ISocket * sock)
{
  Player *p;
  std::list <Game*> listGameCpy = _server->gameList;
  
  Protocol::move *m= (Protocol::move*)data;
  for (std::list<Game*>::iterator it = listGameCpy.begin(); it != listGameCpy.end(); it++)
    {
      if ((p = (*it)->getPlayerBySockUdp(sock)) != NULL)
	{
	  if ((*it)->getUnitByPlayer(p)->getHealth() > 0)
	    {
	      (*it)->move(p->getId(), m);
	      Protocol::response *rep = new Protocol::response();
	      rep->response = Protocol::VALIDE;
	      sock->sendv(sizeof(Protocol::response*), (void*)rep);
	    }
	}
    }
}

void	InterpretPackage::execFire(void * data, ISocket * sock)
{
  Player *p;
  std::list <Game*> listGameCpy = _server->gameList;
  
  for (std::list<Game*>::iterator it = listGameCpy.begin(); it != listGameCpy.end(); it++)
    {
      if ((p = (*it)->getPlayerBySockUdp(sock)) != NULL)
	{
	  if ((*it)->getUnitByPlayer(p)->getHealth() > 0)
	    {
	      (*it)->fire(p->getId());
	      Protocol::response *rep = new Protocol::response();
	      rep->response = Protocol::VALIDE;
	      sock->sendv(sizeof(Protocol::response*), (void*)rep);
	    }
	}
    }
}
