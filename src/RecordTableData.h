#ifndef __RECORDTABLEDATA_H__
#define	__RECORDTABLEDATA_H__

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>
#include <QObject>
#include <QtXml>
#include <QMap>
#include <QString>
#include <QByteArray>

class TreeItem;


class RecordTableData : public QObject
{
    Q_OBJECT

public:
    RecordTableData(QObject *pobj=0);
    virtual ~RecordTableData();

    // Получение текста указанной записи
    QString getText(int pos) const;

    // QByteArray get_text_as_byte_array(int pos) const;

    // Установка текста указанной записи
    void setText(int pos,
                 const QString &text,
                 const QMap<QString, QByteArray> &files=(QMap<QString, QByteArray>()) );
    /*
    void set_text(int pos,
                  const QByteArray &text,
                  const QMap<QString, QByteArray> &files=(QMap<QString, QByteArray>()) );
    */

    bool checkAndFillFileDir(int pos, QString &nameDirFull, QString &nameFileFull);
    
    // Получение значения указанного поля для указанного элемента
    QString getField(QString name, int pos) const;

    // Установка значения указанного поля для указанного элемента
    void setField(QString name, QString value, int pos);
   
    // Получение всех инфополей указанного элемента
    QMap<QString, QString> getFields(int pos) const;
    
    // Получение полного образа записи
    QMap<QString, QString> getRecordExemplar(int pos) const;

    // Первичное заполнение таблицы конечных записей
    void init(TreeItem *item, QDomElement domModel);

    // Удаление всех элементов таблицы конечных записей
    void deleteAllRecords(void);
    
    // Количество записей в таблице данных
    int size(void) const;

    // Функция создания DOM-документа из данных таблицы конечных записей
    QDomDocument exportDataToDom(void);

    // Получение ссылки на объект ветки, которой принадлежит таблица
    TreeItem *getItem(void);

    int insertNewRecord(int mode,
                        int pos,
                        QMap<QString, QString> fields,
                        QString text,
                        QMap<QString, QByteArray> files = (QMap<QString, QByteArray>()) );

    void editRecordFields(int pos,
                     QMap<QString, QString> editFields);

    void deleteRecord(int i);

    void empty(void);

    void moveUp(int pos);
    void moveDn(int pos);

    bool isFieldNameAvailable(QString name) const;
    QStringList fieldNameAvailableList(void) const;
    QStringList fieldNameForCryptList(void) const;

    void switchToEncrypt(void);
    void switchToDecrypt(void);

    static void editorLoadCallback(QObject *editor,
                                     QString &loadText);

    static void editorSaveCallback(QObject *editor,
                                     QString saveText);

    int  getWorkPos(void);
    void setWorkPos(int pos);

private:
    // Функция заполнения таблицы из DOM-документа
    void setupDataFromDom(QDomElement *domModel);

    QMap<QString, QString> getMergeFields(int pos, QMap<QString, QString> fields);

    void setTextSmart(int pos,
                      int mode,
                      const QString &textString,
                      const QByteArray &textByteArray,
                      const QMap<QString, QByteArray> &files);

    // QByteArray get_text_smart(int pos) const;

    // Таблица с инфополями записей
    QList< QMap<QString, QString> > fieldsTable;

    // Ссылка на ветку, которой принадлежит данная таблица
    TreeItem *treeItem;

    // Номер записи, с которой работал пользователь
    int workPos;
};

#endif /* __RECORDTABLEDATA_H__ */

