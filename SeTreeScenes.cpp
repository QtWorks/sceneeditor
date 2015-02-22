/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

// SceneEditor
#include <SeTreeScenes.h>
#include <SeGeneral.h>

// Qt
#include <QUuid>
#include <QMenu>
#include <QDebug>
#include <QTreeWidget>
#include <QMessageBox>
#include <QContextMenuEvent>

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int SeTreeSceneItem::__instances = 0;

SeTreeSceneItem::SeTreeSceneItem(const QString & uuid)
  : QTreeWidgetItem()
{
  ++__instances;
  
  QString name = QString("Scene %1").arg(__instances);
  
  this->setText(0, name);  
  this->setData(0, Roles::Uuid, uuid);
}

SeTreeSceneItem::~SeTreeSceneItem()
{
}

QVariant SeTreeSceneItem::data(int column, int role) const
{
  return QTreeWidgetItem::data(column, role);
}

void SeTreeSceneItem::setData(int column, int role, const QVariant &value)
{
  QTreeWidgetItem::setData(column, role, value);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SeTreeScenes::SeTreeScenes(QWidget *parent)
  : QTreeWidget(parent)
{
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->setDragEnabled(true);
  this->setDropIndicatorShown(true);
  this->setDragDropMode(QAbstractItemView::InternalMove);
  
  QObject::connect(this, &SeTreeScenes::itemClicked, [=](QTreeWidgetItem *item, int column){
    QString identifier = item->data(column, SeTreeSceneItem::Roles::Uuid).toString();
    if(identifier.isEmpty() == false)
    {
      emit sceneLayerClicked(identifier);
    }
  });
  
  //QObject::connect(this, &SeTreeScenes::itemActivated, [=](QTreeWidgetItem *item, int column){
  //  QString identifier = item->data(column, SeTreeSceneItem::Roles::Uuid).toString();
  //  if(identifier.isEmpty() == false)
  //  {
  //    emit sceneLayerClicked(identifier);
  //  }
  //});
  
  QObject::connect(this, &SeTreeScenes::itemChanged, [=](QTreeWidgetItem *item, int column){
    QString identifier = item->data(column, SeTreeSceneItem::Roles::Uuid).toString();
    if(identifier.isEmpty() == false)
    {
      emit sceneLayerClicked(identifier);
    }
  });
}

SeTreeScenes::~SeTreeScenes()
{
}

QStringList SeTreeScenes::identifiers()
{
  QStringList ids;

  QTreeWidgetItemIterator it(this);
  while(*it)
  {
    SeTreeSceneItem *treeSceneItem = dynamic_cast<SeTreeSceneItem*>(*it);
    if(treeSceneItem == NULL) 
    {
      // ...
    }
    else
    {  
      ids.append(treeSceneItem->data(0, SeTreeSceneItem::Roles::Uuid).toString());
    }
    
    ++it;
  }
  
  return ids;
}

int SeTreeScenes::indexOf(const QString &identifier)
{
  int indexPos = -1;

  QStringList ids = this->identifiers();
  for(int i=0; i < ids.count(); i++)
  {
    qDebug() << QString("%1: %2").arg(i).arg(ids.at(i));
    
    if(ids.at(i) == identifier)
    {
      indexPos = i;
    }
  }
  
  return indexPos;
}

void SeTreeScenes::contextMenuEvent(QContextMenuEvent *e)
{
  QTreeWidgetItem *ptrItem = this->itemAt(e->pos());

  if(ptrItem != NULL)
  {
    QMenu m;
    
    QAction *actDuplicate = m.addAction(tr("Duplicate"));
    QAction *actRemove = m.addAction(tr("Remove"));

    QObject::connect(actDuplicate, &QAction::triggered, [=](){ this->duplicateScene(); });
    QObject::connect(actRemove, &QAction::triggered, [=](){ this->removeScene(); });
    
    actDuplicate->deleteLater();
    actRemove->deleteLater();
    
    m.exec(mapToGlobal(e->pos()));
  }
  else
  {
    QMenu m;
    QAction *act0 = m.addAction(tr("New Scene"));
    
    QObject::connect(act0, &QAction::triggered, [=](){ this->addScene(); });
    
    act0->deleteLater();
    
    m.exec(mapToGlobal(e->pos()));
  }
}

void SeTreeScenes::keyPressEvent(QKeyEvent *e)
{
  QTreeWidget::keyPressEvent(e);
}

void SeTreeScenes::keyReleaseEvent(QKeyEvent *e)
{
  emit sceneLayerClicked(this->currentItem()->data(0, SeTreeSceneItem::Roles::Uuid).toString());
  
  QTreeWidget::keyReleaseEvent(e);
}

SeTreeSceneItem *SeTreeScenes::addScene(
    const QString & identifier
  , bool informEnvironment
) {
  QString id;

  if(identifier.isEmpty())
  {    
    QUuid uuid = QUuid::createUuid();      
    id = uuid.toString();
  }
  else
  {
    id = identifier;
  }
  SeTreeSceneItem *p = new SeTreeSceneItem(id);
  this->addTopLevelItem(p);
  this->setCurrentItem(p);
  
  if(informEnvironment == true)
  {
    emit createScene(p->data(0, SeTreeSceneItem::Roles::Uuid).toString());
  }
  emit sceneLayerClicked(p->data(0, SeTreeSceneItem::Roles::Uuid).toString());  

  return p;
}

bool SeTreeScenes::removeScene()
{
  int res = QMessageBox::question(
      this
    , tr("Removing scene?")
    , tr("Do you like to permanently delete the selected scene?")
  );

  if(res == QMessageBox::No)
  {
    return false;
  }

  QList<QTreeWidgetItem*> items = this->selectedItems();
  for(QTreeWidgetItem *p : items)
  {
    SE_CONT4NULL(p);
    SeTreeSceneItem *pitem = dynamic_cast<SeTreeSceneItem*>(p);
    SE_CONT4NULL(pitem);    
    QString id = pitem->data(0, SeTreeSceneItem::Roles::Uuid).toString();
    SE_DELETE(p);    
    emit removeScene(id);
  }
  
  return true;
}

bool SeTreeScenes::duplicateScene()
{
  QList<QTreeWidgetItem*> items = this->selectedItems();

  for(QTreeWidgetItem *p : items)
  {
    SE_CONT4NULL(p);   
    SeTreeSceneItem *pitem = dynamic_cast<SeTreeSceneItem*>(p);
    SE_CONT4NULL(pitem);
        
    SeTreeSceneItem *pitemNew = this->addScene("", false);
    if(pitemNew != NULL)
    {        
      qDebug() << "New item: " << pitemNew->data(0, SeTreeSceneItem::Roles::Uuid).toString();
    }
        
    QString srcUuid = pitem->data(0, SeTreeSceneItem::Roles::Uuid).toString();
    QString dstUuid = pitemNew->data(0, SeTreeSceneItem::Roles::Uuid).toString(); 
        
    emit duplicateScene(srcUuid, dstUuid);
  }
  
  return true;
}
