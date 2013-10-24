#ifndef INSTAGRAMCONNECTION_H
#define INSTAGRAMCONNECTION_H

#include <QNetworkAccessManager>
#include <QString>
#include <QVariantMap>
#include <QScriptValueIterator>

#include "socialconnection.h"
#include "socialconnectionerror.h"

class InstagramRequest;

class InstagramConnection : public SocialConnection
{
    Q_OBJECT
    Q_ENUMS(AuthorizationMethod)

    Q_PROPERTY(QString consumerKey READ consumerKey WRITE setConsumerKey NOTIFY consumerKeyChanged)
    Q_PROPERTY(QString consumerSecret READ consumerSecret WRITE setConsumerSecret NOTIFY consumerSecretChanged)
    Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(QString callbackUrl READ callbackUrl WRITE setCallbackUrl NOTIFY callbackUrlChanged)
    Q_PROPERTY(QString requestToken READ requestToken WRITE setRequestToken NOTIFY requestTokenChanged)

public:
    explicit InstagramConnection(QObject *parent = 0);
    ~InstagramConnection();

public:
    QString consumerKey() const;
    void setConsumerKey(const QString &consumerKey);

    QString consumerSecret() const;
    void setConsumerSecret(const QString &consumerSecret);

    QString accessToken() const;
    void setAccessToken (const QString &accessToken);

    QString callbackUrl() const;
    void setCallbackUrl(const QString &callbackUrl);

    QString requestToken() const;
    void setRequestToken(const QString &token);

    enum AuthorizationMethod {
        Code,
        Token
    };
    AuthorizationMethod m_authMetod;

    QString m_authScope;

    // Virtual method implementations from the SocialConnect base class.
    bool authenticate();
    bool deauthenticate();
    bool postMessage(const QVariantMap &message);
    bool retrieveMessageCount();
    bool retrieveMessages(const QString &from, const QString &to, int max);
    void cancel();

    bool storeCredentials();
    bool restoreCredentials();
    bool removeCredentials();

public slots:
    void setAuthorizationMethod(AuthorizationMethod method);
    void setAuthorizationScope(const QString &scope);

    // Users
    bool getUser(QString userId);
    bool getUserSelfFeed(const QString &count = "", const QString &min_id = "", const QString &max_id = "");
    bool getUserMediaRecent(const QString &user, const QString &count = "",
                            const QString &min_id = "", const QString &max_id = "",
                            const QString &min_timestamp = "", const QString &max_timestamp = "");
    bool getUserSelfMediaLiked(const QString &count = "", const QString &max_like_id = "");
    bool getUserSearch(const QString &key, const QString &count = "");

    // Relationships
    bool getUserFollows(const QString &user);
    bool getUserFollowedBy(const QString &user);
    bool getUserSelfRequestedBy();
    bool getUserRelationship(const QString &user);
    bool setUserRelationship(const QString &user, const QString &action);

    // Media
    bool getMedia(const QString &media);
    bool getMediaSearch(const QString &lat, const QString &lng,
                        const QString &min_timestamp, const QString &max_timestamp, const QString &distance);
    bool getMediaPopular();

    // Comments
    bool getComments(const QString &media);
    bool postComment(const QString &media, const QString &text);
    bool deleteComment(const QString &media, const QString &comment);

    // Likes
    bool getLikes(const QString &media);
    bool postLike(const QString &media);
    bool deleteLike(const QString &media);

    // Tags
    bool getTag(const QString &tag);
    bool getTagMediaRecent(const QString &tag, const QString &min_id = "", const QString &max_id = "");
    bool getTagsSearch(const QString &tag);

    // Locations
    bool getLocation(const QString &location);
    bool getLocationMediaRecent(const QString &location);
    bool getLocationSearch(const QString &lat, const QString &lng,
                           const QString &foursquare_v2_id, const QString &distance);

signals:
    void consumerKeyChanged(const QString &consumerKey);
    void consumerSecretChanged(const QString &consumerSecret);
    void accessTokenChanged(const QString &accessToken);
    void callbackUrlChanged(const QString &callbackUrl);
    void requestTokenChanged(const QString &token);

