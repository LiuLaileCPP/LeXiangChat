#ifndef MAINDLG_H
#define MAINDLG_H

#include <QWidget>
#include <item.h>
#include <chatdlg.h>
#include <QVBoxLayout>
namespace Ui {
class MainDlg;
}

class MainDlg : public QWidget
{
    Q_OBJECT

public:
    explicit MainDlg(QWidget *parent = nullptr);
    ~MainDlg();
    void addItem(item* item);
    void addDlg(chatdlg* dlg);
    void setInfo(int id,int iconId,char* name,char* sign);
    QVBoxLayout* m_layout;
    QVBoxLayout* m_layout2;
private:
    Ui::MainDlg *ui;
    //---------------------------------------------------------------
signals:


    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:




    //---------------------------------------------------------------
};

#endif // MAINDLG_H
