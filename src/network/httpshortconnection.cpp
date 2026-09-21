#include "httpshortconnection.h"
#include "tcplongconnection.h"
#include "../utils/userinfo.h"

HttpShortConnection &HttpShortConnection::getHttpClient()
{
    static HttpShortConnection hsc;
    return hsc;
}

void HttpShortConnection::uploadAvatar(const QString &filePath)
{
    if(!TcpLongConnection::getTcpClient().isConnect())
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(filePath.isEmpty())
    {
        emit mainState(false, "上传失败，请稍后再试");
        return;
    }

    QImage image(filePath);
    if(image.isNull())
    {
        emit mainState(false, "上传失败，请稍后再试");
        return;
    }
    //备份当前头像
    UserInfo::getUserInfo().backupAvatarUrl();

    QImage image_scaled = image.scaled(400,400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray imgData;
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    image_scaled.save(&buffer, "JPEG", 80);
    buffer.close();

    QString localUrl = "local://avatar_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    ImageCacheManager::getManager().insertCache(localUrl, imgData);

    QString cacheFilePath = ImageCacheManager::getManager().getCacheFilePath(localUrl);
    if(cacheFilePath.isEmpty())
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        ImageCacheManager::getManager().removeCache(localUrl);
        return;
    }

    uploadMedia(MediaType::Image, cacheFilePath, [this, localUrl](const QString& url){
        ImageCacheManager::getManager().migrateCache(localUrl, url);
        UserInfo::getUserInfo().setAvatarUrl(url);
        TcpLongConnection::getTcpClient().sendUpadteAvatar(url);
    }, true, [this, localUrl](const QString& info){
        ImageCacheManager::getManager().removeCache(localUrl);
        emit mainState(false, info);
    });
}

