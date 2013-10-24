#include "instagramconnection.h"

#include <QDebug>
#include <QMap>
#include <QNetworkReply>
#include <QSettings>
#include <QStringList>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QUrl>

#include "instagramconstants.h"
#include "instagramrequest.h"
#include "webinterface.h"
#include "socialconnectionerror.h"

/*!
    \class InstagramConnection

    TwitterConnection is an interface to Twitter.
 */

/*!
    \property InstagramConnection::consumerKey

    This \c {mandatory} property specifies Twitter application id that is used
    with the Twitter connection.

    See \l {https://dev.twitter.com/docs/}
    {Twitter documentation} for futher details.
 */

/*!
    \property InstagramConnection::consumerSecret

    This \c {mandatory} property specifies Twitter application secret that is
    used with the Twitter connection.

    See \l {https://dev.twitter.com/docs/}
    {Twitter documentation} for futher details.
 */

/*!
    \property InstagramConnection::callbackUrl

    This \c {mandatory} property specifies Twitter callback URL that is being
    used to redirect the client after successful authentication.

    See \l {https://dev.twitter.com/docs/auth/implementing-sign-twitter/}
    {Twitter authentication documentation} for further details.
 */

/*!
    \property InstagramConnection::accessToken

    This property holds an access token for creating Twitter requests.
    With access token user is granted to create requests to Twitter data
    that is not publically available.

    See \l {https://dev.twitter.com/docs/auth/implementing-sign-twitter}
    {Twitter authentication documentation} for further details.
 */

/*!
    \property InstagramConnection::accessTokenSecret

    This property holds an access token secret for creating Twitter requests.
    With access token secret user can sign the requests (in conjunction with
    the consumer secret) that use the accessToken.

    See \l {https://dev.twitter.com/docs/auth/implementing-sign-twitter}
    {Twitter authentication documentation} for further details.
 */

InstagramConnection::InstagramConnection(QObject *parent) :
    SocialConnection(parent),
    m_instagramRequest(new InstagramRequest(this)),
    m_ongoingRequest(0),
    m_state(NotLogged)
{
}

InstagramConnection::~InstagramConnection()
{
    delete m_instagramRequest;
    delete m_ongoingRequest;
}

QString InstagramConnection::consumerKey() const
{
    return m_consumerKey;
}

void InstagramConnection::setConsumerKey(const QString &consumerKey)
{
    if (m_consumerKey != consumerKey) {
        m_consumerKey = consumerKey;
        m_instagramRequest->setConsumerKey(consumerKey);
        emit consumerKeyChanged(m_consumerKey);
    }
}

QString InstagramConnection::consumerSecret() const
{
    return m_consumerSecret;
}

void InstagramConnection::setConsumerSecret(const QString &consumerSecret)
{
    if (m_consumerSecret != consumerSecret) {
        m_consumerSecret = consumerSecret;
        m_instagramRequest->setConsumerSecret(consumerSecret);
        emit consumerSecretChanged(m_consumerSecret);
    }
}

QString InstagramConnection::accessToken() const
{
    return m_accessToken;
}

void InstagramConnection::setAccessToken (const QString &accessToken)
{
    if (m_accessToken != accessToken) {
        m_accessToken = accessToken;
        m_instagramRequest->setAccessToken(accessToken);
        emit accessTokenChanged(m_accessToken);
    }
}

QString InstagramConnection::callbackUrl() const
{
    return m_callbackUrl;
}

void InstagramConnection::setCallbackUrl(const QString &callbackUrl)
{
    if (m_callbackUrl != callbackUrl) {
        m_callbackUrl = callbackUrl;
        emit callbackUrlChanged(m_callbackUrl);
    }
}

QString InstagramConnection::requestToken() const
{
    return m_requestToken;
}

void InstagramConnection::setRequestToken(const QString &token)
{
    if (m_requestToken != token) {
        m_requestToken = token;
        m_instagramRequest->setRequestToken(token);
        emit requestTokenChanged(m_requestToken);
    }
}

InstagramConnection::State InstagramConnection::state() const
{
    return m_state;
}

void InstagramConnection::setState(State state)
{
    if (m_state != state) {
        m_state = state;
    }
}

