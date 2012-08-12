#include "pdfwidget.h"
#include <QImage>
#include <QDebug>
#include <X11/X.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <cmath>

#define SAFEFREE(x, y) if(x) { delete y x; x = NULL; }

pdfWidget::pdfWidget(QWidget *parent) :
    QGLWidget(parent), pdf(NULL), page(0), scale(0),
    pages(NULL), worker(), loader(new pdfLoader())
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
    QColor back = palette().background().color();
    glClearColor(back.redF(), back.greenF(),
                 back.blueF(), back.alphaF());
    glClear(GL_COLOR_BUFFER_BIT);
    if(pages && page < pdf->numPages() &&
            page >= 0 && pages[page].valid)
    {
        qDebug() << "Rendering page" << page << pages[page].texture;
        glLoadIdentity();
        double x = pageWidth(), y = pageHeight();
        if(scale > 0)
            glScalef(scale / 100, scale / 100, 0);
        else
        {
            /* Right triangle math:
             * Calculate our "real" x and y values
             * with rotation mathematics
             * x' = mag * sin(t + t')
             *    = mag * (sin(t) * cos(t') + cos(t) * sin(t'))
             *    = x * cos(t') + y * sin(t');
             * y' = mag * cos(t + t')
             *    = mag * (cos(t) * cos(t') - sin(t) * sin(t'))
             *    = y * cos(t') - x * sin(t');
             * We always add, because the rotation affects
             * different corners in different ways.
             * We need the greatest x and y values of the corners */
            double radians = rotation * M_PI / 180,
                   realx = fabs(x * sin(radians))
                    + fabs(y * cos(radians)),
                   realy = fabs(y * cos(radians))
                    + fabs(x * sin(radians));
            if(width() / realx < height() / realy)
                glScalef(width() / realx, width() / realx, 0);
            else
                glScalef(height() / realy, height() / realy, 0);
        }
        x /= 2; y /= 2;
        glRotated(rotation, 0.0, 0.0, 1.0);
        glBindTexture(GL_TEXTURE_2D, pages[page].texture);
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
    //Enable antialiasing
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glClearColor(0, 0, 0, 0);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    // when texture area is large, bilinear filter the original
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // the texture wraps over at the edges (repeat)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    readyGL();
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
        glGenTextures(1, &pages[i].texture);
    page = -1;
    readyLoad(pdf);
    pdfLoaded(pdfName);
}

void pdfWidget::pageLoaded(int p, QImage img, unsigned w, unsigned h)
{
    if(pages)
    {
        qDebug() << "Page loaded" << p;
        glBindTexture(GL_TEXTURE_2D, pages[p].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     img.width(), img.height(), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_TEXTURE_MAX_LOD);

        pages[p].valid = true;
        pages[p].width = w;
        pages[p].height = h;
        if(this->page == -1)
            changePage(p);
    }
}

void pdfWidget::scalePDF(double scale)
{
    this->scale = scale;
    this->repaint();
}

void pdfWidget::rotatePDF(double angle)
{
    this->rotation = angle;
    this->repaint();
}

void pdfWidget::changePage(int page)
{
    if(page >= 0 && page < pdf->numPages() && pages)
    {
        this->page = page;
        glBindTexture(GL_TEXTURE_2D, pages[page].texture);
        repaint();
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

void pdfWidget::setClearColor(const QColor &c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, c);
    setPalette(pal);
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
        QImage prePage = p->renderToImage(144, 144);
        QImage finalPage = QGLWidget::convertToGLFormat(prePage);
        emit pageLoaded(i, finalPage, p->pageSize().width(),
                        p->pageSize().height());
    }
}
