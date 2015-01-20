#ifndef OBDSUR_H
#define OBDSUR_H

#include <QMainWindow>

namespace Ui {
class OBDSUR;
}

class OBDSUR : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit OBDSUR(QWidget *parent = 0);
    ~OBDSUR();
    
private:
    Ui::OBDSUR *ui;
};

#endif // OBDSUR_H