bool InstagramConnection::authenticate()
{
    bool ret = false;

    if (authenticated() || state() != NotLogged || busy()) {
        ret = authenticated();
        qWarning() << "State already logged / busy. Skipping. Authenticated:" << ret;
    }
    else  {
        setState(AcquiringRequestToken);
        setBusy(true);
        setTransmitting(true);

        QUrl url(OAUTH_AUTH_URL);
        url.addEncodedQueryItem(OAUTH_CLIENT_ID, consumerKey().toUtf8());
        url.addEncodedQueryItem(OAUTH_REDIRECT_URI, callbackUrl().toUtf8());

        if(m_authScope.isEmpty())
            url.addEncodedQueryItem("scope", QString("basic").toUtf8());
        else
            url.addEncodedQueryItem("scope", m_authScope.toUtf8());

        if(m_authMetod == Token)
            url.addEncodedQueryItem(OAUTH_RESPONSE_TYPE, OAUTH_RESPONSE_TYPE_TOKEN);
        else
            url.addEncodedQueryItem(OAUTH_RESPONSE_TYPE, OAUTH_RESPONSE_TYPE_CODE);

        qDebug() << "URL: " + url.toString();

        setState(Authorizing);
        setTransmitting(false);     // Not anymore transmitting, but still "Busy"!
        setAuthenticated(false);    // TODO: Is this in the right place??
        WebInterface *webInterface = qobject_cast<WebInterface*>(SocialConnection::webInterface());
        webInterface->setActive(true);
        webInterface->setUrl(url);

        ret = true;
    }

    return ret;
}

bool InstagramConnection::deauthenticate()
{
    clearAllMembers();
    setState(NotLogged);
    setName("");
    setAuthenticated(false);

    QMetaObject::invokeMethod(this, "deauthenticateCompleted",
                              Qt::QueuedConnection, Q_ARG(bool, true));

    return true;
}

bool InstagramConnection::postMessage(const QVariantMap &message)
{
    return true;
}

bool InstagramConnection::retrieveMessageCount()
{
    return true;
}

bool InstagramConnection::retrieveMessages(const QString &from, const QString &to, int max)
{
    return true;
}

void InstagramConnection::cancel()
{
    if (m_ongoingRequest != 0 && m_ongoingRequest->isRunning()) {
        qWarning() << "Request ongoing, aborting!";
        m_ongoingRequest->abort();
    }
    // Not busy or transmitting anymore
    setTransmitting(false);
    setBusy(false);

    // If we were in the middle of authentication process, reset the state
    if (state() != Logged && state() != NotLogged) {
        clearAllMembers();
        setState(NotLogged);
        WebInterface *webInterface = qobject_cast<WebInterface*>(SocialConnection::webInterface());
        webInterface->setActive(false);
        emit authenticateCompleted(false);
    }
}

bool InstagramConnection::removeCredentials()
{
    QSettings settings("Nokia", consumerKey());
    settings.remove(OAUTH_ACCESS_TOKEN);

    return settings.status() == QSettings::NoError;
}

void InstagramConnection::onUrlChanged(const QUrl &url)
{
    SocialConnection::onUrlChanged(url);

    if (busy() && !url.isEmpty() && url.toString().startsWith(m_callbackUrl)) {
        WebInterface *webInterface = qobject_cast<WebInterface*>(SocialConnection::webInterface());
        webInterface->setActive(false);

        if(m_authMetod == Token)
        {
            m_accessToken = url.toString().split("=").at(1);

            if (!m_accessToken.isEmpty())
                authenticationSucceeded();
        }
        else
        {
            setRequestToken(url.queryItemValue(OAUTH_CODE));
            qDebug() << m_requestToken;
            acquireAccessToken();
        }
    }
}

void InstagramConnection::clearAllMembers()
{
    m_accessToken.clear();
    m_requestToken.clear();
    m_instagramRequest->setAccessToken("");
}

void InstagramConnection::authenticationFailed(const QString &errorMsg, const int errorCode)
{
    qWarning() << errorMsg << (errorCode ? QString::number(errorCode) : QString(""));

    clearAllMembers();
    setTransmitting(false);
    setBusy(false);
    setState(NotLogged);
    emit authenticateCompleted(false);
}

void InstagramConnection::setAuthorizationMethod(AuthorizationMethod method)
{
    m_authMetod = method;
}

void InstagramConnection::setAuthorizationScope(const QString &scope)
{
    m_authScope = scope;
}

