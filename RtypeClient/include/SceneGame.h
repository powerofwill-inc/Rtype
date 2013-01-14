#pragma once

#include "IScene.h"
#include "ASceneHover.h"

#include "Drawable.h"
#include "DrawableGeneric.h"
#include "DrawerDrawable.h"
#include "SceneManager.h"
#include "../../protocol.h"

#include "DrawerShip.h"
#include "DrawerEnemyBasic.h"
#include "Drawer2Bars.h"
#include "SceneHoverMenu.h"
#include "SceneHoverConfirmLeave.h"

class SceneGame :
public ASceneHover
{
 public:
  SceneGame(IScene&);
  ~SceneGame(void);
  IScene* update(float elapsedTime);
  void draw();
  void load();
  void unload();
  void setToBackground();
  void setToForeground();
 private:
IScene* manageInput();

  DrawerShip* ship[4]; // these are drawables updatable from a protocol::drawable.
  DrawerEnemyBasic enemy;
  Drawer2Bars drawer_2bars;
};

