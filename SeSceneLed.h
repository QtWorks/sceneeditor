/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SESCENELED_H__
#define __SESCENELED_H__

#pragma once

// SceneEditor
#include <SeSceneItem.h>

// Qt
#include <QColor>
#include <QPixmap>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

// forward-declaration
class SeSceneLed;
class SeSceneItemProperties;

/**
 * @brief The SeSceneLed class
 */
class SeSceneLed 
  : public SeSceneItem
{
public:
  SeSceneLed(SeSceneItem *parent=0);
  ~SeSceneLed();

  friend class SeSceneItemProperties;

protected:
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);  
};

#endif // __SESCENELED_H__
