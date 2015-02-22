/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SESCENEITEM_H__
#define __SESCENEITEM_H__

#pragma once

// Qt
#include <QSize>
#include <QColor>
#include <QObject>
#include <QJsonObject>
#include <QGraphicsItem>

// forward-declaration
class SeSceneLed;
class SeSceneItem;

/**
 * @brief The SeSceneLedProperties class
 */
class SeSceneItemProperties
{
public:
  enum ShapeMode { ShapeRect=0, ShapeCircle };
  enum TransitionMode { Hard=0, Fade=1 };

  SeSceneItemProperties();
  SeSceneItemProperties(const SeSceneItemProperties & obj);
  SeSceneItemProperties & operator=(const SeSceneItemProperties & obj);

  void setOwner(SeSceneItem *p) { this->powner = p; }
  void setIdentifier(const QString & identifier);
  void setRow(int row);
  void setColumn(int column);
  void setBrushColor(QColor c);
  void setPenColor(QColor c);
  void setTransitionMode(TransitionMode mode);
  void setOriginalFilePath(const QString & filePath);
  void setOriginalPixmap(const QPixmap & pix);
  void setSize(int w, int h);
  void setSize(const QSize & size);
  void setShapeMode(ShapeMode mode);
  void setOffset(const QPoint & offset);
  void setSelectionGeometry(const QRect & rect);
  void setScale(int scale);
  void setEnabled(bool state);
  void setIndex(int index);

  QString identifier() const { return midentifier; }
  int row() const { return mrow; }
  int column() const { return mcolumn; }
  QColor brushColor() const { return mbrushcolor; }
  QColor penColor() const { return mpencolor; }
  TransitionMode transitionMode() const { return mtransitionMode; }
  QString originalFilePath() const { return mOriginalFilePath; }
  QPixmap originalPixmap() const { return mOriginalPixmap; }
  QSize size() const { return mSize; }
  int width() const { return mSize.width(); }
  int height() const { return mSize.height(); }
  ShapeMode shapeMode() const { return mShapeMode; }
  QPoint offset() const { return mOffset; }
  QRect selectionGeometry() const { return mSelectionGeometry; }
  int scale() const { return mScale; }
  bool enabled() const { return mEnabled; }
  int index() const { return mIndex; }

  friend class SeSceneItem;
  friend class SeSceneLed;
  friend class SeSceneLayer;

  void restore(const QJsonObject & obj);
  QJsonObject toJson();

private:
  QString midentifier;
  ShapeMode mShapeMode;
  int mrow;
  int mcolumn;
  QColor mbrushcolor;
  QColor mpencolor;
  TransitionMode mtransitionMode;
  QString mOriginalFilePath;
  QPixmap mOriginalPixmap;
  QSize mSize;
  QPoint mOffset;
  QRect mSelectionGeometry;
  int mScale;
  bool mEnabled;
  int mIndex;
  
  SeSceneItem *powner;
};

/**
 * @brief The SeSceneItem class
 */
class SeSceneItem 
  : public QGraphicsItem
{
public:
  explicit SeSceneItem(QGraphicsItem *parent = 0);
  ~SeSceneItem();
  
public:  
  SeSceneItemProperties & properties() { return mProperties; }
protected:
  SeSceneItemProperties mProperties;
  
public:
  inline void setIdentifier(const QString & identifier) { properties().setIdentifier(identifier); }
  inline QString identifier() const { return mProperties.identifier(); }
  
public:
  int width() { return properties().size().width(); }
  int height() { return properties().size().height(); } 
};



#endif // __SESCENEITEM_H__

