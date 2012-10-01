#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QInputDialog>

#include "main.h"
#include "GlobalParameters.h"

#include "TreeScreen.h"
#include "MetaEditor.h"
#include "RecordTableScreen.h"
#include "FindScreen.h"


GlobalParameters::GlobalParameters(QObject *pobj)
{
 Q_UNUSED(pobj);

}

GlobalParameters::~GlobalParameters()
{

}


void GlobalParameters::setMainProgramFile(QString file)
{
 mainProgramFile=file;
}


void GlobalParameters::init(void)
{
 pointTreeScreen=NULL;
 pointRecordTableScreen=NULL;
 pointFindScreen=NULL;
 pointMetaEditor=NULL;
 pointStatusBar=NULL;

 initWorkDirectory(); // Инициализация рабочей директории
}



void GlobalParameters::initWorkDirectory(void)
{
 // Если рабочая директория найдена автоматически
 if(findWorkDirectory()) return;

 // Рабочая директория не найдена, и нужно создать начальные данные

 // Проверяется, можно ли создавать файлы в директории бинарника
 // То есть, возможно ли создание "портируемой" версии

 bool enablePortable=false;

 // Путь к директории, где лежит бинарник
 QFileInfo mainProgramFileInfo(mainProgramFile);
 QString fullCurrentPath=mainProgramFileInfo.absolutePath();

 // Проверяется, можно ли читать и писать файлы в этой директории
 QDir currentBinaryDir(fullCurrentPath);
 if(currentBinaryDir.isReadable())
  {
   QFile file(currentBinaryDir.filePath("seed.txt"));
   if(file.open(QIODevice::ReadWrite))
    {
     enablePortable=true; // Портируемую версию можно создавать
     file.close();
     file.remove();
    }
  }

 QString dataDirName=".config/"+getApplicationName();

 QString welcomeText=tr("Welcome to MyTetra v.")+QString::number(APPLICATION_RELEASE_VERSION)+'.'+QString::number(APPLICATION_RELEASE_SUBVERSION)+'.'+QString::number(APPLICATION_RELEASE_MICROVERSION)+"!";

 QString standartText=tr("Create subdirectory  \"%1\"\nin user directory  \"%2\",\nand create application files in it.").arg(dataDirName).arg(QDir::homePath());

 QString portableText=tr("Create application files\nin current directory  \"%1\".").arg(fullCurrentPath);

 // Если возможно создать только стандартную версию файлового окружения
 if(enablePortable==false)
  {
   QString infoText=tr("The following actions will be performed before running this application: \n\n")+
                    standartText+"\n\n"+
                    tr("Do you agree to perform these?");

   QMessageBox msgBox;
   msgBox.setText(welcomeText);
   msgBox.setInformativeText(infoText);
   msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
   msgBox.setDefaultButton(QMessageBox::Ok);
   int ret = msgBox.exec();

   if(ret==QMessageBox::Ok)
    createStandartProgramFiles();
   else
    exit(0); // Была нажата отмена
  }
 else
  {
   // Иначе есть возможность создать как стандартное файловое окружение,
   // так и "переносимое"

   QString infoText=welcomeText+"\n\n"+
                    tr("Please, select application mode: \n\n")+
                    tr("Standart:\n")+standartText+"\n\n"+
                    tr("Portable:\n")+portableText+"\n\n";

   QStringList items;
   QString standartItem=tr("Standart");
   QString portableItem=tr("Portable");
   items << standartItem << portableItem;

   bool ok;

   // QInputDialog inputDialog;
   // inputDialog.setLabelText("<b>"+welcomeText+"</b>\n\n"+infoText);
   // inputDialog.setComboBoxItems(items);

   QWidget *tempWidget=new QWidget();
   QString item=QInputDialog::getItem(tempWidget,
                                      welcomeText,
                                      infoText,
                                      items, 0, false, &ok);
   delete tempWidget;

   // Если пользователь сделал выбор
   if(ok && !item.isEmpty())
    {
     if(item==standartItem)
      createStandartProgramFiles();
     else
      createPortableProgramFiles();
    }
   else
    exit(0); // Была нажата отмена

  }

 // Заново запускается поиск рабочей директории
 workDirectory="";
 findWorkDirectory();
}


