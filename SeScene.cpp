/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeGeneral.h>
#include <SeScene.h>
#include <SeSceneLed.h>
#include <SeMosaicWindow.h>

// Qt
#include <QDebug>
#include <QPainter>
#include <QTransform>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

SeScene::SeScene(QObject *parent) 
  : QGraphicsScene(parent)
{ }

SeScene::~SeScene()
{ }

SeSceneLayer *SeScene::addLayer(int rows, int columns)
{
  SeSceneLayer *p = new SeSceneLayer(rows, columns);
  p->initialize<SeSceneLed>();
  
  this->addItem(p);
  
  return p;
}

SeSceneLayer* SeScene::layer(const QString & identifier)
{
  for(QGraphicsItem *p : this->items())
  {
    SE_CONT4NULL(p);
      
    SeSceneLayer *pl = dynamic_cast<SeSceneLayer*>(p);    
    SE_CONT4NULL(pl);    
    
    if(pl->identifier() == identifier)
    {
      return pl;
    }
  }
  
  return NULL;
}

int SeScene::sceneLayers(QList<SeSceneLayer*> & layers, SeSceneItem *parent)
{
  return this->sceneItems<SeSceneLayer>(layers, parent);
}

int SeScene::sceneLeds(QList<SeSceneLed*> &leds, SeSceneItem *parent)
{
  return this->sceneItems<SeSceneLed>(leds, parent);
}

int SeScene::selectedLeds(QList<SeSceneLed*> &leds)
{
  return this->sceneItems<SeSceneLed>(leds);

  for(QGraphicsItem *p : this->selectedItems())
  {
    SE_CONT4NULL(p);
    SeSceneLed *pled = dynamic_cast<SeSceneLed*>(p);
    SE_CONT4NULL(pled);
    leds.append(pled);
  }
  return leds.count();
}

void SeScene::exportLayer(SeSceneLayer *ptrLayer, const QString &filepath)
{
  this->hideAllLayer();
  
  ptrLayer->show();
  
  SeSceneItem *firstItem = ptrLayer->sceneItem(0, 0);
  
  int w = ptrLayer->numberOfColumns() * firstItem->width();
  int h = ptrLayer->numberOfRows() * firstItem->height();
  
  QImage image(w, h, QImage::Format_RGB32);
  
  QPainter painter;
  painter.begin(&image);
  painter.setBrush(QColor(255, 255, 255));
  painter.setPen(QColor(255, 255, 255));
  painter.drawRect(0, 0, w, h);
  this->render(&painter, QRectF(0, 0, w, h), QRectF(0, 0, w, h));
  painter.end();
  
  if(filepath.isEmpty())
  {  
    #ifdef WIN32
      image.save(QString("C:/temp/exports/%1.png").arg(ptrLayer->identifier()));
    #else
      image.save(QString("~/exports/%1.png").arg(ptrLayer->identifier()));
    #endif
  }
  else
  {
    image.save(filepath);
  }
}

void SeScene::hideAllLayer()
{
  QList<SeSceneLayer*> layers;
  /*int n =*/ this->sceneLayers(layers);
  
  for(SeSceneLayer *ptr : layers)
  {
    SE_CONT4NULL(ptr);
    ptr->hide();
  }
}

bool SeScene::showLayer(const QString & identifier)
{
  SeSceneLayer *p = this->layer(identifier);
  if(p == NULL) { return false; }
  
  QList<SeSceneLayer*> layers;
  
  int n = this->sceneLayers(layers);  
  if(n == 0) { return false; }
  
  for(auto pitem : layers)
  {
    pitem->hide();
  }
  
  p->show();
  
  return true;
}

void SeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem *p = this->itemAt(event->scenePos(), QTransform());

  if(p != NULL)
  {
    SeSceneItem *pit = dynamic_cast<SeSceneItem*>(p);
    if(pit != NULL)
    {
      emit this->item(pit);
    }
  }

  QGraphicsScene::mouseReleaseEvent(event);
}
