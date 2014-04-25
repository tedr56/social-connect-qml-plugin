/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include <QtDeclarative/QtDeclarative>
#include <QtCore/QtPlugin>

#include "socialconnectplugin.h"
#include "webinterface.h"

#ifdef ENABLE_SMOKE_CONNECTION
#include "smoke/smokeconnection.h"
#endif

#include "facebook/facebookconnection.h"
#include "twitter/twitterconnection.h"

void SocialConnectPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<WebInterface>(uri, 1, 0, "WebInterface");
    qmlRegisterType<TwitterConnection>(uri, 1, 0, "TwitterConnection");
    qmlRegisterType<FacebookConnection>(uri, 1, 0, "FacebookConnection");

#ifdef ENABLE_SMOKE_CONNECTION
    qmlRegisterType<SmokeConnection>(uri, 1, 0, "SmokeConnection");
#endif
}

Q_EXPORT_PLUGIN2(socialconnect, SocialConnectPlugin)
