#include <QAbstractItemModel>
#include <QMap>
#include <QDomNamedNodeMap>

#include "KnowTreeModel.h"
#include "TreeItem.h"
#include "TreeModel.h"
#include "ClipboardRecords.h"
#include "ClipboardBranch.h"
#include "main.h"
#include "AppConfig.h"
#include "TreeScreen.h"
#include "crypt/Password.h"

extern AppConfig mytetraconfig;



// Конструктор модели дерева, состоящего из Item элементов
// Принимает заголовки колонок и DOM модель древовидных данных
KnowTreeModel::KnowTreeModel(const QStringList &headers, QDomDocument domModel, QObject *parent)
{
  Q_UNUSED(headers);
  Q_UNUSED(parent);

  rootItem=NULL;

  init(domModel);
}


// Деструктор Item модели.
// По-хорошему деструктор перед удалением корневого элемента должен пробежать по
// дереву элементов и удалить их
KnowTreeModel::~KnowTreeModel()
{
 delete rootItem;
}


void KnowTreeModel::init(QDomDocument domModel)
{
  QMap<QString, QString> rootData;

  // Определяется одно поле в корневом объекте
  // то есть на экране будет один столбец
  rootData["id"]="0";
  rootData["name"]="";

  // Создание корневого Item объекта
  if(rootItem!=NULL) delete rootItem;
  rootItem = new TreeItem(rootData);

  // Динамическое создание дерева из Item объектов на основе DOM модели
  setupModelData(domModel, rootItem);

  // Вызывается метод, который должен известить view о том что данные обновились
  reset();
}


// Разбор DOM модели и преобразование ее в Item модель
void KnowTreeModel::setupModelData(QDomDocument dommodel, TreeItem *parent)
{
  QDomElement contentrootnode=dommodel.documentElement().firstChildElement("content").firstChildElement("node");

  if(contentrootnode.isNull())
   {
    qDebug() << "Unable load xml tree, first content node not found.";
    return;
   }

  parseNodeElement(contentrootnode, parent);

  return;
}


// Рекурсивный обход DOM дерева и извлечение из него узлов
void KnowTreeModel::parseNodeElement(QDomElement n, TreeItem *parent)
{
 TreeItem *prnt = parent;

 // У данного Dom-элемента ищется таблица конечных записей
 // и XML записи заполняются в Item таблицу конечных записей
 prnt->recordtableInit(n);

 // Пробегаются все DOM элементы текущего уровня
 // и рекурсивно вызывается обработка подуровней
 while(!n.isNull())
 {
  if(n.tagName()=="node")
   {
    // Обнаруженый подузел прикрепляется к текущему элементу
    prnt->insertChildren(prnt->childCount(), 1, 1);

    /*
    QString line1,line_name,line_id;
    line1=n.tagName();
    line_name=n.attribute("name");
    line_id=n.attribute("id");
    qDebug() << "Read node " << line1 << " " << line_id<< " " << line_name;
    */

    // Определяются атрибуты узла
    QDomNamedNodeMap attributeMap = n.attributes();

    // Перебираются атрибуты узла
    for(int i = 0; i<attributeMap.count(); ++i)
     {
      QDomNode attribute = attributeMap.item(i);

      QString name=attribute.nodeName();
      QString value=attribute.nodeValue();

      // В дерево данных устанавливаются считанные атрибуты
      prnt->child(prnt->childCount()-1)->setFieldDirect(name ,value);
     }


    // Вызов перебора оставшегося DOM дерева с прикреплением обнаруженных объектов
    // к только что созданному элементу
    parseNodeElement(n.firstChildElement(), prnt->child(prnt->childCount()-1) );

   }
  n = n.nextSiblingElement();
 }

}


// Генерирование полного DOM дерева хранимых данных
QDomElement KnowTreeModel::exportFullModelDataToDom(TreeItem *root)
{
 QDomDocument doc;
 QDomElement elm=doc.createElement("content");

 // qDebug() << "New element for export" << xmlnode_to_string(elm);

 parseTreeToDom(elm, root);

 // qDebug() << "In export_fullmodeldata_to_dom stop element " << xmlnode_to_string(elm);

 return elm;
}