bool InstagramConnection::getUser(QString userId)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + userId);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUser(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedUser(const QByteArray &result)
{
    QVariantList list;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");
    QScriptValueIterator it(scValue);

    while (it.hasNext()) {
        it.next();

        QVariantMap message;

        qDebug() << it.value().property("data").property("username").toString();

        message.insert("user_username", it.value().property("data").property("username").toString());
        message.insert("user_id", it.value().property("data").property("id").toString());
        message.insert("user_full_name", it.value().property("data").property("full_name").toString());
        message.insert("user_profile_picture", it.value().property("data").property("profile_picture").toString());
        message.insert("user_bio", it.value().property("data").property("bio").toString());
        message.insert("user_website", it.value().property("data").property("website").toString());
        message.insert("user_counts_media", it.value().property("data").property("counts").property("media").toString());
        message.insert("user_counts_follows", it.value().property("data").property("counts").property("follows").toString());
        message.insert("user_counts_followed_by", it.value().property("data").property("counts").property("followed_by").toString());

        list.append(message);
    }

    return list;
}

bool InstagramConnection::getUserSelfFeed(const QString &count, const QString &min_id, const QString &max_id)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_SELF_FEED_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!count.isEmpty())
            url.addQueryItem("count", count);
        if(!min_id.isEmpty())
            url.addQueryItem("min_id", min_id);
        if(!max_id.isEmpty())
            url.addQueryItem("max_id", max_id);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserSelfFeedReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserSelfFeedReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserSelfFeedCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserMediaRecent(const QString &user, const QString &count,
                                             const QString &min_id, const QString &max_id,
                                             const QString &min_timestamp, const QString &max_timestamp)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + user + "/media/recent/");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!count.isEmpty())
            url.addQueryItem("count", count);
        if(!min_id.isEmpty())
            url.addQueryItem("min_id", min_id);
        if(!max_id.isEmpty())
            url.addQueryItem("max_id", max_id);
        if(!min_timestamp.isEmpty())
            url.addQueryItem("min_timestamp", min_timestamp);
        if(!max_timestamp.isEmpty())
            url.addQueryItem("max_timestamp", max_timestamp);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserMediaRecentReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserMediaRecentReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserMediaRecentCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserSelfMediaLiked(const QString &count, const QString &max_like_id)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_SELF_MEDIA_LIKED_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!count.isEmpty())
            url.addQueryItem("count", count);
        if(!max_like_id.isEmpty())
            url.addQueryItem("max_like_id", max_like_id);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserSelfMediaLikedReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserSelfMediaLikedReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserSelfMediaLikedCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserSearch(const QString &key, const QString &count)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_SEARCH_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!key.isEmpty())
            url.addQueryItem("q", key);
        if(!count.isEmpty())
            url.addQueryItem("count", count);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserSearchReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserSearchReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserSearchCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUsers(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserFollows(const QString &user)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + user + "/follows");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserFollowsReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserFollowsReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserFollowsCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUsers(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedUsers(const QByteArray &result)
{
    QVariantList list;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList datalist = it.value().property("data").toVariant().toList();
    int lenght = datalist.length();

    for(int index = 0; index < lenght; index++) {
        QVariantMap message;

        message.insert("user_username", it.value().property("data").property(index).property("username").toString());
        message.insert("user_full_name", it.value().property("data").property(index).property("full_name").toString());
        message.insert("user_profile_picture", it.value().property("data").property(index).property("profile_picture").toString());
        message.insert("user_id", it.value().property("data").property(index).property("id").toString());
        message.insert("user_bio", it.value().property("data").property(index).property("bio").toString());
        message.insert("user_website", it.value().property("data").property(index).property("website").toString());

        list.append(message);
    }

    return list;
}

bool InstagramConnection::getUserFollowedBy(const QString &user)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + user + "/followed-by");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserFollowedByReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserFollowedByReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserFollowedByCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUsers(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserSelfRequestedBy()
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_REQUESTEDBY_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserSelfRequestedByReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserSelfRequestedByReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserSelfRequestedByCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUsers(data));
    }
    else
        emitError();
}

bool InstagramConnection::getUserRelationship(const QString &user)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + user + "/relationship");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetUserRelationshipReply()));
    }

    return ret;
}

void InstagramConnection::onGetUserRelationshipReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveUserRelationshipCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedUserRelationship(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedUserRelationship(const QByteArray &result)
{
    QVariantList list;

    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap message;

    message.insert("outgoing_status", it.value().property("data").property("outgoing_status").toString());
    message.insert("target_user_is_private", it.value().property("data").property("target_user_is_private").toString());
    message.insert("incoming_status", it.value().property("data").property("incoming_status").toString());

    list.append(message);

    return list;
}

bool InstagramConnection::setUserRelationship(const QString &user, const QString &action)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_USERS_URL + user + "/relationship");

        qDebug() << url.toString();

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        QUrl params;
        params.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);
        params.addQueryItem("action", action);
        qDebug() << "ACTION: " + action;

        QByteArray data;
        data = params.encodedQuery();

        m_ongoingRequest = m_networkManager.post(req, data);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onSetUserRelationshipReply()));
    }

    return ret;
}

void InstagramConnection::onSetUserRelationshipReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit setUserRelationshipCompleted(requestError == QNetworkReply::NoError,
                                       parseSetUserRelationship(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseSetUserRelationship(const QByteArray &result)
{
    QVariantList list;

    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap message;

    message.insert("outgoing_status", it.value().property("data").property("outgoing_status").toString());

    list.append(message);

    return list;
}

bool InstagramConnection::getMedia(const QString &media)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetMediaReply()));
    }

    return ret;
}

void InstagramConnection::onGetMediaReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveMediaCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedia(data));
    }
    else
        emitError();
}

