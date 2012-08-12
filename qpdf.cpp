#include "qpdf.h"
#include "ui_qpdf.h"
#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>
#include <QWheelEvent>

#include "pdfwidget.h"

const QColor textWhiteColor(Qt::white);
const QColor textErrorColor(240, 180, 170);

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
    connect(ui->pdfView, SIGNAL(readyGL()), this, SLOT(initialized()));
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

void QPdf::wheelEvent(QWheelEvent *e)
{
    if(e->delta() < 0)
        nextPage();
    else
        prevPage();
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
    double scale = ui->lineScale->text().toDouble(&chk);
    QPalette clear = ui->lineScale->palette();
    clear.setColor(QPalette::Base, textWhiteColor);
    ui->lineScale->setPalette(clear);
    if(chk && scale > 0)
    {
        ui->pdfView->scalePDF(scale);
    }
    else
    {
        if(ui->lineScale->text().toLower() == "fit" ||
                ui->lineScale->text().isEmpty())
            ui->pdfView->scalePDF(0.0);
        else
        {
            QPalette err = ui->lineScale->palette();
            err.setColor(QPalette::Base, textErrorColor);
            ui->lineScale->setPalette(err);
        }
    }
}

void QPdf::setRotation()
{
    bool chk;
    double angle = ui->lineRotate->text().toDouble(&chk);
    QPalette clear = ui->lineRotate->palette();
    clear.setColor(QPalette::Base, textWhiteColor);
    ui->lineRotate->setPalette(clear);
    if(chk)
    {
        ui->pdfView->rotatePDF(angle);
    }
    else
    {
        if(ui->lineRotate->text().isEmpty())
            ui->pdfView->rotatePDF(0);
        else
        {
            QPalette err = ui->lineRotate->palette();
            err.setColor(QPalette::Base, textErrorColor);
            ui->lineRotate->setPalette(err);
        }
    }
}

void QPdf::pdfLoaded(QString pdfName)
{
    ui->labelPages->setText(QString().sprintf("/ %d", ui->pdfView->pageCount()));
    setWindowTitle(QString("QPdf: ") + pdfName);
}

void QPdf::showFiles()
{
    fd->show();
}

void QPdf::initialized()
{
    if(qApp->arguments().size() > 1)
        ui->pdfView->loadPDF(qApp->arguments()[1]);
}