    void retrieveAuthorizationCompleted(bool success, const QVariantList &messages);
    void retrieveUserCompleted(bool success, const QVariantList &messages);
    void retrieveUserSelfFeedCompleted(bool success, const QVariantList &messages);
    void retrieveUserMediaRecentCompleted(bool success, const QVariantList &messages);
    void retrieveUserSelfMediaLikedCompleted(bool success, const QVariantList &messages);
    void retrieveUserSearchCompleted(bool success, const QVariantList &messages);
    void retrieveUserFollowsCompleted(bool success, const QVariantList &messages);
    void retrieveUserFollowedByCompleted(bool success, const QVariantList &messages);
    void retrieveUserSelfRequestedByCompleted(bool success, const QVariantList &messages);
    void retrieveUserRelationshipCompleted(bool success, const QVariantList &messages);
    void setUserRelationshipCompleted(bool success, const QVariantList &messages);
    void retrieveMediaCompleted(bool success, const QVariantList &messages);
    void retrieveMediaSearchCompleted(bool success, const QVariantList &messages);
    void retrieveMediaPopularCompleted(bool success, const QVariantList &messages);
    void retrieveCommentsCompleted(bool success, const QVariantList &messages);
    void postCommentCompleted(bool success, const QVariantList &messages);
    void deleteCommentCompleted(bool success, const QVariantList &messages);
    void retrieveLikesCompleted(bool success, const QVariantList &messages);
    void postLikeCompleted(bool success, const QVariantList &messages);
    void deleteLikeCompleted(bool success, const QVariantList &messages);
    void getTagCompleted(bool success, const QVariantList &messages);
    void getTagMediaRecentCompleted(bool success, const QVariantList &messages);
    void getTagsSearchCompleted(bool success, const QVariantList &messages);
    void getLocationCompleted(bool success, const QVariantList &messages);
    void getLocationMediaRecentCompleted(bool success, const QVariantList &messages);
    void getLocationSearchCompleted(bool success, const QVariantList &messages);

protected slots:
    void onUrlChanged(const QUrl &url);

private slots:
    void onAccessTokenReply();
    void onGetUserReply();
    void onGetUserSelfFeedReply();
    void onGetUserMediaRecentReply();
    void onGetUserSelfMediaLikedReply();
    void onGetUserSearchReply();
    void onGetUserFollowsReply();
    void onGetUserFollowedByReply();
    void onGetUserSelfRequestedByReply();
    void onGetUserRelationshipReply();
    void onSetUserRelationshipReply();
    void onGetMediaReply();
    void onGetMediaSearchReply();
    void onGetMediaPopularReply();
    void onGetCommentsReply();
    void onPostCommentReply();
    void onDeleteCommentReply();
    void onGetLikesReply();
    void onPostLikeReply();
    void onDeleteLikeReply();
    void onGetTagReply();
    void onGetTagMediaRecentReply();
    void onGetTagsSearchReply();
    void onGetLocationReply();
    void onGetLocationMediaRecentReply();
    void onGetLocationSearchReply();

private:    // Members

    // Instagram login internal state tracking and setters & getters for it.
    enum State {
        NotLogged = 0,
        AcquiringRequestToken,
        Authorizing,
        AcquiringAccessToken,
        Logged
    };
    State state() const;
    void setState(State state);

    // Continues the authentication process after user has entered his
    // credentials to the Instagram login webview.
    void acquireAccessToken();

    // Methods for emitting signals and setting the State & Busy status correctly
    // after successful / failed authentication attempt.
    void authenticationSucceeded();
    void authenticationFailed(const QString &errorMsg, const int errorCode = 0);

    // Check if the reply had some errors. Currently only reacts to
    // "401 Unauthorized" by setting the state to NotLogged & unauthenticated.
    int checkReplyErrors();

    // Closes the QNetworkReply & nullifies the pointer. Should be called
    // always after each request completed signal handler.
    void deleteReply();

    // Traverses the retrieveMessages reply and creates a messagelist in the
    // specified format. See socialconnection.h for details.
    QVariantList parseRetrievedAuthorization(const QByteArray &result);
    QVariantList parseRetrievedUser(const QByteArray &result);
    QVariantList parseRetrievedUsers(const QByteArray &result);
    QVariantList parseRetrievedUserRelationship(const QByteArray &result);
    QVariantList parseSetUserRelationship(const QByteArray &result);
    QVariantList parseRetrievedMedia(const QByteArray &result);
    QVariantList parseRetrievedMedias(const QByteArray &result);
    QVariantList parseRetrievedComments(const QByteArray &result);
    QVariantList parsePostedComment(const QByteArray &result);
    QVariantList parseDeletedComment(const QByteArray &result);
    QVariantList parseRetrievedLikes(const QByteArray &result);
    QVariantList parsePostedLike(const QByteArray &result);
    QVariantList parseDeletedLike(const QByteArray &result);

    QVariantList parseComments(const QScriptValueIterator &it);
    QVariantList parseLikes(const QScriptValueIterator &it);

    QVariantList parseTag(const QByteArray &result);
    QVariantList parseTags(const QByteArray &result);

    QVariantList parseLocation(const QByteArray &result);
    QVariantList parseLocations(const QByteArray &result);

    SocialConnectionError* parseError(const QByteArray &result);
    void emitError();

    // A helper method for clearing the XXXToken etc. QString members.
    void clearAllMembers();

private:    // Data

    InstagramRequest *m_instagramRequest;

    QNetworkReply *m_ongoingRequest;
    QNetworkAccessManager m_networkManager;

    QString m_consumerKey;
    QString m_consumerSecret;
    QString m_callbackUrl;

    QString m_requestToken;
    QString m_accessToken;

    State m_state;
};
#endif // INSTAGRAMCONNECTION_H
