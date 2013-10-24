#include "instagramrequest.h"

InstagramRequest::InstagramRequest(QObject *parent) :
    QObject(parent)
{
}

void InstagramRequest::setConsumerKey(const QString &consumerKey)
{
    m_consumerKey = consumerKey;
}

void InstagramRequest::setConsumerSecret(const QString &consumerSecret)
{
    m_consumerSecret = consumerSecret;
}

void InstagramRequest::setAccessToken(const QString &accessToken)
{
    m_accessToken = accessToken;
}

void InstagramRequest::setRequestToken(const QString &requestToken)
{
    m_requestToken = requestToken;
}
