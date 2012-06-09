#include "pdfwidget.h"
#include <GL/glu.h>
#include <QImage>
#include <QDebug>

#define SAFEFREE(x, y) if(x) { delete y x; x = NULL; }

pdfWidget::pdfWidget(QWidget *parent) :
    QGLWidget(parent), pdf(NULL), page(0),
    textures(NULL), textureCount(0), scale(1),
    validPages(NULL), pageW(NULL), pageH(NULL)
{}

pdfWidget::~pdfWidget()
{
    if(pdf)
    {
        delete pdf;
        pdf = NULL;
    }
    if(textures)
    {
        glDeleteTextures(textureCount, textures);
        delete[] textures;
    }
    SAFEFREE(validPages, []);
    SAFEFREE(pageW, []);
    SAFEFREE(pageH, []);
}

void pdfWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if(validPages && validPages[page])
    {
        float x = pageWidth(), y = pageHeight();
        x /= 2; y /= 2;
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0); glVertex3f(-x, -y, 0.0f);
        glTexCoord2d(1.0, 0.0); glVertex3f(x, -y, 0.0f);
        glTexCoord2d(1.0, 1.0); glVertex3f(x, y, 0.0f);
        glTexCoord2d(0.0, 1.0); glVertex3f(-x, y, 0.0f);
        glEnd();
    }
    glFlush();
}

void pdfWidget::initializeGL()
{
    glEnable(GL_TEXTURE_2D);
    glClearColor(0, 0, 0, 0);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    // when texture area is large, bilinear filter the original
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // the texture wraps over at the edges (repeat)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void pdfWidget::loadPDF(QString pdfName)
{
    SAFEFREE(pdf,);
    if(textures)
    {
        glDeleteTextures(textureCount, textures);
        delete[] textures;
        textures = NULL;
    }
    SAFEFREE(validPages, []);
    SAFEFREE(pageW, []);
    SAFEFREE(pageH, []);
    pdf = Poppler::Document::load(pdfName);
    if(!pdf)
        return;
    if(pdf->isLocked())
    {
        delete pdf;
        pdf = NULL;
        return;
    }
    textureCount = pdf->numPages();
    textures = new GLuint[textureCount];
    validPages = new bool[textureCount];
    pageW = new int[textureCount];
    pageH = new int[textureCount];
    for(int i = 0; i < textureCount; i++)
        textures[i] = 0;
    glGenTextures(textureCount, textures);
    for(int i = 0; i < pdf->numPages(); i++)
    {
        qDebug() << textures[i];
        Poppler::Page *p = pdf->page(i);
        if(!p)
        {
            validPages[i] = false;
            continue;
        }
        validPages[i] = true;
        pageW[i] = p->pageSize().width();
        pageH[i] = p->pageSize().height();
        QImage img = QGLWidget::convertToGLFormat(p->renderToImage(220, 220));
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MAX_LOD);
    }
    page = 0;
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    update();
    emit pdfLoaded();
}

void pdfWidget::scalePDF(double scale)
{
    this->scale = scale;
}

void pdfWidget::changePage(int page)
{
    if(page >= 0 && page < pdf->numPages())
    {
        this->page = page;
        glBindTexture(GL_TEXTURE_2D, textures[page]);
        update();
    }
}

int pdfWidget::pageHeight()
{
    if(validPages && validPages[page])
        return pageH[page];
    return -1;
}

int pdfWidget::pageWidth()
{
    if(validPages && validPages[page])
        return pageW[page];
    return -1;
}

void pdfWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* Set up (0, 0) in the center of the widget */
    double negw = -w, negh = -h, width = w, height = h;
    negw /= 2; negh /= 2; width /= 2; height /= 2;
    gluOrtho2D(negw, width, negh, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int pdfWidget::pageCount()
{
    if(pdf)
        return pdf->numPages();
    return -1;
}

int pdfWidget::pageNumber()
{
    if(pdf)
        return page;
    return -1;
}

void pdfWidget::setClearColor(const QColor &c)
{
    glClearColor(c.red(), c.green(), c.blue(), c.alpha());
}
