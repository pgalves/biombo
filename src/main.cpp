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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KUrl>
#include <KLocale>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData( "biombo", NULL,
                          ki18n("Biombo"), "0.2",
                          ki18n("View OpenPGP encrypted images securely."),
                          KAboutData::License_GPL,
                          ki18n("Copyright (c) 2010 Pedro Alves") );

    aboutData.setProgramIconName("kgpg");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[File]", ki18n("OpenPGP encrypted picture to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    // Initialize QCA: the Initializer object sets things up, and
    // also does cleanup when it goes out of scope
    QCA::Initializer init;

    MainWindow* window = new MainWindow();
    window->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if(args->count())
    {
         window->openFile(args->arg(0));
    }

    return app.exec();
}
