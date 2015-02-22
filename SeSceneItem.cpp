/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeSceneLayer.h>
#include <SeSceneItem.h>
#include <SeSceneLed.h>

// Qt
#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QBuffer>
#include <QPainter>

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeSceneItemProperties::SeSceneItemProperties() 
  : mrow(-1)
  , mcolumn(-1)
  , mtransitionMode(Hard)
  , mSize(50, 50)
  , mShapeMode(ShapeRect)
  , mOffset(0, 0)
  , mScale(100)
  , mEnabled(true)
  , mIndex(-1)
  , powner(NULL)
{  
  mbrushcolor = QColor(255, 255, 255);
  mpencolor = QColor(0, 0, 0);
}

SeSceneItemProperties::SeSceneItemProperties(const SeSceneItemProperties &obj)
  : midentifier(obj.midentifier)
  , mrow(obj.mrow)
  , mcolumn(obj.mcolumn)
  , mbrushcolor(obj.mbrushcolor)
  , mpencolor(obj.mpencolor)
  , mtransitionMode(obj.mtransitionMode)
  , mOriginalFilePath(obj.mOriginalFilePath)
  , mOriginalPixmap(obj.mOriginalPixmap)
  , mSize(obj.mSize)
  , mOffset(obj.mOffset)
  , mSelectionGeometry(obj.mSelectionGeometry)
  , mShapeMode(obj.mShapeMode)  
  , mEnabled(obj.mEnabled)
  , mIndex(obj.mIndex)
  , powner(obj.powner) { }

SeSceneItemProperties & SeSceneItemProperties::operator=(const SeSceneItemProperties & obj)
{
  midentifier = obj.midentifier;
  mrow = obj.mrow;
  mcolumn = obj.mcolumn;
  mbrushcolor = obj.mbrushcolor;
  mpencolor = obj.mpencolor;
  mtransitionMode = obj.mtransitionMode;
  mOriginalFilePath = obj.mOriginalFilePath;
  mOriginalPixmap = obj.mOriginalPixmap;
  mSize = obj.mSize;
  mOffset = obj.mOffset;
  mSelectionGeometry = obj.mSelectionGeometry;
  mShapeMode = obj.mShapeMode;
  mEnabled = obj.mEnabled;
  mIndex = obj.mIndex;
  powner = obj.powner;
  return *this;
}

void SeSceneItemProperties::setIdentifier(const QString &identifier) { this->midentifier = identifier; }
void SeSceneItemProperties::setRow(int row)         { this->mrow = row;  }
void SeSceneItemProperties::setColumn(int column)   { this->mcolumn = column; }
void SeSceneItemProperties::setBrushColor(QColor c) { this->mbrushcolor = c; }
void SeSceneItemProperties::setPenColor(QColor c)   { this->mpencolor = c; }
void SeSceneItemProperties::setTransitionMode(TransitionMode mode) { this->mtransitionMode = mode; }
void SeSceneItemProperties::setOriginalFilePath(const QString &filePath) { mOriginalFilePath = filePath; }
void SeSceneItemProperties::setOriginalPixmap(const QPixmap &pix) { mOriginalPixmap = pix; }
void SeSceneItemProperties::setSize(const QSize &size) { mSize = size; }
void SeSceneItemProperties::setSize(int w, int h) { mSize = QSize(w, h); }
void SeSceneItemProperties::setShapeMode(ShapeMode mode) { mShapeMode = mode; }
void SeSceneItemProperties::setOffset(const QPoint &offset) { mOffset = offset; }
void SeSceneItemProperties::setSelectionGeometry(const QRect & rect) { mSelectionGeometry = rect; }
void SeSceneItemProperties::setScale(int scale) { if(scale < 0) { mScale = 100; } else { mScale = scale; } }
void SeSceneItemProperties::setEnabled(bool state) { mEnabled = state; }
void SeSceneItemProperties::setIndex(int index) { mIndex = index; }

