#include "qpdf.h"
#include "ui_qpdf.h"
#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>

#include "pdfwidget.h"

QPdf::QPdf(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QPdf),
    pdf(NULL),
    scene(new QGraphicsScene(this)),
    fd(new QFileDialog(this)),
    scale(1.0)
{
    ui->setupUi(this);
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->setFilter("*.pdf");
    fd->setFileMode(QFileDialog::ExistingFile);
    connect(fd, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(palette().brush(QPalette::Window));
    ui->graphicsView->scale(0.5, 0.5);
    pdfWidget *p = new pdfWidget(this);
    layout()->addWidget(p);
}

QPdf::~QPdf()
{
    if(pdf)
        delete pdf;
    delete ui;
}

void QPdf::showFiles()
{
    fd->show();
}

void QPdf::openFile(QString file)
{
    if(pdf)
        delete pdf;
    pdf = Poppler::Document::load(file);
    if(!pdf)
        return;
    if(pdf->isLocked())
    {
        QMessageBox *msg = new QMessageBox(this);
        msg->setWindowTitle("QPdf");
        msg->setText(file + " is locked");
        msg->show();
        delete pdf;
    }
    setWindowTitle(QString("QPdf: ") + file);
    pageNum = 0;
    ui->linePage->setText(QString().sprintf("%d", pageNum + 1));
    ui->labelPages->setText(QString().sprintf("/ %d", pdf->numPages()));
    changePage();
}

void QPdf::keyPressEvent(QKeyEvent *e)
{
    qDebug() << e->key();
    this->setFocus();
    switch(e->key())
    {
    case Qt::Key_Right:
        nextPage();
        break;
    case Qt::Key_Left:
        prevPage();
        break;
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    }
}

bool QPdf::changePage()
{
    bool chk;
    int tmp = ui->linePage->text().toInt(&chk) - 1;
    if(!chk)
        return false;
    if(!pdf || tmp >= pdf->numPages() || tmp < 0)
        return false;
    Poppler::Page *page = pdf->page(tmp);
    if(!page)
        return false;
    pageNum = tmp;
    QImage img = page->renderToImage(144, 144);
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(img));
    scene->addRect(0, 0, img.width(), img.height());
    return true;
}

void QPdf::nextPage()
{
    if(!pdf || pageNum > pdf->numPages())
        return;
    pageNum++;
    ui->linePage->setText(QString().sprintf("%d", pageNum + 1));
    if(!changePage())
    {
        pageNum--;
        ui->linePage->setText(QString().sprintf("%d", pageNum + 1));
    }
}

void QPdf::prevPage()
{
    if(!pdf || pageNum == 0)
        return;
    pageNum--;
    ui->linePage->setText(QString().sprintf("%d", pageNum + 1));
    if(!changePage())
    {
        pageNum++;
        ui->linePage->setText(QString().sprintf("%d", pageNum + 1));
    }
}

void QPdf::setScale()
{
    bool chk;
    double tmp = ui->lineScale->text().toDouble(&chk);
    if(chk && tmp > 0)
    {
        ui->graphicsView->scale(1 / scale, 1 / scale);
        scale = tmp / 100;
        ui->graphicsView->scale(scale, scale);
    }
    else
        ui->lineScale->setText(QString().sprintf("%3.0f", scale * 100));
}
