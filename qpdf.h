#ifndef QPDF_H
#define QPDF_H

#include <QMainWindow>
#include <QFileDialog>
#include <QGraphicsScene>
#include <poppler/qt4/poppler-qt4.h>

namespace Ui {
class QPdf;
}

class QPdf : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit QPdf(QWidget *parent = 0);
    ~QPdf();
public slots:
    void showFiles();
    void openFile(QString file);
    void nextPage();
    void prevPage();
    bool changePage();
    void setScale();
private:
    void keyPressEvent(QKeyEvent *e);
    double scale;

    Ui::QPdf *ui;
    QGraphicsScene *scene;
    Poppler::Document *pdf;
    QFileDialog *fd;
    int pageNum;
};

#endif // QPDF_H
