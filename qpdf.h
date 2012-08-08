#ifndef QPDF_H
#define QPDF_H

#include <QMainWindow>
#include <QFileDialog>
#include <QGraphicsScene>
#include <poppler/qt4/poppler-qt4.h>
#include "pdfwidget.h"

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
    void nextPage();
    void prevPage();
private slots:
    void pdfLoaded(QString pdfName);
    void setScale();
    void setRotation();
    bool changePage();
private:
    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
    double scale;

    Ui::QPdf *ui;
    pdfWidget *pdfView;
    QFileDialog *fd;
};

#endif // QPDF_H
