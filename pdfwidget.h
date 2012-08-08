#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <poppler/qt4/poppler-qt4.h>
#include <QThread>
#include <QList>

#ifndef DEFMAXTEXTURES
#define DEFMAXTEXTURES 50
#endif

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
    void setMaxTextures(unsigned);
signals:
    void pdfLoaded(QString name);
    void readyLoad(Poppler::Document *pdf);

public slots:
    void loadPDF(QString pdfName);
    void scalePDF(double scale);
    void rotatePDF(double angle);
    void changePage(int page);

protected:
    void paintGL();
    void resizeGL(int w, int h);
    void initializeGL();

private slots:
    void pageLoaded(int page, QImage img, unsigned w, unsigned h);

private:
    void createTex(unsigned pnum);

    Poppler::Document *pdf;
    int page;
    double scale;
    double rotation;
    struct PageData {
        bool valid;
        bool activeTex;
        unsigned width;
        unsigned height;
        GLuint texture;
        QImage img;
    } *pages;

    QThread worker;
    pdfLoader *loader;
    QList<unsigned> activeTexs;
    unsigned maxTextures, curTextures;
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
