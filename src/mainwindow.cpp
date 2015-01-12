/***************************************************************************
 *   Copyright 2010 Pedro Guedes Alves <devel@pgalves.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

#include <QLabel>
#include <QScrollArea>
#include <QFileInfo>
#include <QImageReader>
#include <QTimer>
#include <QScrollBar>

#include <KMessageBox>
#include <KFileDialog>
#include <KUrl>
#include <KActionCollection>
#include <KStandardAction>
#include <KAboutApplicationDialog>
#include <KStatusBar>
#include <KApplication>
#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
//#include <KGlobal>

#include "mainwindow.h"
#include "kgpgme.h"
#include "configdialog.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      lastPath(QDir::homePath())
{
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setMinimumSize(400, 250);
    setCentralWidget(scrollArea);

    createActions();
    createStatusBar();

    setWindowTitle(i18n("Biombo"));
    resize(500, 350);

    config = KSharedConfig::openConfig("biomborc");

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutLock()));
}

void MainWindow::openFile()
{
    QString fileFormats = i18n("OpenPGP Encrypted Images (");
    // Get all inputformats available
    for (int i = 0; i < QImageReader::supportedImageFormats().count(); i++) {
            fileFormats += "*.";
            fileFormats += QString(QImageReader::supportedImageFormats().at(i));
            fileFormats += ".gpg ";
    }
    fileFormats += ")";

    KConfigGroup generalGroup(config, "General");

    lastPath = generalGroup.readPathEntry("lastDir", QDir::homePath());

    openFile(KFileDialog::getOpenFileName(lastPath, fileFormats, this, i18n("Open File")));
}

void MainWindow::openFile(const QString &inputFileName)
{
    m_gpg = new KGpgMe();
    bool useAgent = m_gpg->isGnuPGAgentAvailable();
    m_gpg->setUseGnuPGAgent(useAgent);

    secureArray = new QCA::SecureArray();
    tmpArray = new QByteArray();

    if (!inputFileName.isEmpty()) {

        QFile file(inputFileName);
        QFileInfo f(file);
        lastPath = f.absolutePath();

        // Update configuration file for last used path
        KConfigGroup generalGroup(config, "General");
        generalGroup.writePathEntry( "lastDir", lastPath);
        generalGroup.config()->sync();

        if (file.open(QIODevice::ReadOnly)) {
            *tmpArray = file.readAll();
            file.close();
            // decrypt data to secure array
            m_gpg->decrypt(*tmpArray, secureArray);
        }

        // Disable pixmap cache (don't need this but keep it anyway, pixmap is only auto
        // added to QPixmapCache when loading from file, where using loadFromData()).
        QPixmapCache::setCacheLimit(0);

        bool loadOk =  pixmap.loadFromData(*secureArray);
        if (pixmap.isNull() || !loadOk) {
            KMessageBox::information(this, i18n("Cannot load %1.").arg(inputFileName),
                                     i18n("Secure Image Viewer"));
            return;
        }

        imageLabel->setPixmap(pixmap);

        // Adjust imageLabel/pixmap to window size at loading
        normalSize();

        // Image loaded, cleanup, do we need this??
        m_gpg->clearCache();
        secureArray->clear();
        delete m_gpg;
        delete secureArray;
        delete tmpArray;

        quint64 timeOut = generalGroup.readEntry("timeOutPicture", 60);

        // Initialize timer to hide picture after delay
        timer->start(timeOut * 1000);
        timeout = false;

        // Update config TODO: only nedded because we dont have a configuration interface yet
        generalGroup.writeEntry( "timeOutPicture", timeOut);
        generalGroup.config()->sync();

        // Clear statusbar
        statusBar()->removeWidget(statusbarLabel);

        fitToWindowAct->setEnabled(true);
        updateActions();

        if (!fitToWindowAct->isChecked())
            imageLabel->adjustSize();
    }
}

void MainWindow::zoomIn()
{
    scaleImage(1.25);
}

void MainWindow::zoomOut()
{
    scaleImage(0.8);
}

void MainWindow::normalSize()
{
    updateImageLabel();
    imageLabel->adjustSize();
}

void MainWindow::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}

void MainWindow::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void MainWindow::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());

    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::timeoutLock()
{
    // Pixmap to load when timeout, if unable to find pixmap a null pixmap is created
    // TODO: find better way to define default lock icon
    QPixmap pixmapLock = QPixmap("/usr/share/icons/oxygen/128x128/status/image-missing.png");

    imageLabel->setPixmap(pixmapLock);
    imageLabel->adjustSize();

    statusbarLabel = new QLabel(i18n("Picture locked, open picture again."));

    if(!timeout) {
        statusBar()->addWidget(statusbarLabel);
    }

    // Don't rescale lock icon
    fitToWindowAct->setEnabled(false);

    timeout = true;
}

void MainWindow::resizeEvent(QResizeEvent * /* event */)
{
        updateImageLabel();
}

void MainWindow::updateImageLabel()
{
    // Only rescale pixmap on resize when displaying a pixmap and when that
    // pixmap is not the lock icon
    if (imageLabel->pixmap() != NULL && !timeout) {
        imageLabel->setPixmap(pixmap.scaled(scrollArea->size() * 0.98, Qt::KeepAspectRatio));
        imageLabel->adjustSize();
    }
}

// TODO: remove this function, only here for testing
void MainWindow::about()
{
    configdialog = new ConfigDialog();

    //KMessageBox::about(this,
      //      i18n("<p>The <b>Biombo</b> example shows how to combine QLabel "
       //        "and QScrollArea to display an image. QLabel is typically used "
        //       "for displaying a text, but it can also display an image. "
         //      "QScrollArea provides a scrolling view around another widget. </p>"
     //          ), i18n("About Biombo"));
    configdialog->show();
}

void MainWindow::createActions()
{
    openAct = KStandardAction::open(this, SLOT(openFile()), actionCollection());

    exitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    zoomInAct = KStandardAction::zoomIn(this, SLOT(zoomIn()), actionCollection());
    zoomInAct->setEnabled(false);

    zoomOutAct = KStandardAction::zoomOut(this, SLOT(zoomOut()), actionCollection());
    zoomOutAct->setEnabled(false);

    normalSizeAct = new KAction(this);
    normalSizeAct->setText(i18n("&Normal Size"));
    normalSizeAct->setIcon(KIcon("zoom-fit-best"));
    normalSizeAct->setShortcut(Qt::CTRL + Qt::Key_S);
    normalSizeAct->setEnabled(false);
    actionCollection()->addAction("normal-size", normalSizeAct);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new KAction(this);
    fitToWindowAct->setText(i18n("&Fit to Window"));
    fitToWindowAct->setIcon(KIcon("zoom-fit-best"));
    fitToWindowAct->setShortcut(Qt::CTRL + Qt::Key_F);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    actionCollection()->addAction("fit", fitToWindowAct);
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    // TODO:set correct slot for preferences when complete
    preferencesAct = KStandardAction::preferences(this, SLOT(about()/*setPreferences()*/),
                                                  actionCollection());



    aboutAct = new KAction(this);
    aboutAct->setText(i18n("&About"));
    aboutAct->setIcon(KIcon("help-about"));
    actionCollection()->addAction("help-about", aboutAct);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // TODO: Set path for gui configuration file as user application default folder,
    // should be $HOME/.kde4/share/apps/biombo/, use kde4-config or similar
    setupGUI(Default, QDir::currentPath() + "/biomboui.rc");
}

void MainWindow::createStatusBar()
{
    statusBar();
}

MainWindow::~MainWindow()
{
    //delete &tmp;
}

