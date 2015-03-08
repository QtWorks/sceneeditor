/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#pragma once

#ifndef __SESCENE_H__
#define __SESCENE_H__

// Qt
#include <QObject>
#include <QStatusBar>
#include <QGraphicsScene>
#include <QSharedPointer>

// SceneEditor
#include <SeGeneral.h>
#include <SeSceneItem.h>
#include <SeSceneLayer.h>

namespace SceneEditor 
{
  extern QStatusBar *__statusBar;
}

/**
 * @brief The SeScene class
 */
class SeScene 
  : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit SeScene(QObject *parent = 0);
  ~SeScene();
  
  SeSceneLayer* addLayer(int rows, int columns);
  
  SeSceneLayer* layer(const QString & identifier);      
  
  int sceneLayers(QList<SeSceneLayer*> & layers, SeSceneItem *parent=0);
  int sceneLeds(QList<SeSceneLed*> & leds, SeSceneItem *parent=0);    
  template<class T> int sceneItems(QList<T*> & itemlist, SeSceneItem *parent=NULL) {
    for(QGraphicsItem *p : this->items()) {
      SE_CONT4NULL(p); T *pl = dynamic_cast<T*>(p);
      if(parent != NULL && p->parentItem() != parent) { continue; }
      SE_CONT4NULL(pl); itemlist.append(pl);
    }
    return itemlist.count();
  }
  
  bool showLayer(const QString & identifier);
  
  int selectedLeds(QList<SeSceneLed*> & leds);
     
  //! \brief ...
  //! \param ptrLayer ...
  //! \param filepath Default path is "C:\temp\exports\" on Windows
  //!                 and "~/exports/ on Unix-based systems.
  void exportLayer(SeSceneLayer *ptrLayer, const QString & filepath="");
  
  void hideAllLayer();
     
protected:
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
           
signals:
  void item(SeSceneItem*);
  
public slots:
};

#endif // __SESCENE_H__
