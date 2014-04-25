/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include <QtCore/QUrl>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QtGui/QApplication>
//#include <QtNetwork/QNetworkProxyFactory>

#include "socialconnectplugin.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(new QApplication(argc, argv));

    //QNetworkProxyFactory::setUseSystemConfiguration(true);

    // Construct and register the Social Connect plug-in
    SocialConnectPlugin socialConnectPlugin;
    socialConnectPlugin.registerTypes("SocialConnect");

    QDeclarativeView view;
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    QObject::connect(view.engine(), SIGNAL(quit()), app.data(), SLOT(quit()));
    view.setSource(QUrl("qrc:/main.qml"));
    view.showFullScreen();

    return app->exec();
}