// Рекурсивное преобразование Item-элементов в Dom дерево
void KnowTreeModel::parseTreeToDom(QDomElement &xmlData, TreeItem *currItem)
{

 // Если в ветке присутсвует таблица конечных записей
 // В первую очередь добавляется она
 if(currItem->recordtableGetRowCount() > 0)
  {
   // Обработка таблицы конечных записей

   // Получение Dom дерева таблицы конечных записей
   QDomDocument rectab=currItem->recordtableExportDataToDom();

   // qDebug() << "In parsetreetodom() rectab " << rectab.toString();

   // Получаем корневой элемент документа
   QDomElement e = rectab.documentElement(); 

   // Dom дерево таблицы конечных записей добавляется
   // как подчиненный элемент к текущему элементу
   if(!e.isNull())
    xmlData.appendChild(e.cloneNode());
   else
    qDebug() << "No convert QDomDocument to element for recordtable";
  }

 // Обработка каждой подчиненной ветки
 int i;
 for(i=0;i<currItem->childCount();i++)
  {
   // Временный элемент, куда будет внесена текущая перебираемая ветка
   QDomDocument doc;
   QDomElement  tempelement = doc.createElement("node");

   // Получение всех полей для данной ветки
   QMap<QString, QString> fields=currItem->child(i)->getAllFieldsDirect();

   // Перебираются поля элемента ветки
   QMapIterator<QString, QString> fields_iterator(fields);
   while (fields_iterator.hasNext())
    {
     fields_iterator.next();

     // Установка для временного элемента значения перебираемого поля как атрибута
     tempelement.setAttribute(fields_iterator.key(), fields_iterator.value());
    }


   // Добавление временного элемента к основному документу
   xmlData.appendChild(tempelement);

   // qDebug() << "In parsetreetodom() current construct doc " << xmlnode_to_string(*xmldata);

   // Рекурсивная обработка
   QDomElement workElement=xmlData.lastChildElement();
   parseTreeToDom(workElement, currItem->child(i) );
  }

}


// Добавление новой подветки к указанной ветке
void KnowTreeModel::addNewChildBranch(const QModelIndex &index, QString id, QString name)
{
 // Получение ссылки на Item элемент по QModelIndex
 TreeItem *parent=getItem(index);

 beginInsertRows(index, parent->childCount(), parent->childCount());
 addNewBranch(parent, id, name);
 endInsertRows();
}


// Добавление новой ветки после указанной ветки
void KnowTreeModel::addNewSiblingBranch(const QModelIndex &index, QString id, QString name)
{
 // Получение ссылки на родительский Item элемент по QModelIndex
 TreeItem *current=getItem(index);
 TreeItem *parent=current->parent();
 
 beginInsertRows(index.parent(), parent->childCount(), parent->childCount());
 addNewBranch(parent, id, name);
 endInsertRows();
}


// Добавление новой подветки к Item элементу
void KnowTreeModel::addNewBranch(TreeItem *parent, QString id, QString name)
{
 // Подузел прикрепляется к указанному элементу 
 // в конец списка подчиненных элементов
 parent->addChildren();

 // Определяется ссылка на только что созданную ветку
 TreeItem *current=parent->child(parent->childCount()-1);

 // Устанавливается идентификатор узла
 current->setField("id",id);

 // Устанавливается навание узла
 current->setField("name",name);

 // Инициализируется таблица конечных записей
 current->recordtableGetTableData()->init(current, QDomElement());

 // Определяется, является ли родительская ветка зашифрованной
 if(parent->getField("crypt")=="1")
  {
   // Новая ветка превращается в зашифрованную
   current->switchToEncrypt();
  }
}


// Добавление новой подветки к указанной ветке из буфера обмена
QString KnowTreeModel::pasteNewChildBranch(const QModelIndex &index, ClipboardBranch *subbranch)
{
 QString pasted_branch_id;

 // Получение ссылки на Item элемент по QModelIndex
 TreeItem *parent=getItem(index);

 beginInsertRows(index, parent->childCount(), parent->childCount());
 pasted_branch_id=pasteSubbranch(parent, (ClipboardBranch *)subbranch);
 endInsertRows();

 return pasted_branch_id;
}


QString KnowTreeModel::pasteNewSiblingBranch(const QModelIndex &index, ClipboardBranch *subbranch)
{
 QString pasted_branch_id;

 // Получение ссылки на родительский Item элемент по QModelIndex
 TreeItem *current=getItem(index);
 TreeItem *parent=current->parent();

 beginInsertRows(index.parent(), parent->childCount(), parent->childCount());
 pasted_branch_id=pasteSubbranch(parent, (ClipboardBranch *)subbranch);
 endInsertRows();

 return pasted_branch_id;
}


// Перемещение ветки вверх
QModelIndex KnowTreeModel::moveUpBranch(const QModelIndex &index)
{
 return moveUpDnBranch(index,1);
}