void HttpShortConnection::uploadMedia(MediaType type, const QString &filePath, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    if(!TcpLongConnection::getTcpClient().isConnect())
    {
        if(failed_notice)
            emit mainState(false, "无法连接服务器，请稍后再试");
        if(cb_failed)
            cb_failed("无法连接服务器，请稍后再试");
        return;
    }

    if(filePath.isEmpty() || !QFile::exists(filePath))
    {
        if(failed_notice)
            emit mainState(false, "无法连接服务器，请稍后再试");
        if(cb_failed)
            cb_failed("无法连接服务器，请稍后再试");
        return;
    }

    QFileInfo info(filePath);
    qint64 fileSize = info.size();
    if(fileSize <= 0)
    {
        if(failed_notice)
            emit mainState(false, "空文件");
        if(cb_failed)
            cb_failed("空文件");
        return;
    }

    QString mimeType;
    QString suffix;
    if(type == MediaType::Image)
    {
        if(fileSize > 10LL * 1024 * 1024)
        {
            if(failed_notice)
                emit mainState(false, "图片大小要 ≤10 MB");
            if(cb_failed)
                cb_failed("图片大小要 ≤10 MB");
            return;
        }

        //读取文件
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QByteArray fileData = file.readAll();
        file.close();

        QByteArray format = getImageFormat(fileData);
        if(format.isEmpty() || (format != "JPEG" && format != "PNG"))
        {
            QImage image;
            if(!image.loadFromData(fileData))
            {
                if(failed_notice)
                    emit mainState(false, "不支持的图片格式");
                if(cb_failed)
                    cb_failed("不支持的图片格式");
                return;
            }

            //尝试转换为PNG
            QByteArray newData;
            QBuffer buffer(&newData);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            buffer.close();
            fileData = newData;
            format = "PNG";
        }

        mimeType = (format == "JPEG") ? "image/jpeg" : "image/png";
        suffix = (format == "JPEG" ? "jpg" : "png");
        QByteArray md5 = QCryptographicHash::hash(fileData, QCryptographicHash::Md5).toHex();
        sendUploadInit(type, filePath, fileSize, md5, suffix, mimeType, fileData, cb_success, failed_notice, cb_failed);
        return;
    }
    else if(type == MediaType::Video)
    {
        if(fileSize > 1024LL * 1024 * 1024)
        {
            if(failed_notice)
                emit mainState(false, "视频大小要 ≤1 GB");
            if(cb_failed)
                cb_failed("视频大小要 ≤1 GB");
            return;
        }

        suffix = info.suffix().toLower();
        if(suffix != "mp4" && suffix != "mov" && suffix != "webm")
        {
            if(failed_notice)
                emit mainState(false, "不支持的视频格式（仅支持 mp4/mov/webm）");
            if(cb_failed)
                cb_failed("不支持的视频格式（仅支持 mp4/mov/webm）");
            return;
        }

        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QByteArray header = file.read(12);
        file.close();

        suffix = isVideo(header);
        if(suffix.isEmpty())
        {
            if(failed_notice)
                emit mainState(false, "不支持的视频格式（仅支持 mp4/mov/webm）");
            if(cb_failed)
                cb_failed("不支持的视频格式（仅支持 mp4/mov/webm）");
            return;
        }

        if(suffix == "mp4")
            mimeType = "video/mp4";
        else if(suffix == "mov")
            mimeType = "video/quicktime";
        else if(suffix == "webm")
            mimeType = "video/webm";
    }
    else if(type == MediaType::File)
    {
        if(fileSize > 1024LL * 1024 * 1024)
        {
            if(failed_notice)
                emit mainState(false, "文件大小要 ≤1 GB");
            if(cb_failed)
                cb_failed("文件大小要 ≤1 GB");
            return;
        }

        suffix = info.suffix().toLower();

        mimeType = "application/octet-stream";
    }
    else
    {
        if(failed_notice)
            emit mainState(false, "不支持的类型");
        if(cb_failed)
            cb_failed("不支持的类型");
        return;
    }

    QPointer<HttpShortConnection> self(this);

    auto future = QtConcurrent::run([=](){
        QCryptographicHash hash(QCryptographicHash::Md5);
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMetaObject::invokeMethod(self, [=](){
                if(!self)
                    return;
                if(failed_notice)
                    emit self->mainState(false, "上传失败，请稍后再试");
                if(cb_failed)
                    cb_failed("上传失败，请稍后再试");
                return;
            }, Qt::QueuedConnection);
            return;
        }

        const qint64 chunk_const = 4 * 1024 * 1024;
        while(!file.atEnd())
        {
            QByteArray chunk = file.read(chunk_const);
            if(chunk.isEmpty())
                break;
            hash.addData(chunk);
        }
        file.close();

        QByteArray md5 = hash.result().toHex();

        QMetaObject::invokeMethod(self, [=](){
            if(!self)
                return;
            self->sendUploadInit(type, filePath, fileSize, md5, suffix, mimeType, QByteArray(), cb_success, failed_notice, cb_failed);
        }, Qt::QueuedConnection);
    });
}

void HttpShortConnection::getImage(const QString &url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess, bool failed_notice)
{
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());
    QNetworkReply* reply = this->httpmanager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, this, url, retryTime, failed_notice, onSuccess](){
        reply->deleteLater();

        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([this, url, retryTime, onSuccess, failed_notice](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess)
                {
                    if(!newAccessToken.isEmpty())
                    {
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        getImage(url, retryTime, onSuccess, failed_notice);
                    }
                    else
                    {
                        emit refreshExpiredExit();
                    }
                    return;
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                        return;
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "获取头像信息失败");
                    }
                }
            });
            return;
        }

        if(reply->error() == QNetworkReply::ContentNotFoundError)
        {
            if(onSuccess)
                onSuccess(QByteArray(), ImageError::NotFound);
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(retryTime > 1)
            {
                QTimer::singleShot(2000, [url, retryTime, onSuccess, failed_notice](){
                    HttpShortConnection::getHttpClient().getImage(url, retryTime - 1, onSuccess, failed_notice);
                });
            }
            else
            {
                if(failed_notice)
                    emit mainState(false, "获取头像信息失败");
            }
            return;
        }

        QByteArray data = reply->readAll();
        if(onSuccess)
        {
            onSuccess(data, ImageError::NoError);
        }
    });
}

HttpShortConnection::HttpShortConnection(QObject *parent)
    : QObject{parent}
{
    httpmanager = new QNetworkAccessManager(this);
}

