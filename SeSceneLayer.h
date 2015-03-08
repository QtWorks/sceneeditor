/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#pragma once

#ifndef __SESCENELAYER_H__
#define __SESCENELAYER_H__

// SceneEditor
#include <SeSceneItem.h>

// Qt
#include <QMap>
#include <QList>
#include <QJsonObject>
#include <QSharedPointer>

#define DEFAULT_ROWS 10      // was 10
#define DEFAULT_COLUMNS 20   // was 20

/**
 * @brief The SeSceneLayer class
 */
class SeSceneLayer
  : public SeSceneItem
{
public:
  SeSceneLayer(int rows=DEFAULT_ROWS, int columns=DEFAULT_COLUMNS, SeSceneItem *parent=0);
  ~SeSceneLayer();

  friend class SeScenePlayerTransitions;

  template<class T> void initialize() {
    for(int i=0; i < mRows; i++) {
      QList<SeSceneItem*> rowItems;
      for(int j=0; j < mColumns; j++) {
        T *p = new T(this);
        p->properties().setRow(i);
        p->properties().setColumn(j);
        p->setPos(j * p->width(), i * p->height());
        rowItems.append(p);     
      }      
      mItems[i] = rowItems;      
    }
  } 

  void deepCopy(SeSceneLayer * ptrSourceLayer);
  void deepCopy(SeSceneLayer & sourceLayer);

public:
  void changeShapeMode(SeSceneItemProperties::ShapeMode mode);  
   
   void loadJson(const QJsonObject & jsonObj);
   QJsonObject toJson();
   
   QString toAvrCsv();
   
   //! Generated and returns the JSON command used for 
   //! deploying this Layer to the Node.js target.
   //! Following format is used for the JSON object:
   //!
   //! Protocol:
   //! --------------------------------------------------------------
   //! { "type" : "single",
   //!   "data" : {
   //!       "x" : INT, "y" : INT,
   //!       "red" : INT, "green" : INT, "blue" : INT
   //! }}
   //!
   //! { "type" : "grid",
   //!   "data" : [
   //!  { "x":INT, "y":INT, "red":INT, "green":INT, "blue":INT },
   //!      { ... }
   //! ]}
   //! --------------------------------------------------------------
   QJsonObject toGridCommand();   
   
protected:
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
      
public:
  void setDelay(double value) { mDelay = value; }
  double delay() const { return mDelay; }      
      
  inline int numberOfColumns() const { return mColumns; }
  inline int numberOfRows() const { return mRows; }
      
private:
  double mDelay;
  int mRows;
  int mColumns;

  // multi-dimensional field: [y][x] = scene item
  QMap< int, QList< SeSceneItem* > > mItems;

  bool isEmpty() const { return mItems.count() <= 0; }

public:
  SeSceneItem* sceneItem(int x, int y);
};

#endif // __SESCENELAYER_H__