// Перемещение ветки вниз
QModelIndex KnowTreeModel::moveDnBranch(const QModelIndex &index)
{
 return moveUpDnBranch(index,-1);
}


// Перемещение ветки вверх или вниз
QModelIndex KnowTreeModel::moveUpDnBranch(const QModelIndex &index,int direction)
{
 // Получение QModelIndex расположенного над или под элементом index
 QModelIndex swap_index=index.sibling(index.row()-direction,0);

 // Проверяется допустимость индекса элемента, куда будет сделано перемещение
 if(!swap_index.isValid())
  return QModelIndex(); // Возвращается пустой указатель
 
 // Запоминаются параметры для абсолютного позиционирования засветки
 // после перемещения ветки
 int         swpidx_row=swap_index.row();
 int         swpidx_column=swap_index.column();
 QModelIndex swpidx_parent=swap_index.parent();

 // Получение ссылки на Item элемент по QModelIndex
 TreeItem *current=getItem(index);

 // Перемещается ветка
 emit layoutAboutToBeChanged();

 bool moveok;
 if(direction==1) moveok=current->moveUp();  // Перемещение в Item представлении
 else             moveok=current->moveDn();

 if(moveok)
  {
   changePersistentIndex(swap_index,index);
   changePersistentIndex(index,swap_index);
  } 

 emit layoutChanged();

 // Возвращается указатель на перемещенную ветку
 if(moveok) return this->index(swpidx_row, swpidx_column, swpidx_parent);
 else return QModelIndex(); // Возвращается пустой указатель
}


// Получение индекса подчиненного элемента с указанным номером
// Нумерация элементов считается что идет с нуля
QModelIndex KnowTreeModel::indexChildren(const QModelIndex &parent, int n) const
{
 // Проверяется, передан ли правильный QModelIndex
 // Если он неправильный, возвращается пустой индекс
 if(!parent.isValid()) 
  {
   qDebug() << "In indexChildren() unavailable model index";
   return QModelIndex();
  } 
 
 // Выясняется указатель на основной Item элемент
 TreeItem *item = getItem(parent);
 
 // Если у основного Item элемента запрашивается индекс 
 // несуществующего подэлемента, возвращается пустой индекс
 if( n<0 || n>(item->childCount()-1) ) 
  {
   qDebug() << "In indexChildren() unavailable children number";
   return QModelIndex();
  } 
  
 // Выясняется указатель на подчиненный Item элемент
 TreeItem *childitem = item->child(n);
    
 // Если указатель на подчиненный Item элемент непустой
 if(childitem)
  {
   // return createIndex(0, 0, childitem); // Индекс подчиненного элемента
   // return createIndex(n, 0, parent.internalPointer());
   
   return index(n,0,parent);
  }
 else
  {
   qDebug() << "In indexChildren() empty child element";
   return QModelIndex(); // Возвращается пустой индекс
  }
}


// Получение QModelIndex по известному TreeItem
QModelIndex KnowTreeModel::getIndexByItem(TreeItem *item)
{
 int itemrow=item->childNumber();
 
 return this->createIndex(itemrow, 0, item);
}


// Возвращает общее количество записей, хранимых в дереве
int KnowTreeModel::getAllRecordCount(void)
{
 // Обнуление счетчика
 getAllRecordCountRecurse(rootItem, 0);

 return getAllRecordCountRecurse(rootItem, 1);
}


int KnowTreeModel::getAllRecordCountRecurse(TreeItem *item, int mode)
{
 static int n=0;
 
 if(mode==0)
  {
   n=0;
   return 0;
  }
 
 n=n+item->recordtableGetRowCount();

 for(int i=0; i < item->childCount(); i++)
  getAllRecordCountRecurse(item->child(i), 1);
 
 return n;
}


// Добавление подветки из буфера обмена относительно указанного элемента
// Функция возвращает новый идентификатор стартовой добавленной подветки
QString KnowTreeModel::pasteSubbranch(TreeItem *item, ClipboardBranch *subbranch)
{
 qDebug() << "In paste_subbranch()";
 
 // Выясняется линейная структура добавляемого дерева
 QList<CLIPB_TREE_ONE_LINE> tree=subbranch->getIdTree();

 // Идентификатор стартовой ветки лежит в первом элементе списка
 QString start_branch_id=tree[0].branch_id;

 return pasteSubbranchRecurse(item, start_branch_id, subbranch);
}


