//#define _HAS_STD_BYTE 0
//#include "ims_dialog.h"
#include"CKernel/ckernel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // IMS_Dialog w;
    // w.show();

    //创建核心处理类的对象
    CKernel k;
    return a.exec();
}
