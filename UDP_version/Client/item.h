#ifndef ITEM_H
#define ITEM_H

#include <QDialog>

namespace Ui {
class item;
}

class item : public QDialog
{
    Q_OBJECT

public:
    item(QWidget *parent = nullptr);
    ~item();
    void setInfo(bool state,int iconId,QString name);
    void setClrName(int);
private:
    Ui::item *ui;

protected:
    void mousePressEvent(QMouseEvent* ev);


    //---------------------------------------------------------------
signals:
    void SIG_clicked();

    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:




    //---------------------------------------------------------------
};

#endif // ITEM_H
