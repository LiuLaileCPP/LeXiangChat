#ifndef WD_MSG_H
#define WD_MSG_H

#include <QWidget>

namespace Ui {
class wd_msg;
}

class wd_msg : public QWidget
{
    Q_OBJECT

public:
    explicit wd_msg(QWidget *parent = nullptr);
    ~wd_msg();
    void setInfo(int cli_iconId,QString content,QString name);
private:
    Ui::wd_msg *ui;
    //---------------------------------------------------------------
signals:


    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:




    //---------------------------------------------------------------
};

#endif // WD_MSG_H
