#include "pdfwidget.h"
#include <GL/glu.h>
#include <QImage>

pdfWidget::pdfWidget(QWidget *parent) :
    QGLWidget(parent), page(0), pdf(NULL),
    textures(NULL), validPages(NULL)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

pdfWidget::~pdfWidget()
{
    if(pdf)
        delete pdf;
    if(textures)
    {
        glDeleteTextures(textureCount, textures);
        delete[] textures;
    }
    if(validPages)
        delete[] validPages;
}

void pdfWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0); glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2d(1.0, 0.0); glVertex3f(1.0f, 0.0f, 0.0f);
    glTexCoord2d(1.0, 1.0); glVertex3f(1.0f, 1.0f, 0.0f);
    glTexCoord2d(0.0, 1.0); glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();
    glFlush();
}

void pdfWidget::initializeGL()
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the original
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // the texture wraps over at the edges (repeat)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

void pdfWidget::loadPDF(QString pdfName)
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
        textures = NULL;
    }
    if(validPages)
        delete[] validPages;
    pdf = Poppler::Document::load(pdfName);
    if(!pdf)
        return;
    if(pdf->isLocked())
    {
        delete pdf;
        return;
    }
    textureCount = pdf->numPages();
    textures = new GLuint[textureCount];
    validPages = new bool[textureCount];
    glGenTextures(textureCount, textures);
    for(int i = 0; i < pdf->numPages(); i++)
    {
        Poppler::Page *p = pdf->page(i);
        if(!p)
        {
            validPages[i] = false;
            continue;
        }
        validPages[i] = true;
        QImage img = p->renderToImage(288, 288);
        gluBuild2DMipmaps( GL_TEXTURE_2D, 3, img.width(), img.height(), GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    }
}

void pdfWidget::scalePDF(double scale)
{

}

void pdfWidget::changePage(int page)
{
    if(page >= 0 && page < pdf->numPages())
    {
        glBindTexture(GL_TEXTURE_2D, textures[page]);
        update();
    }
}

int pdfWidget::docHeight()
{
}

int pdfWidget::docWidth()
{

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