void SeSceneItemProperties::restore(const QJsonObject & obj)
{
  bool isLayer = dynamic_cast<SeSceneLayer*>(powner) != NULL;
  bool isLed   = dynamic_cast<SeSceneLed*>(powner) != NULL;

  this->midentifier = obj["Identifier"].toString();

  if(isLed == true)
  {
    this->mrow = obj["Row"].toInt();
    this->mcolumn = obj["Column"].toInt();
  
    int rgb0;
    int rgb1;
  
    rgb0 = obj["BrushColor"].toInt();
    rgb1 = obj["PenColor"].toInt();
  
    this->setBrushColor(QColor(qRed(rgb0), qGreen(rgb0), qBlue(rgb0)));
    this->setPenColor(QColor(qRed(rgb1), qGreen(rgb1), qBlue(rgb1)));

    this->mtransitionMode = (TransitionMode) obj["TransitionMode"].toInt();

    int w = obj["Width"].toInt();
    int h = obj["Height"].toInt();

    mSize.setWidth(w);
    mSize.setHeight(h);
  }

  if(isLayer)
  {  
    this->mOriginalFilePath = obj["OriginalFilePath"].toString();
    
    QByteArray ba0; ba0.append(obj["OriginalPixmap"].toString());
    QByteArray img0 = QByteArray::fromBase64(ba0);
    QImage img00 = QImage::fromData(img0, "PNG");
    mOriginalPixmap = QPixmap::fromImage(img00);
    
    mShapeMode = (ShapeMode) obj["ShapeMode"].toInt();  
    
    mOffset.setX(obj["OffsetX"].toInt());
    mOffset.setY(obj["OffsetY"].toInt());
    
    mSelectionGeometry.setX(obj["SelectionGeometryX"].toInt());
    mSelectionGeometry.setY(obj["SelectionGeometryY"].toInt());
    mSelectionGeometry.setWidth(obj["SelectionGeometryW"].toInt());
    mSelectionGeometry.setHeight(obj["SelectionGeometryH"].toInt());
    
    mScale = obj["Scale"].toInt();    
    mEnabled = obj["Enabled"].toBool();
    mIndex = obj["Index"].toInt();
  }
      
  powner->update();
}

QJsonObject SeSceneItemProperties::toJson()
{
  bool isLayer = dynamic_cast<SeSceneLayer*>(powner) != NULL;
  bool isLed   = dynamic_cast<SeSceneLed*>(powner) != NULL;

  QJsonObject o;

  o["Identifier"] = this->midentifier;

  if(isLed == true)
  {
    o["Row"] = this->mrow;
    o["Column"] = this->mcolumn;
    o["BrushColor"] = (int) this->mbrushcolor.rgb();
    o["PenColor"] = (int) this->mpencolor.rgb();
    o["TransitionMode"] = this->mtransitionMode;

    o["Width"] = this->mSize.width();
    o["Height"] = this->mSize.height();
  }

  if(isLayer == true)
  {  
    o["OriginalFilePath"] = this->mOriginalFilePath;
    
    QByteArray byteArray0;
    QBuffer buffer0(&byteArray0);
    mOriginalPixmap.save(&buffer0, "PNG");
    o["OriginalPixmap"] = byteArray0.toBase64().data();
      
    o["ShapeMode"] = (int) this->shapeMode();
    o["OffsetX"] = (int) this->offset().x();
    o["OffsetY"] = (int) this->offset().y();
      
    o["SelectionGeometryX"] = (int) this->mSelectionGeometry.x();
    o["SelectionGeometryY"] = (int) this->mSelectionGeometry.y(); 
    o["SelectionGeometryW"] = (int) this->mSelectionGeometry.width();
    o["SelectionGeometryH"] = (int) this->mSelectionGeometry.height();
    
    o["Scale"] = (int) this->mScale;
    o["Enabled"] = (bool) this->mEnabled;
    o["Index"] = (int) this->mIndex;
  }
  
  return o;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeSceneItem::SeSceneItem(QGraphicsItem *parent) 
  : QGraphicsItem(parent)
{
  this->properties().setIdentifier(QUuid::createUuid().toString());
  this->properties().setOwner(this);
}

SeSceneItem::~SeSceneItem()
{
}
