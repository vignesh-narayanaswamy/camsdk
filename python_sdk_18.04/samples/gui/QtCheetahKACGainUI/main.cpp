// *****************************************************************************
//
//     Copyright (c) 2017, Imperx Inc., All rights reserved.
//
// *****************************************************************************
#include <QtWidgets/QApplication>
#include "QtCheetahKACGainUI.h"

int main(int argc, char *argv[])
{	
	// Bug 3013: now forcing locale to English, US for consistency
    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
	QApplication a(argc, argv);
    QCoreApplication::setOrganizationName( "Imperx Inc" );
    QCoreApplication::setOrganizationDomain( "imperx.com" );
    QCoreApplication::setApplicationName( "QtCheetahKACGainUI" );

    QtCheetahKACGainUI w;
    w.show();
    return a.exec();
}