QByteArray HttpShortConnection::getImageFormat(const QByteArray &data) const
{
    if(data.startsWith("\xFF\xD8"))
        return "JPEG";
    if(data.startsWith("\x89PNG\r\n\x1A\n"))
        return "PNG";
    if(data.startsWith("GIF87a") || data.startsWith("GIF89a"))
        return "GIF";

    return QByteArray();
}

QString HttpShortConnection::isVideo(const QByteArray &data) const
{
    if(data.size() < 12)
        return "";

    if(data.mid(4, 4) == "ftyp")
    {
        QByteArray brand = data.mid(8, 4);
        if(brand == "qt  ")
            return "mov";
        return "mp4";
    }

    if(static_cast<unsigned char>(data[0]) == 0x1A &&
        static_cast<unsigned char>(data[1]) == 0x45 &&
        static_cast<unsigned char>(data[2]) == 0xDF &&
        static_cast<unsigned char>(data[3]) == 0xA3)
    {
        return "webm";
    }

    return "";
}

void HttpShortConnection::sendUploadInit(MediaType type, const QString &filePath, qint64 fileSize, const QByteArray &md5, const QString &suffix, const QString &mimeType, const QByteArray &fileData, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    QNetworkRequest request(QUrl("http://192.168.153.128:9003/upload/init"));
    request.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["Md5"] = QString::fromLatin1(md5);
    obj["Suffix"] = suffix;
    obj["Size"] = QString::number(fileSize);

    QNetworkReply* reply = this->httpmanager->post(request, QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, [=](){
        reply->deleteLater();
        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([=](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess)
                {
                    if(!newAccessToken.isEmpty())
                    {
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        sendUploadInit(type, filePath, fileSize, md5, suffix, mimeType, fileData, cb_success, failed_notice, cb_failed);
                    }
                    else
                        emit refreshExpiredExit();
                    return;
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                        return;
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "上传失败，请稍后再试");
                        if(cb_failed)
                            cb_failed("上传失败，请稍后再试");
                        return;
                    }
                }
            });
            return;
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if(doc.isNull() || !doc.isObject())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QJsonObject resp = doc.object();
        bool isExists = resp.value("Exists").toBool();
        QString url = doc.object().value("Url").toString();
        if(url.isEmpty())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        if(isExists)
        {
            if(cb_success)
                cb_success(url);
            return;
        }

        if(resp.contains("UploadId"))
        {
            QString objectKey = resp.value("ObjectKey").toString();
            QString uploadId = resp.value("UploadId").toString();
            qint64 partSize = resp.value("PartSize").toString().toLongLong();

            if(objectKey.isEmpty() || uploadId.isEmpty() || partSize <= 0)
            {
                if(failed_notice)
                    emit mainState(false, "上传失败，请稍后再试");
                if(cb_failed)
                    cb_failed("上传失败，请稍后再试");
                return;
            }
            multipartUpload(filePath, partSize, objectKey, uploadId, url, cb_success, failed_notice, cb_failed);
            return;
        }

        QString uploadUrl = resp.value("UploadUrl").toString();
        if(uploadUrl.isEmpty())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QNetworkRequest putReq(uploadUrl);
        putReq.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);

        QNetworkReply* reply_upload = nullptr;
        QFile* uploadFile = nullptr;

        if(!fileData.isEmpty())
            reply_upload = this->httpmanager->put(putReq, fileData);
        else
        {
            uploadFile = new QFile(filePath);
            if(!uploadFile->open(QIODevice::ReadOnly))
            {
                uploadFile->deleteLater();
                if(failed_notice)
                    emit mainState(false, "上传失败，请稍后再试");
                if(cb_failed)
                    cb_failed("上传失败，请稍后再试");
                return;
            }
            reply_upload = this->httpmanager->put(putReq, uploadFile);
        }

        connect(reply_upload, &QNetworkReply::finished, this, [this, reply_upload, cb_success, cb_failed, failed_notice, url, uploadFile](){
            reply_upload->deleteLater();
            if(uploadFile)
                uploadFile->deleteLater();

            if(reply_upload->error() != QNetworkReply::NoError)
            {
                if(failed_notice)
                    emit mainState(false, "上传失败，请稍后再试");
                if(cb_failed)
                    cb_failed("上传失败，请稍后再试");
                return;
            }

            if(cb_success)
                cb_success(url);
        });
    });
}

