#ifndef SOCIALCONNECTIONERROR_H
#define SOCIALCONNECTIONERROR_H

#include <QObject>

class SocialConnectionError : public QObject
{
    Q_OBJECT
public:
    explicit SocialConnectionError(const QString &errorType, const QString &errorCode, const QString &errorMessage, QObject *parent = 0);

public:
    QString errorType() const;
    QString errorCode() const;
    QString errorMessage() const;
    
signals:
    
public slots:

private:
    QString m_errorType;
    QString m_errorCode;
    QString m_errorMessage;
    
};

#endif // SOCIALCONNECTIONERROR_H
