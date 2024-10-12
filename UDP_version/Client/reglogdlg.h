#ifndef REGLOGDLG_H
#define REGLOGDLG_H

#include <QMainWindow>
#include <QString>
namespace Ui {
class ReglogDlg;
}

class ReglogDlg : public QMainWindow
{
    Q_OBJECT

public:
    explicit ReglogDlg(QWidget *parent = nullptr);
    ~ReglogDlg();
    void setInfo(int id,int iconId,char* name,char* sign);
private:
    Ui::ReglogDlg *ui;
    //---------------------------------------------------------------
signals:
    void SIG_registerCommit(QString account,QString password,int type);
    void SIG_loginCommit(QString account,QString password,int type);
    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:
    void slot_regist(bool res,QString errMsg);

    void slot_login(bool res,QString errMsg);

    //---------------------------------------------------------------
    void on_pb_regist_clicked();
    void on_pb_login_clicked();
};

#endif // REGLOGDLG_H
