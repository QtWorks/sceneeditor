/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SESCENEVIEW_H__
#define __SESCENEVIEW_H__

#pragma once

// SceneEditor
#include <SeScene.h>
#include <SeSceneItem.h>

// Qt
#include <QWidget>
#include <QGraphicsView>

/**
 * @brief The SeSceneView class
 */
class SeSceneView 
  : public QGraphicsView
{
  Q_OBJECT
public:
  SeSceneView(QWidget *parent=0);
  ~SeSceneView();

  SeScene *seScene() const { return mpScene; }
  
private:
  SeScene *mpScene;
};



#endif // SESCENEVIEW_H

