#include "obdsur.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OBDSUR w;
    w.show();
    
    return a.exec();
}
