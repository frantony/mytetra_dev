#ifndef __APPCONFIG_H__
#define __APPCONFIG_H__

#include <QObject>
#include <QDir>
#include <QSettings>
#include <QDebug>

class QRect;
class QStringList;

class AppConfig : public QObject
{
    Q_OBJECT

public:
    AppConfig(QObject *pobj=0);
    ~AppConfig();

    void init(void);
    bool is_init(void);

    // Версия формата конфигфайла
    int get_config_version(void);
    void set_config_version(int i);

    // Рабочая директория
    QString get_tetradir(void);
    bool set_tetradir(QString dirName);

    // Директория с корзиной
    QString get_trashdir(void);
    bool set_trashdir(QString dirName);
    
    // Максимальный размер директории корзины в мегабайтах
    unsigned int get_trashsize(void);
    bool set_trashsize(unsigned int mbSize);
    
    // Максимально допустимое число файлов в корзине
    int get_trashmaxfilecount(void);
    bool set_trashmaxfilecount(int count);

    // Нужно ли показывать подтверждение при выполнении действия "cut на ветке
    bool get_cutbranchconfirm(void);
    bool set_cutbranchconfirm(bool confirm);

    // Нужно ли печатать отладочные сообщения в консоль
    bool get_printdebugmessages(void);
    bool set_printdebugmessages(bool isPrint);

    // Язык интерфейса
    QString get_interfacelanguage(void);
    bool set_interfacelanguage(QString language);

    // Номер последнего префикса в виде строки с ведущими нулями
    QString get_addnewrecord_expand_info(void);
    void set_addnewrecord_expand_info(QString);

    // Геометрия основного окна
    QRect get_mainwingeometry(void);
    void set_mainwingeometry(int x, int y, int w, int h);    
    

    QList<int> get_vspl_size_list(void);
    void set_vspl_size_list(QList<int> list);

    QList<int> get_hspl_size_list(void);
    void set_hspl_size_list(QList<int> list);

    QList<int> get_findsplitter_size_list(void);
    void set_findsplitter_size_list(QList<int> list);

    QList<int> get_splitter_size_list(QString name);
    void set_splitter_size_list(QString name, QList<int> list);

    
    QStringList get_tree_position(void);
    void set_tree_position(QStringList list);

    int get_recordtable_position(void);
    void set_recordtable_position(int pos);


    // Настройки виджета поиска по базе
    int get_findscreen_wordregard(void);
    void set_findscreen_wordregard(int pos);

    int get_findscreen_howextract(void);
    void set_findscreen_howextract(int pos);

    bool get_findscreen_find_in_field(QString fieldName);
    void set_findscreen_find_in_field(QString fieldName, bool isChecked);

    bool get_findscreen_show(void);
    void set_findscreen_show(bool isShow);

    QString get_howpassrequest(void);
    void set_howpassrequest(QString mode);

    bool get_runinminimizedwindow(void);
    void set_runinminimizedwindow(bool flag);

    bool get_autoClosePasswordEnable(void);
    void set_autoClosePasswordEnable(bool flag);

    int  get_autoClosePasswordDelay(void);
    void set_autoClosePasswordDelay(int delay);

    QString get_synchrocommand(void);
    void set_synchrocommand(QString command);

    bool get_synchroonstartup(void);
    void set_synchroonstartup(bool flag);

    bool get_synchroonexit(void);
    void set_synchroonexit(bool flag);

    bool getSyncroConsoleDetails(void);
    void setSyncroConsoleDetails(bool flag);
    
    int  getEditorCursorPosition(void);
    void setEditorCursorPosition(int n);

    int  getEditorScrollBarPosition(void);
    void setEditorScrollBarPosition(int n);

    QString getPasswordMiddleHash(void);
    void setPasswordMiddleHash(QString hash);

    bool getPasswordSaveFlag(void);
    void setPasswordSaveFlag(bool flag);


    bool getRememberCursorAtHistoryNavigation(void);
    void setRememberCursorAtHistoryNavigation(bool flag);

    bool getRememberCursorAtOrdinarySelection(void);
    void setRememberCursorAtOrdinarySelection(bool flag);


private:

    QSettings *conf; // было static
    QString get_parameter(QString name);

    QStringList remove_parameter_from_table(QString removeName, QStringList table);

    void update_version_process(void);

    QStringList get_parameter_table_1 (bool withEndSignature=true);
    QStringList get_parameter_table_2 (bool withEndSignature=true);
    QStringList get_parameter_table_3 (bool withEndSignature=true);
    QStringList get_parameter_table_4 (bool withEndSignature=true);
    QStringList get_parameter_table_5 (bool withEndSignature=true);
    QStringList get_parameter_table_6 (bool withEndSignature=true);
    QStringList get_parameter_table_7 (bool withEndSignature=true);
    QStringList get_parameter_table_8 (bool withEndSignature=true);
    QStringList get_parameter_table_9 (bool withEndSignature=true);
    QStringList get_parameter_table_10(bool withEndSignature=true);
    QStringList get_parameter_table_11(bool withEndSignature=true);
    QStringList get_parameter_table_12(bool withEndSignature=true);
    QStringList get_parameter_table_13(bool withEndSignature=true);
    
    bool is_init_flag;
};

#endif // __APPCONFIG_H__
