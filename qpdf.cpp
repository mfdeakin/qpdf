#include "qpdf.h"
#include "ui_qpdf.h"
#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>

#include "pdfwidget.h"

QPdf::QPdf(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QPdf),
    fd(new QFileDialog(this))
{
    ui->setupUi(this);
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->setFilter("*.pdf");
    fd->setFileMode(QFileDialog::ExistingFile);
    connect(fd, SIGNAL(fileSelected(QString)), ui->pdfView, SLOT(loadPDF(QString)));
    ui->pdfView->setClearColor(palette().color(QPalette::Window));
}

QPdf::~QPdf()
{
    delete ui;
}

void QPdf::keyPressEvent(QKeyEvent *e)
{
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
    int page = ui->linePage->text().toInt(&chk) - 1;
    if(!chk || page >= ui->pdfView->pageCount() || page < 0)
        return false;
    ui->pdfView->changePage(page);
    return true;
}

void QPdf::nextPage()
{
    int pageNum = ui->pdfView->pageNumber();
    if(pageNum >= ui->pdfView->pageCount())
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
    int pageNum = ui->pdfView->pageNumber() + 1;
    if(pageNum == 0)
        return;
    pageNum--;
    ui->linePage->setText(QString().sprintf("%d", pageNum));
    if(!changePage())
    {
        pageNum++;
        ui->linePage->setText(QString().sprintf("%d", pageNum));
    }
}

void QPdf::setScale()
{
    bool chk;
    double tmp = ui->lineScale->text().toDouble(&chk);
    if(chk && tmp > 0)
    {
        ui->pdfView->scalePDF(tmp);
    }
    else
        ui->lineScale->setText(QString().sprintf("%3.0f", scale * 100));
}

void QPdf::pdfLoaded()
{
    ui->labelPages->setText(QString().sprintf("/ %d", ui->pdfView->pageCount()));
}

void QPdf::showFiles()
{
    fd->show();
}