void GlobalParameters::createStandartProgramFiles(void)
{
 qDebug() << "Create standart program files";

 QDir userDir=QDir::home();
 QString dataDirName=".config/"+getApplicationName();
 if(userDir.mkpath(dataDirName))
  {
   QString createFilePath=QDir::homePath()+"/"+dataDirName;

   createFirstProgramFiles(createFilePath);
  }
 else
  {
   critical_error("Can not created directory \""+dataDirName+"\" in user directory \""+QDir::homePath()+"\"");
   exit(0);
  }
}


void GlobalParameters::createPortableProgramFiles(void)
{
 qDebug() << "Create portable program files";

 // Путь к директории, где лежит бинарник
 QFileInfo mainProgramFileInfo(mainProgramFile);
 QString createFilePath=mainProgramFileInfo.absolutePath();

 createFirstProgramFiles(createFilePath);
}


// Создание первоначального набора файлов в указанной директории
void GlobalParameters::createFirstProgramFiles(QString dirName)
{
 qDebug() << "Create first program files in directory " << dirName;

 QDir dir(dirName);

 // Создается дерево директорий в указанной директории
 dir.mkpath("data/base/0000000001");
 dir.mkdir("trash");

 // Создаются файлы конфигурации
 QString targetOs=getTargetOs(); // "any" или "meego"

 QFile::copy(":/resource/standartconfig/"+targetOs+"/conf.ini", dirName+"/conf.ini");
 QFile::setPermissions(dirName+"/conf.ini", QFile::ReadUser | QFile::WriteUser);

 QFile::copy(":/resource/standartconfig/"+targetOs+"/editorconf.ini", dirName+"/editorconf.ini");
 QFile::setPermissions(dirName+"/editorconf.ini", QFile::ReadUser | QFile::WriteUser);


 // Создается файл базы данных
 QFile::copy(":/resource/standartdata/mytetra.xml", dirName+"/data/mytetra.xml");
 QFile::setPermissions(dirName+"/data/mytetra.xml", QFile::ReadUser | QFile::WriteUser);

 // Создается файл первой записи
 QFile::copy(":/resource/standartdata/base/0000000001/text.html", dirName+"/data/base/0000000001/text.html");
 QFile::setPermissions(dirName+"/data/base/0000000001/text.html", QFile::ReadUser | QFile::WriteUser);
}


// Автоопределение рабочей директории
bool GlobalParameters::findWorkDirectory(void)
{
 // Поиск файла conf.ini в той же директории, где находится бинарник

 // Нужно учесть, что программу могут запускать из другой директории
 // QDir::currentPath() - выдает директорию, где была выполнена команда запуска
 // mainProgramFile - содержит путь к бинарнику относительно директории запуска

 // Директория, где была выполнена команда запуска
 QFileInfo mainProgramFileInfo(mainProgramFile);
 QString fullCurrentPath=mainProgramFileInfo.absolutePath();
 qDebug() << "Check full current path " << fullCurrentPath;

 if(isMytetraIniConfig(fullCurrentPath+"/conf.ini")==true)
  {
   qDebug() << "Work directory set to path " << fullCurrentPath;

   // QDir dir=QDir("./");
   // QDir dir=QDir(QDir::currentPath());
   // workDirectory=dir.absolutePath();
   workDirectory=fullCurrentPath;
  }
 else
  {
   // Если в текущей директории запуска нет conf.ini

   // Поиск файла conf.ini в домашней директории пользователя
   // в поддиректории ".имя_программы"
   QString dir=QDir::homePath()+"/."+getApplicationName();

   qDebug() << "Detect home directory " << dir;

   // Если директория существует и в ней есть настоящий файл конфигурации
   if(isMytetraIniConfig(dir+"/conf.ini")==true)
    workDirectory=dir;
   else
    {
     // Иначе директории "~/.имя_программы" нет
     // и нужно пробовать найти данные в "~/.config/имя_программы"
     dir=QDir::homePath()+"/.config/"+getApplicationName();

     qDebug() << "Detect home directory " << dir;

     // Если директория существует и в ней есть настоящий файл конфигурации
     if(isMytetraIniConfig(dir+"/conf.ini")==true)
      workDirectory=dir;
    }
  }

 // Если рабочая директория не определена
 if(workDirectory.length()==0)
  {
   return false;
  }
 else
  {
   // Иначе рабочая директория установлена
   qDebug() << "Set work directory to " << workDirectory;

   // Устанавливается эта директория как рабочая
   if(QDir::setCurrent(workDirectory))
    return true;
   else
    {
     critical_error("Can not set work directory as '"+workDirectory+"'. System problem.");
     return false;
    }
  }
}


