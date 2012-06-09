#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <poppler/qt4/poppler-qt4.h>

class pdfWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit pdfWidget(QWidget *parent = 0);
    ~pdfWidget();

    int pageCount();
    int docWidth();
    int docHeight();
signals:
    
public slots:
    void loadPDF(QString pdfName);
    void scalePDF(double scale);
    void changePage(int page);

protected:
    void paintGL();
    void resizeGL(int w, int h);
    void initializeGL();

private:
    Poppler::Document *pdf;
    int page;
    bool *validPages;
    GLuint *textures;
    GLsizei textureCount;
};

#endif // PDFWIDGET_H
