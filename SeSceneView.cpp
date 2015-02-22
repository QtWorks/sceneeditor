/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeSceneLayer.h>
#include <SeSceneView.h>
#include <SeScene.h>

// Qt
#include <QDebug>
#include <QGLWidget>
#include <QGraphicsView>

SeSceneView::SeSceneView(QWidget *parent)
  : QGraphicsView(parent)
  , mpScene(NULL)
{
  this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
  this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  this->setCacheMode(QGraphicsView::CacheNone);    
  this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  this->translate(0.f, 0.f);

  this->setSceneRect(0, 0, DEFAULT_COLUMNS * 50, DEFAULT_ROWS * 50);
  
  mpScene = new SeScene();
  
  this->setScene(mpScene);
}

SeSceneView::~SeSceneView()
{
  
}



