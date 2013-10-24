#ifndef INSTAGRAMREQUEST_H
#define INSTAGRAMREQUEST_H

#include <QObject>
#include <QNetworkRequest>
#include <QMap>
#include <QVariantMap>

#include "instagramconstants.h"

class InstagramRequest : public QObject
{
    Q_OBJECT
public:
    explicit InstagramRequest(QObject *parent = 0);
    
public:
    void setConsumerKey(const QString &consumerKey);
    void setConsumerSecret(const QString &consumerSecret);
    void setAccessToken (const QString &accessToken);
    void setRequestToken (const QString &requestToken);

private:
    /*QNetworkRequest createRequest(const QUrl& requestUrl, const QString httpMethod,
                                  QVariantMap params = QVariantMap());*/

    QString generateAuthHeader(const QMap<QString, QString> &requestHeaders);
    
private:
    QString m_consumerKey;
    QString m_consumerSecret;
    QString m_accessToken;
    QString m_requestToken;
    
};

#endif // INSTAGRAMREQUEST_H