// Проверка ini-файла
bool GlobalParameters::isMytetraIniConfig(QString fileName)
{
 qDebug() << "Check config file " << fileName;

 QFileInfo info(fileName);

 // Если файл существует, надо определить, от MyTetra он или от другой программы
 if(info.exists())
  {
   qDebug() << "Config file " << fileName << " is exists";

   // Выясняется имя файла без пути к директории
   QString shortFileName=info.fileName();
   qDebug() << "Short config file name " << shortFileName;

   // Выясняется имя директории из имени файла
   QDir dir=info.dir();
   QString dirName=dir.absolutePath();
   qDebug() << "Config directory name " << dirName;

   // Открывается хранилище настроек
   QSettings *conf=new QSettings(fileName, QSettings::IniFormat);
   // conf->setPath(QSettings::IniFormat, QSettings::UserScope, dirName);
   // conf->setPath(QSettings::IniFormat, QSettings::SystemScope, dirName);

   // Если есть переменная version
   if(conf->contains("version"))
    {
     int version=conf->value("version").toInt();

     // Если номер версии конфига до 3 включительно
     if(version<=3)
      {
       // В этих версиях небыло переменной programm, поэтому проверяется
       // переменная tetradir
       if(conf->contains("tetradir")) return true;
       else return false;
      }
     else
      {
       // Иначе номер версии больше 3
       if(conf->contains("programm"))
        {
         if(conf->value("programm").toString()=="mytetra") return true;
         else return false;
        }
       else return false;
      }
    }
   else
    return false; // Нет переменной version
  }
 else
  return false; // Нет указанного файла
}


QString GlobalParameters::getWorkDirectory(void)
{
 return workDirectory;
}


QString GlobalParameters::getTargetOs(void)
{
#ifndef TARGET_OS_MEEGO
 return "any";
#else
 return "meego";
#endif
}


// Имя программы в системе
// Используется для создания и поиска каталога с данными пользователя
QString GlobalParameters::getApplicationName(void)
{
 qDebug() << "In getApplicationName() call getTargetOs() return " << getTargetOs();
 
 if(getTargetOs()!="meego")
  {
   qDebug() << "In getApplicationName() return \"mytetra\"";
   return "mytetra";
  }
 else
  {
   qDebug() << "In getApplicationName() return \"ru.webhamster.mytetra\"";
   return "ru.webhamster.mytetra";
  }
}


void GlobalParameters::setTreeScreen(TreeScreen *point)
{
 pointTreeScreen=point;
}

TreeScreen *GlobalParameters::getTreeScreen()
{
 return pointTreeScreen;
}


void GlobalParameters::setRecordTableScreen(RecordTableScreen *point)
{
 pointRecordTableScreen=point;
}

RecordTableScreen *GlobalParameters::getRecordTableScreen()
{
 return pointRecordTableScreen;
}


void GlobalParameters::setFindScreen(FindScreen *point)
{
 pointFindScreen=point;
}

FindScreen *GlobalParameters::getFindScreen()
{
 return pointFindScreen;
}


void GlobalParameters::setMetaEditor(MetaEditor *point)
{
 pointMetaEditor=point;
}

MetaEditor *GlobalParameters::getMetaEditor()
{
 return pointMetaEditor;
}


void GlobalParameters::setStatusBar(QStatusBar *point)
{
 pointStatusBar=point;
}

QStatusBar *GlobalParameters::getStatusBar()
{
 return pointStatusBar;
}


void GlobalParameters::setCryptKey(QByteArray hash)
{
 passwordHash=hash;
}


QByteArray GlobalParameters::getCryptKey(void)
{
 return passwordHash;
}