void HttpShortConnection::multipartUpload(const QString &filePath, qint64 everyPartSize, const QString &objectKey, const QString &uploadId, const QString &finalUrl, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    QFileInfo info(filePath);
    if(!info.exists())
    {
        if(failed_notice)
            emit mainState(false, "上传失败，请稍后再试");
        if(cb_failed)
            cb_failed("上传失败，请稍后再试");
        return;
    }

    int totalParts = static_cast<int>((info.size() + everyPartSize - 1) / everyPartSize);
    auto parts = QSharedPointer<QJsonArray>::create();
    auto next = QSharedPointer<std::function<void()>>::create();

    *next = [=](){
        int currentPart = parts->size() + 1;
        if(currentPart > totalParts)
        {
            sendComplete(objectKey, uploadId, *parts, finalUrl, cb_success, failed_notice, cb_failed);
            return;
        }

        uploadOnePart(filePath, everyPartSize, objectKey, uploadId, currentPart, totalParts, parts, next, finalUrl, cb_success, failed_notice, cb_failed);
    };

    (*next)();
}

void HttpShortConnection::uploadOnePart(const QString &filePath, qint64 partSize, const QString &objectKey, const QString &uploadId, int partNumber, int totalParts, QSharedPointer<QJsonArray> parts, QSharedPointer<std::function<void ()> > next, const QString &finalUrl, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    qint64 offset = static_cast<qint64>(partNumber - 1) * partSize;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly) || !file.seek(offset))
    {
        if(failed_notice)
            emit mainState(false, "上传失败，请稍后再试");
        if(cb_failed)
            cb_failed("上传失败，请稍后再试");
        return;
    }

    QByteArray partData = file.read(partSize);
    file.close();

    QUrl url("http://192.168.153.128:9003/upload/part");
    QUrlQuery query;
    query.addQueryItem("ObjectKey", objectKey);
    query.addQueryItem("UploadId", uploadId);
    query.addQueryItem("partNumber", QString::number(partNumber));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QNetworkReply* reply = this->httpmanager->post(req, partData);
    connect(reply, &QNetworkReply::finished, this, [=](){
        reply->deleteLater();

        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([=](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess && !newAccessToken.isEmpty())
                {
                    UserInfo::getUserInfo().setAccessToken(newAccessToken);
                    uploadOnePart(filePath, partSize, objectKey, uploadId, partNumber, totalParts, parts, next, finalUrl, cb_success, failed_notice, cb_failed);
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "上传失败，请稍后再试");
                        if(cb_failed)
                            cb_failed("上传失败，请稍后再试");
                    }
                }
            });
            return;
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString etag = doc.object().value("ETag").toString();
        if(etag.isEmpty())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QJsonObject part;
        part["PartNumber"] = QString::number(partNumber);
        part["ETag"] = etag;
        parts->append(part);

        (*next)();
    });
}

void HttpShortConnection::sendComplete(const QString &objectKey, const QString &uploadId, const QJsonArray &parts, const QString &finalUrl, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    QNetworkRequest req(QUrl("http://192.168.153.128:9003/upload/complete"));
    req.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["ObjectKey"] = objectKey;
    obj["UploadId"] = uploadId;
    obj["Parts"] = parts;

    QNetworkReply* reply = this->httpmanager->post(req, QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, [=](){
        reply->deleteLater();

        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([=](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess)
                {
                    if(!newAccessToken.isEmpty())
                    {
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        sendComplete(objectKey, uploadId, parts, finalUrl, cb_success, failed_notice, cb_failed);
                    }
                    else
                    {
                        emit refreshExpiredExit();
                    }
                    return;
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                        return;
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "上传失败，请稍后再试");
                    }
                }
            });
            return;
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString url = doc.object().value("Url").toString();
        if(url.isEmpty())
            url = finalUrl;

        if(cb_success)
            cb_success(url);
    });
}