bool InstagramConnection::getMediaSearch(const QString &lat, const QString &lng,
                                         const QString &min_timestamp, const QString &max_timestamp, const QString &distance)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_SEARCH_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!lat.isEmpty())
            url.addQueryItem("lat", lat);
        if(!lng.isEmpty())
            url.addQueryItem("lng", lng);
        if(!min_timestamp.isEmpty())
            url.addQueryItem("min_timestamp", min_timestamp);
        if(!max_timestamp.isEmpty())
            url.addQueryItem("max_timestamp", max_timestamp);
        if(!distance.isEmpty())
            url.addQueryItem("distance", distance);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetMediaSearchReply()));
    }

    return ret;
}

void InstagramConnection::onGetMediaSearchReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveMediaSearchCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::getMediaPopular()
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_POPULAR_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetMediaPopularReply()));
    }

    return ret;
}

void InstagramConnection::onGetMediaPopularReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveMediaPopularCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::postComment(const QString &media, const QString &text)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/comments");

        qDebug() << url.toString();

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        QUrl params;
        params.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);
        params.addQueryItem("text", text);

        QByteArray data;
        data = params.encodedQuery();

        m_ongoingRequest = m_networkManager.post(req, data);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onPostCommentReply()));
    }

    return ret;
}

void InstagramConnection::onPostCommentReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit postCommentCompleted(requestError == QNetworkReply::NoError,
                                       parsePostedComment(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parsePostedComment(const QByteArray &result)
{
    QVariantList comments;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap comment;
    comment.insert("comment_created_time", it.value().property("data").property("created_time").toString());
    comment.insert("comment_text", it.value().property("data").property("text").toString());
    comment.insert("comment_id", it.value().property("data").property("id").toString());
    comment.insert("comment_from_username", it.value().property("data").property("from").property("username").toString());
    comment.insert("comment_from_profile_picture", it.value().property("data").property("from").property("profile_picture").toString());
    comment.insert("comment_from_id", it.value().property("data").property("from").property("id").toString());
    comment.insert("comment_from_full_name", it.value().property("data").property("from").property("full_name").toString());

    comments.append(comment);
    return comments;
}

bool InstagramConnection::deleteComment(const QString &media, const QString &comment)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/comments/" + comment);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.deleteResource(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onDeleteCommentReply()));
    }

    return ret;
}

void InstagramConnection::onDeleteCommentReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit deleteCommentCompleted(requestError == QNetworkReply::NoError,
                                       parseDeletedComment(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseDeletedComment(const QByteArray &result)
{
    QVariantList comments;
    return comments;
}

bool InstagramConnection::getLikes(const QString &media)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/likes");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetLikesReply()));
    }

    return ret;
}

void InstagramConnection::onGetLikesReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveLikesCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedLikes(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedLikes(const QByteArray &result)
{
    QVariantList likes;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList likeslist = it.value().property("data").toVariant().toList();
    int likeslistLenght = likeslist.length();

    if(likeslistLenght > 0) {
        for(int like_index = 0; like_index < likeslistLenght; like_index++) {
            QVariantMap like;
            like.insert("like_username", it.value().property("data").property(like_index)
                           .property("username").toString());
            like.insert("like_bio", it.value().property("data").property(like_index)
                           .property("bio").toString());
            like.insert("like_website", it.value().property("data").property(like_index)
                           .property("website").toString());
            like.insert("like_profile_picture", it.value().property("data").property(like_index)
                           .property("profile_picture").toString());
            like.insert("like_full_name", it.value().property("data").property(like_index)
                           .property("full_name").toString());
            like.insert("like_id", it.value().property("data").property(like_index)
                           .property("id").toString());

            likes.append(like);
        }
    }

    return likes;
}

bool InstagramConnection::postLike(const QString &media)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/likes");

        qDebug() << url.toString();

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        QUrl params;
        params.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        QByteArray data;
        data = params.encodedQuery();

        m_ongoingRequest = m_networkManager.post(req, data);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onPostLikeReply()));
    }

    return ret;
}

void InstagramConnection::onPostLikeReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit postLikeCompleted(requestError == QNetworkReply::NoError,
                                       parsePostedLike(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parsePostedLike(const QByteArray &result)
{
    QVariantList like;
    return like;
}

bool InstagramConnection::deleteLike(const QString &media)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/likes");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.deleteResource(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onDeleteLikeReply()));
    }

    return ret;
}

void InstagramConnection::onDeleteLikeReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit deleteLikeCompleted(requestError == QNetworkReply::NoError,
                                       parseDeletedLike(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseDeletedLike(const QByteArray &result)
{
    QVariantList like;
    return like;
}

bool InstagramConnection::getComments(const QString &media)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_MEDIA_URL + media + "/comments");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()),
                this, SLOT(onGetCommentsReply()));
    }

    return ret;
}

