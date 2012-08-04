#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <poppler/qt4/poppler-qt4.h>
#include <QThread>

class pdfLoader;

class pdfWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit pdfWidget(QWidget *parent = 0);
    ~pdfWidget();

    int pageCount();
    int pageNumber();
    int pageWidth();
    int pageHeight();

    void setClearColor(const QColor &c);
signals:
    void pdfLoaded();
    void readyLoad(Poppler::Document *pdf);

public slots:
    void loadPDF(QString pdfName);
    void scalePDF(double scale);
    void changePage(int page);

protected:
    void paintGL();
    void resizeGL(int w, int h);
    void initializeGL();

private slots:
    void pageLoaded(int page, QImage img, unsigned w, unsigned h);

private:
    Poppler::Document *pdf;
    int page;
    double scale;
    struct PageData {
        bool valid;
        unsigned width;
        unsigned height;
        GLuint texture;
    } *pages;

    QThread worker;
    pdfLoader *loader;
};

class pdfLoader : public QObject
{
    Q_OBJECT
public:
    pdfLoader(QObject *parent = 0);
    bool stop;
public slots:
    void loadPDF(Poppler::Document *pdf);
signals:
    void pageLoaded(int page, QImage, unsigned, unsigned);
};

#endif // PDFWIDGET_H
