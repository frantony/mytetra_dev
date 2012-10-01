#ifndef __ADDNEWRECORD_H__
#define __ADDNEWRECORD_H__

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

class Editor;
class InfoFieldEnter;

// Окно добавления новой записи

class AddNewRecord : public QDialog
{
Q_OBJECT

public:
 AddNewRecord( QWidget * parent = 0, Qt::WFlags f = 0 );
 ~AddNewRecord();

 QString getField(QString name);
 QString getImagesDirectory(void);
 void deleteImagesDirectory(void);

private slots:

 void okClick(void);

private:

 // Ввод инфополей записи
 InfoFieldEnter *infoField;

 // Ввод текста записи
 Editor    *recordTextEditor;

 QDialogButtonBox *buttonBox;

 QString imagesDirName;

 void setupUI(void);
 void setupSignals(void);
 void assembly(void);
};

#endif // __ADDNEWRECORD_H__
