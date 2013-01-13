#include "DrawerMenu.h"

DrawerMenu::DrawerMenu() : arrow(DrawableGeneric(*FactoryDrawable::getInstance()->createSelectionArrow()))
{
  selectionId = 0;
  addButton();
  std::list<ValueDrawer > arrowAnimation;
  arrowAnimation.push_back({{0,0},{1,1}, 0});
  arrowAnimation.push_back({{10, 0},{1,1},0});
  arrowAnimation.push_back({{0,0},{1,1}, 0});
  arrow.setAnimation((new Animation<ValueDrawer>(arrowAnimation, 500, true)));
  updateArrowPosition();
}

void DrawerMenu::drawTo(IGraphicsManager* gm) const
{
  for (const ADrawer& d : buttons)
    d.drawTo(gm, this->getModifiedValue());
  arrow.drawTo(gm, this->getModifiedValue());
}

void DrawerMenu::drawTo(IGraphicsManager* gm, const ValueDrawer& modifier) const
{
  for (const DrawerUDrawable& d : buttons)
    d.drawTo(gm, this->getModifiedValue() + modifier);
  arrow.drawTo(gm, this->getModifiedValue() + modifier);
}

void DrawerMenu::update(float ms)
{
  ADrawer::update(ms);
  SceneManager* sm = SceneManager::getInstance();
  const Vector2<bool>& returnKey(sm->getInputManager()->getKeyStatus(sf::Keyboard::Return));
  if (!returnKey.x && returnKey.y)
    {
      buttons[selectionId].animate(1);
      status = selectionType::SELECTED;
    }
  else if (returnKey.x && !returnKey.y)
    {
      status = selectionType::VALIDATED;
    }

  const Vector2<bool>& downKey(sm->getInputManager()->getKeyStatus(sf::Keyboard::Down));
  if (!downKey.x && downKey.y)
    {
      selectionId++;
      status = selectionType::HOVER;
      if (selectionId >= buttons.size())
	selectionId = 0;
    }
  const Vector2<bool>& upKey(sm->getInputManager()->getKeyStatus(sf::Keyboard::Up));
  if (!upKey.x && upKey.y)
    {
      selectionId--;
      status = selectionType::HOVER;
      if (selectionId < 0)
	selectionId = buttons.size() - 1;
    }
  updateArrowPosition();
  for (DrawerUDrawable& d : buttons)
    d.update(ms);
  arrow.update(ms);
}

void DrawerMenu::addButton()
{  
  buttons.push_back(DrawerUDrawable(DrawableGeneric(*FactoryDrawable::getInstance()->createButton())));
  buttons.back().setInitialValue({{30, (buttons.size() - 1) * 42}, {1, 1}, 0});
}

int DrawerMenu::getSelectedId() const
{
  return selectionId;
}

DrawerMenu::selectionType DrawerMenu::getSelectionType() const
{
  return status;
}

void DrawerMenu::updateArrowPosition()
{
  arrow.setInitialValue({{0, (selectionId) * 42}, {1, 1}, 0});
}
