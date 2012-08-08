#include "pdfwidget.h"
#include <QImage>
#include <QDebug>
#include <X11/X.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define SAFEFREE(x, y) if(x) { delete y x; x = NULL; }

pdfWidget::pdfWidget(QWidget *parent) :
    QGLWidget(parent), pdf(NULL), page(-1), scale(1),
    pages(NULL), worker(), loader(new pdfLoader()),
    activeTexs(), maxTextures(DEFMAXTEXTURES),
    curTextures(0)
{
    connect(this, SIGNAL(readyLoad(Poppler::Document*)),
            loader, SLOT(loadPDF(Poppler::Document*)));
    connect(loader, SIGNAL(pageLoaded(int,QImage,unsigned,unsigned)),
            this, SLOT(pageLoaded(int,QImage,unsigned,unsigned)));
    loader->moveToThread(&worker);
    worker.start();
}

pdfWidget::~pdfWidget()
{
    loader->stop = true;
    worker.quit();
    if(pages)
    {
        for(int i = 0; i < pdf->numPages(); i++)
            glDeleteTextures(1, &pages[page].texture);
        delete [] pages;
    }
    if(pdf)
    {
        delete pdf;
        pdf = NULL;
    }
    worker.wait();
    delete loader;
}

void pdfWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if(pages && pages[page].valid)
    {
        qDebug() << "Rendering page" << page;
        if(!pages[page].activeTex)
        {
            if(curTextures == maxTextures)
            {
                unsigned initial = activeTexs.first();
                activeTexs.pop_front();
                pages[initial].activeTex = false;
                glDeleteTextures(1, &pages[initial].texture);
            }
            createTex(page);
        }
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
    if(pages)
    {
        for(int i = 0; i < pdf->numPages(); i++)
            glDeleteTextures(i, &pages[i].texture);
        delete [] pages;
        pages = NULL;
    }
    SAFEFREE(pdf,);
    pdf = Poppler::Document::load(pdfName);
    if(!pdf)
        return;
    if(pdf->isLocked())
    {
        SAFEFREE(pdf, );
        return;
    }
    pages = new struct PageData[pdf->numPages()];
    for(int i = 0; i < pdf->numPages(); i++)
    {
        pages[i].valid = false;
        pages[i].activeTex = false;
    }
    page = -1;
    readyLoad(pdf);
}

void pdfWidget::pageLoaded(int page, QImage img, unsigned w, unsigned h)
{
    if(pages)
    {
        qDebug() << "Page loaded" << page;

        pages[page].img = img;
        pages[page].valid = true;
        pages[page].width = w;
        pages[page].height = h;
        pages[page].activeTex = false;
        if(page == -1)
            changePage(page);
    }
}

void pdfWidget::createTex(unsigned pnum)
{
    qDebug() << "Creating texture" << pnum;
    glGenTextures(1, &pages[page].texture);
    glBindTexture(GL_TEXTURE_2D, pages[page].texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 pages[pnum].img.width(),
                 pages[pnum].img.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE,
                 pages[pnum].img.bits());
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_TEXTURE_MAX_LOD);
    activeTexs.append(pnum);
    pages[pnum].activeTex = true;
    curTextures++;
}

void pdfWidget::scalePDF(double scale)
{
    this->scale = scale;
}

void pdfWidget::changePage(int page)
{
    if(page >= 0 && page < pdf->numPages() && pages)
    {
        this->page = page;
        glBindTexture(GL_TEXTURE_2D, pages[page].texture);
        update();
    }
}

int pdfWidget::pageHeight()
{
    if(pages)
        return pages[page].height;
    return -1;
}

int pdfWidget::pageWidth()
{
    if(pages)
        return pages[page].width;
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

void pdfWidget::setMaxTextures(unsigned count)
{
    maxTextures = count;
}

void pdfWidget::setClearColor(const QColor &c)
{
    glClearColor(c.red(), c.green(), c.blue(), c.alpha());
}

pdfLoader::pdfLoader(QObject *parent) :
    QObject(parent), stop(false)
{}

void pdfLoader::loadPDF(Poppler::Document *pdf)
{
    for(int i = 0; i < pdf->numPages() && !stop; i++)
    {
        Poppler::Page *p = pdf->page(i);
        if(!p)
            continue;
        QImage img = QGLWidget::convertToGLFormat(p->renderToImage(220, 220));
        emit pageLoaded(i, img, p->pageSize().width(),
                        p->pageSize().height());
    }
}
