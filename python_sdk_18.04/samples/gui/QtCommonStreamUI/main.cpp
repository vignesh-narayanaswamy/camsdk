// *****************************************************************************
//
//     Copyright (c) 2017, Imperx Inc., All rights reserved.
//
// *****************************************************************************
#include <QApplication>
#include "QtCommonStreamUI.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    QCoreApplication::setOrganizationName( "Imperx Inc" );
    QCoreApplication::setOrganizationDomain( "imperx.com" );
    QCoreApplication::setApplicationName( "QtCommonStreamUI" );

    QtCommonStreamUI w;
    w.show();
    return a.exec();
}


