/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeSceneLayer.h>
#include <SeSceneItem.h>
#include <SeGeneral.h>

// Qt
#include <QPen>
#include <QUuid>
#include <QDebug>
#include <QPainter>
#include <QJsonArray>
#include <QSharedPointer>

SeSceneLayer::SeSceneLayer(int rows, int columns, SeSceneItem *parent)
  : SeSceneItem(parent)
  , mRows(rows)
  , mColumns(columns)
  , mDelay(1.0f)
{
}

SeSceneLayer::~SeSceneLayer()
{  
}

void SeSceneLayer::deepCopy(SeSceneLayer *ptrSourceLayer)
{
  this->deepCopy(*ptrSourceLayer);
}

void SeSceneLayer::deepCopy(SeSceneLayer &sourceLayer)
{
  QString keepId = this->properties().identifier();
  this->properties() = sourceLayer.properties();
  this->properties().setIdentifier(keepId);
  
  for(int x=0; x < sourceLayer.numberOfColumns(); x++)
  {
    for(int y=0; y < sourceLayer.numberOfRows(); y++)
    {
      SeSceneItem *p = this->sceneItem(x, y);
    
      p->properties() = sourceLayer.sceneItem(x, y)->properties();
      p->properties().setIdentifier(QUuid::createUuid().toString());      
    }
  }
}

void SeSceneLayer::changeShapeMode(SeSceneItemProperties::ShapeMode mode)
{
  for(int i=0; i < mItems.size(); i++)
  {
    for(int j=0; j < mItems[i].size(); j++)
    {
      mItems[i][j]->properties().setShapeMode(mode);
    }
  }
}

void SeSceneLayer::loadJson(const QJsonObject & jsonObj)
{
  this->mDelay   = jsonObj["Delay"].toDouble(); 
  this->mRows    = jsonObj["Rows"].toInt();
  this->mColumns = jsonObj["Columns"].toInt();
  
  this->properties().restore(jsonObj["Properties"].toObject());
  
  QJsonArray ar = jsonObj["Items"].toArray();
  
  for(QJsonValue v : ar)
  {
    SE_CONT4TRUE(!v.isObject());    
    QJsonObject o = v.toObject();    
    QString entryName = o["Name"].toString();
   
    QStringList parts = entryName.split("-");
    if(parts.count() != 3)
    {
      throw new std::exception("Entry name has wrong format, please use 'SceneItem-XX-YY'.");
      
      continue;
    }
    
    int x = parts[1].trimmed().toInt();
    int y = parts[2].trimmed().toInt();
    
    SeSceneItem *pitem = sceneItem(x, y);
    pitem->properties().restore(o["Data"].toObject());
    pitem->show();
    pitem->update();
  }  
  
  this->show();
}

QJsonObject SeSceneLayer::toJson()
{
  QJsonObject obj;

  obj["Delay"] = this->mDelay;
  obj["Rows"] = this->mRows;
  obj["Columns"] = this->mColumns;
  
  obj["Properties"] = this->properties().toJson();

  QJsonArray ar;

  for(int row=0; row < this->mRows; row++)
  {
    for(int col=0; col < this->mColumns; col++)
    {
      int x = this->mColumns - col - 1;
      int y = this->mRows - row - 1;
      
      SeSceneItem *item = this->sceneItem(x, y);
      SE_CONT4NULL(item);
      
      QString entryName = QString("SceneItem-%1-%2").arg(x).arg(y);
      
      QJsonObject o; 
      o["Name"] = entryName;
      o["Data"] = item->properties().toJson();  
      
      ar.append(o);    
    }
  }
  
  obj["Items"] = ar;
    
  return obj;
}

QString SeSceneLayer::toAvrCsv()
{
  QString s; QTextStream ss(&s);
  
  //
  // 10|20,x00,x01,x02,...,x0N-1,x10,x11,...,x1N-1,xM-1N-1
  //
  // x00 := RGB    
  //
  
  ss << this->mRows << "|" << this->mColumns << ",";
  for(int row=0; row < this->mRows; row++)
  {
    for(int col=0; col < this->mColumns; col++)
    {
      SeSceneItem *item = this->sceneItem(col, row);
      SE_CONT4NULL(item);
    
      QColor c = item->properties().brushColor();
    
      ss << QString("%1|%2|%3")
              .arg(c.red(),   2, 16, QChar('0'))
              .arg(c.green(), 2, 16, QChar('0'))
              .arg(c.blue(),  2, 16, QChar('0'));

      ss << ",";
    }
  }
  
  s.remove(s.length()-1, 1);  
  
  return s;
}

QRectF SeSceneLayer::boundingRect() const
{
  if(isEmpty()) { return QRectF(-1, -1, 5, 5); }

  SeSceneItem *p = mItems[0][0];
  int w = p->width();
  int h = p->height();

  return QRectF(-1, -1, mColumns * w + 2, mRows * h + 2);
}

void SeSceneLayer::paint(
    QPainter *painter
  , const QStyleOptionGraphicsItem *option
  , QWidget *widget
) {
  Q_UNUSED(option);
  Q_UNUSED(widget);

  if(isEmpty()) 
  {
    QRectF r = this->boundingRect();
    painter->setPen(QColor(255, 0, 0));
    painter->setBrush(QColor(255, 0, 0));
    painter->drawRect(r);    
    return;
  }

  SeSceneItem *p = mItems[0][0];
  int w = mColumns * p->width();
  int h = mRows * p->height();
  
  painter->save();
    QPen pen;
    pen.setColor(QColor(255, 0, 0, 255));
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawRect(0, 0, w, h);
  painter->restore();

  painter->save();
    painter->setPen(QColor(0, 0, 0));
    painter->setBrush(QColor(0, 0, 0));
    painter->drawRect(1, 1, w-2, h-2);
  painter->restore();
  
}

SeSceneItem* SeSceneLayer::sceneItem(int x, int y)
{
  if(x < 0 || x >= mColumns) { return NULL; }
  if(y < 0 || y >= mRows)    { return NULL; }
  
  return mItems[y][x];
}
