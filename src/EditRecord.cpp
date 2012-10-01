#include <QWidget>
#include <QTextEdit>
#include <QtDebug>
#include <QSizePolicy>
#include <QIcon>
#include <QMessageBox>
#include <QTextDocumentFragment>

#include "InfoFieldEnter.h"
#include "EditRecord.h"
#include "main.h"


EditRecord::EditRecord( QWidget * parent, Qt::WFlags f) : QDialog(parent, f)
{
 setupUI();
 setupSignals();
 assembly();
}


EditRecord::~EditRecord()
{

}


void EditRecord::setupUI(void)
{
 // Ввод инфополей записи
 infoField=new InfoFieldEnter();

 // Кнопки OK и Cancel
 buttonBox=new QDialogButtonBox();
 buttonBox->setOrientation(Qt::Horizontal);
 buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::NoButton|QDialogButtonBox::Cancel);
}


void EditRecord::setupSignals(void)
{
 connect(buttonBox, SIGNAL(accepted()), this, SLOT(okClick(void)));
 connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


void EditRecord::assembly(void)
{
 // Размещалка элементов
 QVBoxLayout *layout=new QVBoxLayout();
 layout->setMargin(8);
 layout->setSpacing(10);

 // Добавление элементов в размещалку
 layout->addWidget(infoField);
 layout->addWidget(buttonBox,0,Qt::AlignRight);

 layout->setSizeConstraint(QLayout::SetFixedSize);

 setLayout(layout);

 // setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));

 // Фокус устанавливается на поле ввода названия записи
 infoField->setFocusToStart();


 // QWidget *wdgt=new QWidget;
 // wdgt->setLayout(vbl);
 // setCentralWidget(wdgt);
}


void EditRecord::okClick(void)
{
 QString message="";

 // Проверка наличия названия записи
 if(infoField->getField("name").length()==0)
  message=message+tr("Please enter the note's <b>title</b>.");

 // Если что-то не заполнено, выдается предупреждение
 if(message.length()>0)
  {
   QMessageBox::warning(this,tr("The note's fields cannot be modified"),message,
                        QMessageBox::Close);
   return;
  }
 else
  {
   emit(accept());
  }
}


QString EditRecord::getField(QString name)
{
 if(infoField->checkFieldName(name)==true)
  return infoField->getField(name);
 else
  critical_error("Can not get field "+name+" in editrecord method get_field");

 return QString();
}


void EditRecord::setField(QString name, QString value)
{
 if(infoField->checkFieldName(name)==true)
  infoField->setField(name,value);
 else
  critical_error("Can not set field "+name+" in editrecord method set_field");
}

