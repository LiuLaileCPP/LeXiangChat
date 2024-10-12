#ifndef WD_MSGME_H
#define WD_MSGME_H

#include <QWidget>

namespace Ui {
class wd_msgMe;
}

class wd_msgMe : public QWidget
{
    Q_OBJECT

public:
    explicit wd_msgMe(QWidget *parent = nullptr);
    ~wd_msgMe();
    void setInfo(int cli_iconId,QString content,QString name);
private:
    Ui::wd_msgMe *ui;
    //---------------------------------------------------------------
signals:


    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:




    //---------------------------------------------------------------
};

#endif // WD_MSGME_H