void InstagramConnection::onGetCommentsReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveCommentsCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedComments(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedComments(const QByteArray &result)
{
    QVariantList comments;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList commentslist = it.value().property("data").toVariant().toList();
    int commentslistLenght = commentslist.length();

    if(commentslistLenght > 0) {
        for(int comment_index = 0; comment_index < commentslistLenght; comment_index++) {
            QVariantMap comment;
            comment.insert("comment_created_time", it.value().property("data").property(comment_index)
                           .property("created_time").toString());
            comment.insert("comment_text", it.value().property("data").property(comment_index)
                           .property("text").toString());
            comment.insert("comment_id", it.value().property("data").property(comment_index)
                           .property("id").toString());
            comment.insert("comment_from_username", it.value().property("data").property(comment_index)
                           .property("from").property("username").toString());
            comment.insert("comment_from_profile_picture", it.value().property("data").property(comment_index)
                           .property("from").property("profile_picture").toString());
            comment.insert("comment_from_id", it.value().property("data").property(comment_index)
                           .property("from").property("id").toString());
            comment.insert("comment_from_full_name", it.value().property("data").property(comment_index)
                           .property("from").property("full_name").toString());

            comments.append(comment);
        }
    }

    return comments;
}

bool InstagramConnection::getTag(const QString &tag)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_TAGS_URL + tag);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetTagReply()));
    }

    return ret;
}

void InstagramConnection::onGetTagReply()
{
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getTagCompleted(requestError == QNetworkReply::NoError,
                                       parseTag(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseTag(const QByteArray &result)
{
    QVariantList tags;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap tag;
    tag.insert("tag_media_count", it.value().property("data").property("media_count").toString());
    tag.insert("tag_name", it.value().property("data").property("name").toString());

    tags.append(tag);
    return tags;
}

bool InstagramConnection::getTagMediaRecent(const QString &tag, const QString &min_id, const QString &max_id)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_TAGS_URL + tag + "/media/recent");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!min_id.isEmpty())
            url.addQueryItem("min_id", min_id);

        if(!max_id.isEmpty())
            url.addQueryItem("max_id", max_id);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetTagMediaRecentReply()));
    }

    return ret;
}

void InstagramConnection::onGetTagMediaRecentReply()
{
    const int requestError = m_ongoingRequest->error();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getTagMediaRecentCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

bool InstagramConnection::getTagsSearch(const QString &tag)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_TAGS_SEARCH_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);
        url.addQueryItem("q", tag);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetTagsSearchReply()));
    }

    return ret;
}

void InstagramConnection::onGetTagsSearchReply()
{
    const int requestError = m_ongoingRequest->error();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getTagsSearchCompleted(requestError == QNetworkReply::NoError,
                                       parseTags(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseTags(const QByteArray &result)
{
    QVariantList tags;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList tagslist = it.value().property("data").toVariant().toList();
    int tagslistLenght = tagslist.length();

    if(tagslistLenght > 0) {
        for(int tag_index = 0; tag_index < tagslistLenght; tag_index++) {
            QVariantMap tag;
            tag.insert("tag_media_count", it.value().property("data").property(tag_index)
                           .property("media_count").toString());
            tag.insert("tag_name", it.value().property("data").property(tag_index)
                           .property("name").toString());
            tags.append(tag);
        }
    }

    return tags;
}

bool InstagramConnection::getLocation(const QString &location)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_LOCATIONS_URL + location);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetLocationReply()));
    }

    return ret;
}

void InstagramConnection::onGetLocationReply()
{
    const int requestError = m_ongoingRequest->error();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getLocationCompleted(requestError == QNetworkReply::NoError,
                                       parseLocation(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseLocation(const QByteArray &result)
{
    QVariantList locations;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap location;
    location.insert("location_id", it.value().property("data").property("id").toString());
    location.insert("location_name", it.value().property("data").property("name").toString());
    location.insert("location_latitude", it.value().property("data").property("latitude").toString());
    location.insert("location_longitude", it.value().property("data").property("longitude").toString());

    locations.append(location);
    return locations;
}

bool InstagramConnection::getLocationSearch(const QString &lat, const QString &lng, const QString &foursquare_v2_id, const QString &distance)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_LOCATIONS_SEARCH_URL);
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        if(!lat.isEmpty())
            url.addQueryItem("lat", lat);
        if(!lng.isEmpty())
            url.addQueryItem("lng", lng);
        if(!foursquare_v2_id.isEmpty())
            url.addQueryItem("foursquare_v2_id", foursquare_v2_id);
        if(!distance.isEmpty())
            url.addQueryItem("distance", distance);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetLocationSearchReply()));
    }

    return ret;
}

