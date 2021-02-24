// *****************************************************************************
//
//     Copyright (c) 2017, Imperx Inc., All rights reserved.
//
// *****************************************************************************
#include <QtWidgets/QApplication>
#include "QtCheetahPythonGainUI.h"

int main(int argc, char *argv[])
{	
	QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
	QApplication a(argc, argv);
    QCoreApplication::setOrganizationName( "Imperx Inc" );
    QCoreApplication::setOrganizationDomain( "imperx.com" );
    QCoreApplication::setApplicationName( "QtCheetahPythonGainUI" );

    QtCheetahPythonGainUI w;
    w.show();
    return a.exec();
}


