#include "socialconnectionerror.h"

SocialConnectionError::SocialConnectionError(const QString &errorType, const QString &errorCode, const QString &errorMessage, QObject *parent) :
    QObject(parent), m_errorType(errorType), m_errorCode(errorCode), m_errorMessage(errorMessage)
{
}

QString SocialConnectionError::errorType() const
{
    return m_errorType;
}

QString SocialConnectionError::errorCode() const
{
    return m_errorCode;
}

QString SocialConnectionError::errorMessage() const
{
    return m_errorMessage;
}