void InstagramConnection::onGetLocationSearchReply()
{
    const int requestError = m_ongoingRequest->error();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getLocationCompleted(requestError == QNetworkReply::NoError,
                                       parseLocations(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseLocations(const QByteArray &result)
{
    QVariantList locations;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList locationsList = it.value().property("data").toVariant().toList();
    int locationsListLenght = locationsList.length();

    if(locationsListLenght > 0) {
        for(int location_index = 0; location_index < locationsListLenght; location_index++) {
            QVariantMap location;

            location.insert("location_id", it.value().property("data").property(location_index).property("id").toString());
            location.insert("location_name", it.value().property("data").property(location_index).property("name").toString());
            location.insert("location_latitude", it.value().property("data").property(location_index).property("latitude").toString());
            location.insert("location_longitude", it.value().property("data").property(location_index).property("longitude").toString());

            locations.append(location);
        }
    }

    return locations;
}

bool InstagramConnection::getLocationMediaRecent(const QString &location)
{
    bool ret = false;

    if (!authenticated() || state() != Logged || busy() ) {
        qWarning() << "Cannot get User while not authenticated/logged";
    }
    else {
        ret = true;
        setBusy(true);
        setTransmitting(true);

        QUrl url(INSTAGRAM_LOCATIONS_URL + location + "/media/recent");
        url.addQueryItem(OAUTH_ACCESS_TOKEN, m_accessToken);

        qDebug() << url.toString();

        QNetworkRequest req(url);

        m_ongoingRequest = m_networkManager.get(req);
        connect(m_ongoingRequest, SIGNAL(finished()), this, SLOT(onGetLocationMediaRecentReply()));
    }

    return ret;
}

void InstagramConnection::onGetLocationMediaRecentReply()
{
    const int requestError = m_ongoingRequest->error();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit getLocationMediaRecentCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedMedias(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedMedia(const QByteArray &result)
{
    QVariantList list;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantMap message;

    message.insert("image_low_resolution", it.value().property("data")
                   .property("images").property("low_resolution")
                   .property("url").toString());
    message.insert("image_low_resolution_width", it.value().property("data")
                   .property("images").property("low_resolution")
                   .property("width").toString());
    message.insert("image_low_resolution_height", it.value().property("data")
                   .property("images").property("low_resolution")
                   .property("height").toString());

    message.insert("image_standard_resolution", it.value().property("data")
                   .property("images").property("standard_resolution")
                   .property("url").toString());
    message.insert("image_standard_resolution_width", it.value().property("data")
                   .property("images").property("standard_resolution")
                   .property("width").toString());
    message.insert("image_standard_resolution_height", it.value().property("data")
                   .property("images").property("standard_resolution")
                   .property("height").toString());

    message.insert("image_thumbnail", it.value().property("data")
                   .property("images").property("thumbnail")
                   .property("url").toString());
    message.insert("image_thumbnail_width", it.value().property("data")
                   .property("images").property("thumbnail")
                   .property("width").toString());
    message.insert("image_thumbnail_height", it.value().property("data")
                   .property("images").property("thumbnail")
                   .property("height").toString());

    message.insert("location_id", it.value().property("data")
                   .property("location").property("id").toString());
    message.insert("location_latitude", it.value().property("data")
                   .property("location").property("latitude").toString());
    message.insert("location_longitude", it.value().property("data")
                   .property("location").property("longitude").toString());
    message.insert("location_name", it.value().property("data")
                   .property("location").property("name").toString());

    message.insert("user_username", it.value().property("data")
                   .property("user").property("username").toString());
    message.insert("user_full_name", it.value().property("data")
                   .property("user").property("full_name").toString());
    message.insert("user_profile_picture", it.value().property("data")
                   .property("user").property("profile_picture").toString());
    message.insert("user_id", it.value().property("data")
                   .property("user").property("id").toString());

    message.insert("image_id", it.value().property("data").property("id").toString());
    message.insert("image_type", it.value().property("data").property("type").toString());
    message.insert("image_filter", it.value().property("data").property("filter").toString());
    message.insert("image_link", it.value().property("data").property("link").toString());
    message.insert("created_time", it.value().property("data").property("created_time").toString());

    message.insert("caption_created_time", it.value().property("data")
                   .property("caption").property("created_time").toString());
    message.insert("caption_text", it.value().property("data")
                   .property("caption").property("text").toString());
    message.insert("caption_id", it.value().property("data")
                   .property("caption").property("id").toString());

    message.insert("caption_from_username", it.value().property("data")
                   .property("caption").property("from").property("username").toString());
    message.insert("caption_from_full_name", it.value().property("data")
                   .property("caption").property("from").property("full_name").toString());
    message.insert("caption_from_type", it.value().property("data")
                   .property("caption").property("from").property("type").toString());
    message.insert("caption_from_id", it.value().property("data")
                   .property("caption").property("from").property("id").toString());

    message.insert("comments_count", it.value().property("data").property("comments").property("count").toString());    
    message.insert("comments", parseComments(it));

    message.insert("likes_count", it.value().property("data")
                   .property("likes").property("count").toString());
    message.insert("likes", parseLikes(it));

    list.append(message);
    return list;
}

QVariantList InstagramConnection::parseRetrievedMedias(const QByteArray &result)
{
    QVariantList list;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    QVariantList datalist = it.value().property("data").toVariant().toList();
    int lenght = datalist.length();

    for(int index = 0; index < lenght; index++) {
        QVariantMap message;

        message.insert("image_low_resolution", it.value().property("data")
                       .property(index).property("images").property("low_resolution")
                       .property("url").toString());
        message.insert("image_low_resolution_width", it.value().property("data")
                       .property(index).property("images").property("low_resolution")
                       .property("width").toString());
        message.insert("image_low_resolution_height", it.value().property("data")
                       .property(index).property("images").property("low_resolution")
                       .property("height").toString());

        message.insert("image_standard_resolution", it.value().property("data")
                       .property(index).property("images").property("standard_resolution")
                       .property("url").toString());
        message.insert("image_standard_resolution_width", it.value().property("data")
                       .property(index).property("images").property("standard_resolution")
                       .property("width").toString());
        message.insert("image_standard_resolution_height", it.value().property("data")
                       .property(index).property("images").property("standard_resolution")
                       .property("height").toString());

        message.insert("image_thumbnail", it.value().property("data")
                       .property(index).property("images").property("thumbnail")
                       .property("url").toString());
        message.insert("image_thumbnail_width", it.value().property("data")
                       .property(index).property("images").property("thumbnail")
                       .property("width").toString());
        message.insert("image_thumbnail_height", it.value().property("data")
                       .property(index).property("images").property("thumbnail")
                       .property("height").toString());

        message.insert("location_id", it.value().property("data")
                       .property(index).property("location").property("id").toString());
        message.insert("location_latitude", it.value().property("data")
                       .property(index).property("location").property("latitude").toString());
        message.insert("location_longitude", it.value().property("data")
                       .property(index).property("location").property("longitude").toString());
        message.insert("location_name", it.value().property("data")
                       .property(index).property("location").property("name").toString());

        message.insert("user_username", it.value().property("data")
                       .property(index).property("user").property("username").toString());
        message.insert("user_full_name", it.value().property("data")
                       .property(index).property("user").property("full_name").toString());
        message.insert("user_profile_picture", it.value().property("data")
                       .property(index).property("user").property("profile_picture").toString());
        message.insert("user_id", it.value().property("data")
                       .property(index).property("user").property("id").toString());

        message.insert("image_id", it.value().property("data").property(index).property("id").toString());
        message.insert("image_type", it.value().property("data").property(index).property("type").toString());
        message.insert("image_filter", it.value().property("data").property(index).property("filter").toString());
        message.insert("image_link", it.value().property("data").property(index).property("link").toString());
        message.insert("created_time", it.value().property("data").property(index).property("created_time").toString());

        message.insert("caption_created_time", it.value().property("data")
                       .property(index).property("caption").property("created_time").toString());
        message.insert("caption_text", it.value().property("data")
                       .property(index).property("caption").property("text").toString());
        message.insert("caption_id", it.value().property("data")
                       .property(index).property("caption").property("id").toString());

        message.insert("caption_from_username", it.value().property("data")
                       .property(index).property("caption").property("from").property("username").toString());
        message.insert("caption_from_full_name", it.value().property("data")
                       .property(index).property("caption").property("from").property("full_name").toString());
        message.insert("caption_from_type", it.value().property("data")
                       .property(index).property("caption").property("from").property("type").toString());
        message.insert("caption_from_id", it.value().property("data")
                       .property(index).property("caption").property("from").property("id").toString());

        message.insert("comments_count", it.value().property("data").property(index).property("comments").property("count").toString());
        message.insert("comments", parseComments(it));

        message.insert("likes_count", it.value().property("data").property(index).property("likes").property("count").toString());
        message.insert("likes", parseLikes(it));

        list.append(message);
    }

    return list;
}

QVariantList InstagramConnection::parseComments(const QScriptValueIterator &it)
{
    QVariantList comments;

    QVariantList commentslist = it.value().property("data").property("comments").property("data").toVariant().toList();
    int commentslistLenght = commentslist.length();

    if(commentslistLenght > 0) {
        for(int comment_index = 0; comment_index < commentslistLenght; comment_index++) {
            QVariantMap comment;
            comment.insert("comment_created_time", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("created_time").toString());
            comment.insert("comment_text", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("text").toString());
            comment.insert("comment_id", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("id").toString());
            comment.insert("comment_from_username", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("from").property("username").toString());
            comment.insert("comment_from_profile_picture", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("from").property("profile_picture").toString());
            comment.insert("comment_from_id", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("from").property("id").toString());
            comment.insert("comment_from_full_name", it.value().property("data")
                           .property("comments").property("data")
                           .property(comment_index).property("from").property("full_name").toString());

            comments.append(comment);
        }
    }

    return comments;
}

QVariantList InstagramConnection::parseLikes(const QScriptValueIterator &it)
{
    QVariantList likes;
    QVariantList likeslist = it.value().property("data").property("likes").property("data").toVariant().toList();
    int likeslistLenght = likeslist.length();

    if(likeslistLenght > 0) {
        for(int like_index = 0; like_index < likeslistLenght; like_index++) {
            QVariantMap like;
            like.insert("like_username", it.value().property("data")
                           .property("likes").property("data")
                           .property(like_index).property("username").toString());
            like.insert("like_full_name", it.value().property("data")
                           .property("likes").property("data")
                           .property(like_index).property("full_name").toString());
            like.insert("like_id", it.value().property("data")
                           .property("likes").property("data")
                           .property(like_index).property("id").toString());
            like.insert("like_profile_picture", it.value().property("data")
                           .property("likes").property("data")
                           .property(like_index).property("profile_picture").toString());

            likes.append(like);
        }
    }

    return likes;
}

void InstagramConnection::authenticationSucceeded()
{
    setTransmitting(false);
    setBusy(false);
    setState(Logged);
    setAuthenticated(true);
    emit authenticateCompleted(true);
}

bool InstagramConnection::storeCredentials()
{
    QSettings settings("Nokia", consumerKey());
    settings.setValue(OAUTH_ACCESS_TOKEN, accessToken());

    return settings.status() == QSettings::NoError;
}

bool InstagramConnection::restoreCredentials()
{
    QSettings settings("Nokia", consumerKey());
    setAccessToken(settings.value(OAUTH_ACCESS_TOKEN).toString());

    // If the access token, the app should be authenticated
    if (!m_accessToken.isEmpty())
        authenticationSucceeded();

    return settings.status() == QSettings::NoError;
}

void InstagramConnection::acquireAccessToken()
{
    setState(AcquiringAccessToken);
    setBusy(true);
    setTransmitting(true);

    QUrl params;
    params.addQueryItem(OAUTH_CLIENT_ID, m_consumerKey);
    params.addQueryItem(OAUTH_CLIENT_SECRET, m_consumerSecret);
    params.addQueryItem(OAUTH_GRANT_TYPE, OAUTH_GRANT_TYPE_VALUE);
    params.addQueryItem(OAUTH_REDIRECT_URI, m_callbackUrl);
    params.addQueryItem(OAUTH_CODE, m_requestToken);
    params.addQueryItem("scope", "likes+comments+relationship");

    QByteArray data;
    data = params.encodedQuery();

    QNetworkRequest req(QUrl(OAUTH_ACCESSTOKEN_URL));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    m_ongoingRequest = m_networkManager.post(req, data);
    connect(m_ongoingRequest, SIGNAL(finished()),
            this, SLOT(onAccessTokenReply()));
}

void InstagramConnection::onAccessTokenReply()
{
    m_accessToken.clear();
    const int requestError = m_ongoingRequest->error();
    QString errorStr = m_ongoingRequest->errorString();

    if (requestError == QNetworkReply::NoError) {
        QByteArray data = m_ongoingRequest->readAll();
        qDebug() << data;
        deleteReply();

        setBusy(false);
        emit retrieveAuthorizationCompleted(requestError == QNetworkReply::NoError,
                                       parseRetrievedAuthorization(data));
    }
    else
        emitError();
}

QVariantList InstagramConnection::parseRetrievedAuthorization(const QByteArray &result)
{
    QVariantList list;
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");
    QScriptValueIterator it(scValue);

    bool authorized = false;

    while (it.hasNext() && authorized == false) {
        it.next();

        QVariantMap message;

        qDebug() << it.value().property("access_token").toString();
        qDebug() << it.value().property("user").property("username").toString();

        message.insert("access_token", it.value().property("access_token").toString());
        m_accessToken = it.value().property("access_token").toString();

        message.insert("user_username", it.value().property("user").property("username").toString());
        message.insert("user_bio", it.value().property("user").property("bio").toString());
        message.insert("user_website", it.value().property("user").property("website").toString());
        message.insert("user_profile_picture", it.value().property("user").property("profile_picture").toString());
        message.insert("user_full_name", it.value().property("user").property("full_name").toString());
        message.insert("user_id", it.value().property("user").property("id").toString());

        list.append(message);

        if (!m_accessToken.isEmpty())
            authorized = true;
    }

    if (authorized == true)
        authenticationSucceeded();

    return list;
}

void InstagramConnection::deleteReply()
{
    m_ongoingRequest->close();
    m_ongoingRequest->deleteLater();
    m_ongoingRequest = 0;
}

SocialConnectionError* InstagramConnection::parseError(const QByteArray &result)
{
    QScriptEngine engine;
    QScriptValue scValue = engine.evaluate("[" + QString(result) + "]");

    QScriptValueIterator it(scValue);
    it.next();

    SocialConnectionError *err = new SocialConnectionError(it.value().property("meta").property("error_type").toString(),
                                                          it.value().property("meta").property("code").toString(),
                                                          it.value().property("meta").property("error_message").toString());
    return err;
}

void InstagramConnection::emitError()
{
    QByteArray data = m_ongoingRequest->readAll();
    qDebug() << data;
    deleteReply();
    setBusy(false);
    SocialConnectionError *err = parseError(data);
    emit error(err->errorType(), err->errorCode(), err->errorMessage());
    delete err;
}