// Рекурсивное добавление дерева
// item - элемент главного дерева, к которому добавляется ветка
// start_branch_id - идентификатор ветки в переданном линейном представлении
//                   добавляемого дерева
// subbranch - добавляемое дерево
QString KnowTreeModel::pasteSubbranchRecurse(TreeItem *item,
                                               QString startBranchId,
                                               ClipboardBranch *subbranch)
{
 qDebug() << "In paste_subbranch_recurse()";

 // ---------------------------
 // Добавляется стартовая ветка
 // ---------------------------

 // Выясняются поля стартовой ветки
 QMap<QString, QString> subbranch_fields=subbranch->getBranchFieldsById(startBranchId);

 // Выясняется имя ветки
 QString subbranch_name=subbranch_fields["name"];

 // Получение уникального идентификатора, под которым будет добавляться ветка
 QString id=get_unical_id();

 // Стартовая ветка добавляется
 addNewBranch(item, id, subbranch_name);

 // Выясняется указатель на эту добавленную ветку
 TreeItem *newitem=getItemById(id);

 qDebug() << "KnowTreeModel::paste_subbranch_recurse() : create branch with field" << newitem->getAllFields();

 // -----------------------------------------------
 // Для стартовой ветки добавляются конечные записи
 // -----------------------------------------------

 // Выясняются данные конечных записей
 QList< CLIPB_ONE_RECORD_STRUCT > records=subbranch->getBranchRecords(startBranchId);

 foreach(CLIPB_ONE_RECORD_STRUCT record, records)
 {
  QMap<QString, QString> recordFields=record.fields;
  QMap<QString, QByteArray> recordFiles=record.files;

  qDebug() << "Add table record "+recordFields["name"];

  // Запоминается текст записи, полученный из буфера обмена
  QString recordText=recordFields["text"];

  // Поле с текстом записи удаляется из списка инфополей
  recordFields.remove("text");

  qDebug() << "KnowTreeModel::paste_subbranch_recurse() : create branch with field" << newitem->getAllFields();



  newitem->recordtableGetTableData()->insertNewRecord(ADD_NEW_RECORD_TO_END,
                                                         0,
                                                         recordFields,
                                                         recordText,
                                                         recordFiles);
 }

 // --------------------
 // Добавляются подветки
 // --------------------

  // Выясняется линейная структура добавляемого дерева
 QList<CLIPB_TREE_ONE_LINE> tree=subbranch->getIdTree();

 // Выясняется список подветок для стартовой ветки
 QStringList subbranch_list;
 foreach(CLIPB_TREE_ONE_LINE one_line, tree)
  if(one_line.branch_id==startBranchId)
   subbranch_list=one_line.subbranches_id;

 foreach(QString current_subbranch, subbranch_list)
  pasteSubbranchRecurse(newitem, current_subbranch, subbranch);

 return id;
}


// Перешифрование базы с новым паролем
void KnowTreeModel::reEncrypt(QString previousPassword, QString currentPassword)
{
 // Получение путей ко всем подветкам дерева
 QList<QStringList> subbranchespath=rootItem->getAllChildrenPath();
 
 // Перебираются подветки
 foreach(QStringList currPath, subbranchespath)
  {
   // Перешифровываются только те подветки, которые имеют
   // флаг шифрования, и у которых родительская ветка нешифрована
   TreeItem *currBranch=getItem(currPath);
   TreeItem *currBranchParent=currBranch->parent();
    
   if(currBranch->getField("crypt")=="1" && 
      currBranchParent->getField("crypt")!="1")
    {
     Password pwd;

     pwd.setCryptKeyToMemory(previousPassword);
     currBranch->switchToDecrypt();

     pwd.setCryptKeyToMemory(currentPassword);
     currBranch->switchToEncrypt();
    }
    
  } // Закончился перебор подветок


 // Сохранение дерева веток
 find_object<TreeScreen>("treeview")->saveKnowTree();
}


TreeItem *KnowTreeModel::getItemById(QString id)
{
 getItemByIdRecurse(rootItem, 0, 0);

 return getItemByIdRecurse(rootItem, id, 1);
}


TreeItem *KnowTreeModel::getItemByIdRecurse(TreeItem *item, QString id, int mode)
{
 static TreeItem *find_item=NULL;

 if(mode==0)
  {
   find_item=NULL;
   return NULL;
  }

 if(find_item!=NULL) return find_item;

 if(item->getId()==id)
  {
   find_item=item;
   return find_item;
  }
 else
  {
   for(int i=0; i < item->childCount(); i++)
    getItemByIdRecurse(item->child(i), id, 1);

   return find_item;
  }
}


