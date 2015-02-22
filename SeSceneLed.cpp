/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeSceneLed.h>

// Qt
#include <QDebug>
#include <QPainter>

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeSceneLed::SeSceneLed(SeSceneItem *parent)
  : SeSceneItem(parent)  
{
  this->setFlag(QGraphicsItem::ItemIsSelectable, true);

  this->properties().setOwner(this);
}

SeSceneLed::~SeSceneLed()
{
}

QRectF SeSceneLed::boundingRect() const
{
  return QRectF(
          -1.f
        , -1.f
        , mProperties.width() + 2.f
        , mProperties.height() + 2.f
      );
}

void SeSceneLed::paint(
    QPainter *painter
  , const QStyleOptionGraphicsItem *option
  , QWidget *widget
) {
  Q_UNUSED(option);
  Q_UNUSED(widget);
  
  QRectF r(0, 0, properties().size().width(), properties().size().height());
  
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
  painter->setBrush(properties().mbrushcolor);
  painter->setPen(properties().mpencolor);
  switch(properties().shapeMode())
  {
    case SeSceneItemProperties::ShapeMode::ShapeCircle: painter->drawEllipse(r); break;
    case SeSceneItemProperties::ShapeMode::ShapeRect: painter->drawRect(r); break;
  }
  
  if(isSelected())
  {
    painter->save();
    painter->setPen(QPen(Qt::red, 1, Qt::DashLine));
    painter->drawLine(2, 2, properties().size().width()-2, 2);
    painter->drawLine(properties().size().width()-2, 1, properties().size().width()-2, properties().size().height()-2);
    painter->drawLine(properties().size().width()-2, properties().size().height()-2, 2, properties().size().height()-2);
    painter->drawLine(2, properties().size().height()-2, 2, 2);

    painter->restore();
  }
}

void SeSceneLed::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  SeSceneItem::mouseReleaseEvent(event);
}
