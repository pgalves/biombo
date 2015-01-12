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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCrypto>
#include <QDebug>
#include <QPixmapCache>
#include <QPixmap>

#include <KXmlGuiWindow>
#include <KAction>
#include <KMenu>
#include <KMenuBar>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KHelpMenu>

#include "configdialog.h"

using namespace std;

QT_BEGIN_NAMESPACE
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE
class KGpgMe;

// Class allows loading pixmap from secureArray
class QPixmapSec: public QPixmap
{
public:
    // load QPixmap from SecureArray
    bool loadFromData(const QCA::SecureArray &data, const char *format=0,
                      Qt::ImageConversionFlags flags=Qt::AutoColor)
    {
        return QPixmap::loadFromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format, flags);
    }

};


class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();
    void openFile(const QString &inputFileName);
    QString lastPath;
    qint32 timeoutSeconds;
    ConfigDialog *configdialog;

private slots:
    void openFile();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();
    void timeoutLock();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void resizeEvent(QResizeEvent * /* event */);
    void updateImageLabel();

    bool timeout;
    double scaleFactor;
    QLabel *imageLabel;
    QLabel *statusbarLabel;
    QScrollArea *scrollArea;
    QPixmapSec pixmap;
    QByteArray *tmpArray;

    QCA::SecureArray *secureArray;
    QTimer *timer;
    KGpgMe* m_gpg;

    KAction *openAct;
    KAction *exitAct;
    KAction *zoomInAct;
    KAction *zoomOutAct;
    KAction *normalSizeAct;
    KAction *fitToWindowAct;
    KAction *aboutAct;
    KAction *aboutAppAct;
    KAction *aboutKdeAct;
    KAction *preferencesAct;

    KSharedConfigPtr config;
};

#endif // MAINWINDOW_H