// Получение пути к ветке, где находится запись
QStringList KnowTreeModel::getRecordPath(QString recordId)
{
 getRecordPathRecurse(rootItem, QStringList(), "0", 0);

 return getRecordPathRecurse(rootItem, QStringList(), recordId, 1);
}


QStringList KnowTreeModel::getRecordPathRecurse(TreeItem *item,
                                                QStringList currentPath,
                                                QString recordId,
                                                int mode)
{
 static QStringList findPath;
 static bool isFind;

 if(mode==0)
  {
   findPath.clear();
   isFind=false;
   return QStringList();
  }

 if(isFind)
  return findPath;

 // Путь дополняется
 currentPath << item->getId();

 // Если в данной ветке есть искомая запись
 if(item->getRecordPos(recordId)>=0)
  {
   isFind=true;
   findPath=currentPath;
  }
 else
  {
   // Иначе перебираются подветки
   for(int i=0; i < item->childCount(); i++)
    getRecordPathRecurse(item->child(i), currentPath, recordId, 1);
  }

 return findPath;
}


// Метод определяющий есть ли в дереве зашифрованные ветки
bool KnowTreeModel::isContainsCryptBranches(void)
{
 isContainsCryptBranchesRecurse(rootItem, 0);

 return isContainsCryptBranchesRecurse(rootItem, 1);
}


bool KnowTreeModel::isContainsCryptBranchesRecurse(TreeItem *item, int mode)
{
 static bool isCrypt=false;

 if(mode==0)
  {
   isCrypt=false;
   return isCrypt;
  }

 if(item->getField("crypt")=="1")
  {
   isCrypt=true;
   return isCrypt;
  }

 for(int i=0; i < item->childCount(); i++)
  isContainsCryptBranchesRecurse(item->child(i), 1);

 return isCrypt;
}


// Старый вариант поиска QModelIndex по известному TreeItem закомментирован,
// но алгоритм может пригодиться для поиска других данных
/*
// Получение QModelIndex по известному TreeItem
QModelIndex knowtreemodel::get_item_index(TreeItem *item)
{
 // Выясняется начальный QModelIndex дерева
 QModelIndex rootindex=index( 0, 0 );
 
 // Очищается флаг поиска внутри элементов
 get_item_index_recurse(rootindex, item, 0);
 
 // Перебираются элементы на одном уровне вложения с начальным элементом дерева
 for(int i=0;rootindex.sibling(i,0).isValid();i++)
  {
   // qDebug() << "Sibling current " << (find_object<QTreeView>("knowtree")->model()->data(rootindex.sibling(i,0),Qt::EditRole)).toString();

   // Перебираемый элемент проверяется на соответствие с искомым TreeItem
   if(item==static_cast<TreeItem*>(rootindex.sibling(i,0).internalPointer()))
    return rootindex.sibling(i,0);
   else
    {
     // Производится поиск внутри элемента
     QModelIndex idx=get_item_index_recurse(rootindex.sibling(i,0), item, 1);
    
     // Если был найден элемент
     if(idx.isValid())return idx;
    }
  }
 
 // Если ничего небыло найдено, возвращается пустой индекс
 return QModelIndex();
}


QModelIndex knowtreemodel::get_item_index_recurse(QModelIndex currindex, TreeItem *finditem, int mode)
{
 static QModelIndex findindex;
 static int findflag=0;

 // Из QModelIndex можно всегда получить указатель TreeItem,
 // поэтому поиск можно вести по QModelIndex
 
 // Инициализация поиска
 if(mode==0)
  {
   findflag=0;
   return QModelIndex();
  } 

 // qDebug() << "Recurse current " << (find_object<QTreeView>("knowtree")->model()->data(currindex,Qt::EditRole)).toString();
 // qDebug() << "Current index have " << currindex.row() << "row";
 // qDebug() << "Find flag " << findflag;
 
 // Если был найден QModelIndex  
 if(findflag==1)return findindex;
  
 for(int i=0;currindex.child(i,0).isValid();i++)
  {
   // Проверяется текущий элемент, не соответствует ли 
   // его QModelIndex искомому TreeItem
   if(findflag==0 && 
      finditem==static_cast<TreeItem*>(currindex.child(i,0).internalPointer()))
    {
     findflag=1;
     findindex=currindex.child(i,0);
     return findindex;
    }
   
   // Рекурсивный вызов поиска в глубину дерева
   get_item_index_recurse(currindex.child(i,0), finditem, 1);
   
   // Если был найден QModelIndex  
   if(findflag==1)return findindex;
  }

 // Сюда код доходит если на текущем уровне поиска элемент еще не найден
 return QModelIndex();
}
*/
