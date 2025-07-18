#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QDebug>
#include <QEasingCurve>
#include <QNetworkRequest>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <algorithm>
#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QBrush>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QSize>
#include <QUrl>
#include <QLabel>

// Static cache initialization
QHash<QString, QPixmap> CrispCircleFlagWidget::s_flagCache;

// CrispSvgWidget - Optimized SVG rendering with proper aspect ratio
CrispSvgWidget::CrispSvgWidget(const QString &file, QWidget *parent)
    : QWidget(parent)
    , m_svgRenderer(std::make_unique<QSvgRenderer>(this))
{
    setStyleSheet("background: transparent;");
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);

    // Try different paths to find your SVG
    if (!file.isEmpty()) {
        QStringList paths = {
            file,                                           // Direct path
            QApplication::applicationDirPath() + "/" + file, // App directory
            QDir::currentPath() + "/" + file,               // Current directory
            ":/" + file                                     // Resource path
        };

        for (const QString& path : paths) {
            if (QFile::exists(path)) {
                m_svgRenderer->load(path);
                if (m_svgRenderer->isValid()) {
                    qDebug() << "Successfully loaded SVG from:" << path;
                    break;
                }
            }
        }

        if (!m_svgRenderer->isValid()) {
            qDebug() << "Failed to load SVG from any path. Tried:" << paths;
        }
    }
}

void CrispSvgWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (m_svgRenderer && m_svgRenderer->isValid()) {
        // Get the SVG's natural size
        QSize svgSize = m_svgRenderer->defaultSize();
        QRect targetRect = rect();

        // Calculate scaling to fit while maintaining aspect ratio
        if (svgSize.isValid()) {
            double scaleX = (double)targetRect.width() / svgSize.width();
            double scaleY = (double)targetRect.height() / svgSize.height();
            double scale = std::min(scaleX, scaleY);

            // Calculate centered position
            int scaledWidth = (int)(svgSize.width() * scale);
            int scaledHeight = (int)(svgSize.height() * scale);
            int x = (targetRect.width() - scaledWidth) / 2;
            int y = (targetRect.height() - scaledHeight) / 2;

            QRect centeredRect(x, y, scaledWidth, scaledHeight);
            m_svgRenderer->render(&painter, centeredRect);
        } else {
            // Fallback to full rect if no natural size
            m_svgRenderer->render(&painter, targetRect);
        }
    } else {
        // Fallback placeholder
        painter.setBrush(QColor(240, 240, 240));
        painter.setPen(QPen(QColor(200, 200, 200), 2));
        painter.drawRoundedRect(rect().adjusted(10, 10, -10, -10), 20, 20);

        painter.setPen(QColor(100, 100, 100));
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "SVG\nMissing");
    }

    QWidget::paintEvent(event);
}

// SimpleButton - Optimized button with external styles
SimpleButton::SimpleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setFixedSize(220, 60);
    setCursor(Qt::PointingHandCursor);
    setObjectName("continueButton");

    // Fallback styling since ResourceManager might not be available
    setStyleSheet(
        "QPushButton#continueButton {"
        "    background-color: #000000;"
        "    color: white;"
        "    font-size: 22px;"
        "    font-weight: 600;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "    border: none;"
        "    border-radius: 30px;"
        "    padding: 15px 30px;"
        "}"
        "QPushButton#continueButton:hover {"
        "    background-color: #333333;"
        "}"
        "QPushButton#continueButton:pressed {"
        "    background-color: #1a1a1a;"
        "}"
        );

    // Add shadow effect
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
}

void SimpleButton::updateText(const QString &text)
{
    setText(text);
}

// WindowControlButton - Optimized with better memory management
WindowControlButton::WindowControlButton(const QString &svgPath, QWidget *parent)
    : QPushButton(parent)
    , m_filePath(svgPath)
    , m_isHovered(false)
{
    setFixedSize(32, 32);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet("background: transparent; border: none;");

    // Try multiple paths for SVG loading
    QStringList paths = {
        QApplication::applicationDirPath() + "/" + svgPath,
        svgPath,
        ":/" + svgPath
    };

    for (const QString& path : paths) {
        m_svgRenderer.reset(new QSvgRenderer(path, this));
        if (m_svgRenderer->isValid()) {
            break;
        }
    }

    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void WindowControlButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect circleRect = rect().adjusted(2, 2, -2, -2);
    QColor backgroundColor = m_isHovered ?
                                 QColor(120, 120, 120, 180) : QColor(80, 80, 80, 150);

    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(circleRect);

    if (m_svgRenderer && m_svgRenderer->isValid()) {
        QRect iconRect = rect().adjusted(10, 10, -10, -10);
        m_svgRenderer->render(&painter, iconRect);
    } else {
        // Fallback drawing
        painter.setPen(QPen(Qt::white, 2));
        QRect iconRect = rect().adjusted(10, 10, -10, -10);

        if (m_filePath.contains("minimize")) {
            int centerY = iconRect.center().y();
            painter.drawLine(iconRect.left(), centerY, iconRect.right(), centerY);
        } else if (m_filePath.contains("close")) {
            painter.drawLine(iconRect.topLeft(), iconRect.bottomRight());
            painter.drawLine(iconRect.topRight(), iconRect.bottomLeft());
        }
    }

    QPushButton::paintEvent(event);
}

void WindowControlButton::enterEvent(QEvent *event)
{
    m_isHovered = true;
    update();
    QPushButton::enterEvent(event);
}

void WindowControlButton::leaveEvent(QEvent *event)
{
    m_isHovered = false;
    update();
    QPushButton::leaveEvent(event);
}

// AnimatedArrowWidget - Optimized animation
AnimatedArrowWidget::AnimatedArrowWidget(QWidget *parent)
    : QWidget(parent)
    , m_rotation(0)
{
    setFixedSize(24, 24);

    QString arrowSvg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" fill=\"none\" viewBox=\"0 0 24 24\">"
        "<path stroke=\"#8c8c8c\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"m19 9-7 7-7-7\"/>"
        "</svg>";

    m_arrowRenderer.reset(new QSvgRenderer(arrowSvg.toUtf8(), this));

    m_rotationAnimation.reset(new QPropertyAnimation(this, "rotation", this));
    m_rotationAnimation->setDuration(250);
    m_rotationAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void AnimatedArrowWidget::setRotation(qreal rotation)
{
    if (qFuzzyCompare(m_rotation, rotation)) return;

    m_rotation = rotation;
    update();
}

void AnimatedArrowWidget::animateToUp()
{
    m_rotationAnimation->setStartValue(m_rotation);
    m_rotationAnimation->setEndValue(180.0);
    m_rotationAnimation->start();
}

void AnimatedArrowWidget::animateToDown()
{
    m_rotationAnimation->setStartValue(m_rotation);
    m_rotationAnimation->setEndValue(0.0);
    m_rotationAnimation->start();
}

void AnimatedArrowWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(m_rotation);
    painter.translate(-width() / 2.0, -height() / 2.0);

    if (m_arrowRenderer && m_arrowRenderer->isValid()) {
        m_arrowRenderer->render(&painter, rect());
    }

    QWidget::paintEvent(event);
}

// CrispCircleFlagWidget - Optimized with caching and timeouts
CrispCircleFlagWidget::CrispCircleFlagWidget(const QString &flagUrl, QWidget *parent)
    : QWidget(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeoutTimer(new QTimer(this))
    , m_currentReply(nullptr)
    , m_isLoading(false)
    , m_pixmapCached(false)
{
    setFixedSize(Config::FLAG_SIZE, Config::FLAG_SIZE);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(Config::NETWORK_TIMEOUT_MS);
    connect(m_timeoutTimer.get(), &QTimer::timeout, this, &CrispCircleFlagWidget::onNetworkTimeout);

    setFlag(flagUrl);
}

int CrispCircleFlagWidget::calculateOptimalScale() const
{
    qreal devicePixelRatio = devicePixelRatioF();
    int scale = static_cast<int>(std::clamp(devicePixelRatio * 2,
                                            static_cast<qreal>(Config::MIN_RENDER_SCALE),
                                            static_cast<qreal>(Config::MAX_RENDER_SCALE)));
    return scale;
}

void CrispCircleFlagWidget::setFlag(const QString &flagUrl)
{
    if (m_currentFlagUrl == flagUrl) return;

    m_currentFlagUrl = flagUrl;

    // Check cache first
    if (s_flagCache.contains(flagUrl)) {
        m_cachedPixmap = s_flagCache[flagUrl];
        m_pixmapCached = true;
        m_isLoading = false;
        update();
        return;
    }

    if (flagUrl.isEmpty()) return;

    // Cancel previous request
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    m_isLoading = true;
    m_pixmapCached = false;

    QUrl url(flagUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "PandaBlur/1.0");
    request.setRawHeader("Accept", "image/svg+xml,image/*");

    m_currentReply = m_networkManager->get(request);
    m_timeoutTimer->start();

    connect(m_currentReply, &QNetworkReply::finished, this, &CrispCircleFlagWidget::onFlagDownloaded);
}

void CrispCircleFlagWidget::onNetworkTimeout()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    m_isLoading = false;
    update();

    qDebug() << "Flag download timeout for:" << m_currentFlagUrl;
}

void CrispCircleFlagWidget::onFlagDownloaded()
{
    m_timeoutTimer->stop();

    if (!m_currentReply) return;

    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray svgData = m_currentReply->readAll();

        if (!svgData.isEmpty()) {
            m_svgRenderer.reset(new QSvgRenderer(svgData, this));

            if (m_svgRenderer->isValid()) {
                renderFlag();
            }
        }
    } else {
        qDebug() << "Flag download failed:" << m_currentReply->errorString();
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    m_isLoading = false;
}

void CrispCircleFlagWidget::renderFlag()
{
    if (!m_svgRenderer || !m_svgRenderer->isValid()) return;

    int scale = calculateOptimalScale();
    QSize renderSize = size() * scale;

    m_cachedPixmap = QPixmap(renderSize);
    m_cachedPixmap.fill(Qt::transparent);

    QPainter painter(&m_cachedPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    m_svgRenderer->render(&painter, QRect(0, 0, renderSize.width(), renderSize.height()));

    // Cache the result
    s_flagCache[m_currentFlagUrl] = m_cachedPixmap;
    m_pixmapCached = true;

    update();
}

void CrispCircleFlagWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (m_pixmapCached && !m_cachedPixmap.isNull()) {
        painter.drawPixmap(rect(), m_cachedPixmap);
    } else if (m_isLoading) {
        // Loading indicator
        painter.setBrush(QColor(245, 245, 245, 200));
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

        painter.setBrush(QColor(180, 180, 180, 150));
        painter.setPen(Qt::NoPen);
        int dotSize = width() / 3;
        QRect dotRect((width() - dotSize) / 2, (height() - dotSize) / 2, dotSize, dotSize);
        painter.drawEllipse(dotRect);
    }

    QWidget::paintEvent(event);
}

// ResourceManager - Singleton for resource management
ResourceManager& ResourceManager::instance()
{
    static ResourceManager instance;
    return instance;
}

ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
{
    // Simplified for now - no resource loading to avoid dependency issues
}

QString ResourceManager::getTranslation(const QString &key, const QString &language)
{
    // Hardcoded translations as fallback
    static const QMap<QString, QMap<QString, QString>> translations = {
        {"EN", {
                   {"title", "Welcome to\nPandaBlur"},
                   {"subtitle", "PandaBlur is a Security Software\nto protect your devices!"},
                   {"continue", "Continue"},
                   {"autoTranslate", "Detects and translates language automatically"}
               }},
        {"NL", {
                   {"title", "Welkom bij\nPandaBlur"},
                   {"subtitle", "PandaBlur is een beveiligingssoftware\nom uw apparaten te beschermen!"},
                   {"continue", "Doorgaan"},
                   {"autoTranslate", "Detecteert en vertaalt taal automatisch"}
               }},
        {"DE", {
                   {"title", "Willkommen bei\nPandaBlur"},
                   {"subtitle", "PandaBlur ist eine Sicherheitssoftware\nzum Schutz Ihrer Geräte!"},
                   {"continue", "Fortfahren"},
                   {"autoTranslate", "Erkennt und übersetzt Sprache automatisch"}
               }},
        {"FR", {
                   {"title", "Bienvenue à\nPandaBlur"},
                   {"subtitle", "PandaBlur est un logiciel de sécurité\npour protéger vos appareils!"},
                   {"continue", "Continuer"},
                   {"autoTranslate", "Détecte et traduit la langue automatiquement"}
               }},
        {"ES", {
                   {"title", "Bienvenido a\nPandaBlur"},
                   {"subtitle", "PandaBlur es un software de seguridad\npara proteger sus dispositivos!"},
                   {"continue", "Continuar"},
                   {"autoTranslate", "Detecta y traduce idioma automáticamente"}
               }},
        {"IT", {
                   {"title", "Benvenuto a\nPandaBlur"},
                   {"subtitle", "PandaBlur è un software di sicurezza\nper proteggere i tuoi dispositivi!"},
                   {"continue", "Continua"},
                   {"autoTranslate", "Rileva e traduce la lingua automaticamente"}
               }},
        {"PT", {
                   {"title", "Bem-vindo ao\nPandaBlur"},
                   {"subtitle", "PandaBlur é um software de segurança\npara proteger seus dispositivos!"},
                   {"continue", "Continuar"},
                   {"autoTranslate", "Detecta e traduz idioma automaticamente"}
               }},
        {"RU", {
                   {"title", "Добро пожаловать в\nPandaBlur"},
                   {"subtitle", "PandaBlur - это программа безопасности\nдля защиты ваших устройств!"},
                   {"continue", "Продолжить"},
                   {"autoTranslate", "Автоматически определяет и переводит язык"}
               }},
        {"CN", {
                   {"title", "欢迎使用\nPandaBlur"},
                   {"subtitle", "PandaBlur是一款安全软件\n用于保护您的设备！"},
                   {"continue", "继续"},
                   {"autoTranslate", "自动检测并翻译语言"}
               }},
        {"JP", {
                   {"title", "PandaBlurへようこそ"},
                   {"subtitle", "PandaBlurはあなたのデバイスを\n保護するセキュリティソフトウェアです！"},
                   {"continue", "続行"},
                   {"autoTranslate", "言語を自動検出して翻訳します"}
               }},
        {"KR", {
                   {"title", "PandaBlur에 오신 것을\n환영합니다"},
                   {"subtitle", "PandaBlur는 귀하의 기기를\n보호하는 보안 소프트웨어입니다!"},
                   {"continue", "계속"},
                   {"autoTranslate", "언어를 자동으로 감지하고 번역합니다"}
               }}
    };

    return translations.value(language).value(key, translations.value("EN").value(key));
}

QPixmap ResourceManager::getFlagPixmap(const QString &countryCode)
{
    Q_UNUSED(countryCode);  // FIX: Remove unused parameter warning
    return QPixmap(); // Simplified
}

QString ResourceManager::getStyleSheet(const QString &name)
{
    // Fallback styles with THINNER scroll bar
    if (name == "dropdown") {
        return
            "QListWidget {"
            "    background-color: rgba(255, 255, 255, 0.98);"
            "    border: 1px solid #d0d0d0;"
            "    border-radius: 16px;"
            "    font-family: 'Segoe UI', Arial, sans-serif;"
            "    font-size: 15px;"
            "    outline: none;"
            "    padding: 5px;"
            "}"
            "QListWidget::item {"
            "    background-color: transparent;"
            "    color: #1a1a1a;"
            "    border-radius: 8px;"
            "    margin: 1px 2px;"
            "    min-height: 50px;"
            "}"
            "QListWidget::item:hover {"
            "    background-color: rgba(240, 240, 240, 0.9);"
            "}"
            "QScrollBar:vertical {"
            "    background: rgba(248, 248, 248, 0.4);"
            "    width: 6px;"  // MADE THINNER - REDUCED FROM 10px TO 6px
            "    border-radius: 3px;"  // ADJUSTED RADIUS TO MATCH NEW WIDTH
            "}"
            "QScrollBar::handle:vertical {"
            "    background: rgba(180, 180, 180, 0.8);"
            "    border-radius: 3px;"  // ADJUSTED RADIUS TO MATCH NEW WIDTH
            "    min-height: 28px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "    background: rgba(140, 140, 140, 0.9);"
            "}"
            "QScrollBar::add-line:vertical {"
            "    height: 0px;"
            "    subcontrol-position: bottom;"
            "    subcontrol-origin: margin;"
            "}"
            "QScrollBar::sub-line:vertical {"
            "    height: 0px;"
            "    subcontrol-position: top;"
            "    subcontrol-origin: margin;"
            "}"
            "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
            "    width: 0px;"
            "    height: 0px;"
            "    background: none;"
            "}"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
            "    background: none;"
            "}";
    }
    return QString();
}

// GeolocationService - Optimized with timeout and error handling
GeolocationService::GeolocationService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeoutTimer(new QTimer(this))
    , m_currentReply(nullptr)
{
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(Config::NETWORK_TIMEOUT_MS);
    connect(m_timeoutTimer.get(), &QTimer::timeout, this, &GeolocationService::onNetworkTimeout);
}

void GeolocationService::detectUserLocation()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    QUrl url("https://ipapi.co/json/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "PandaBlur/1.0");
    request.setRawHeader("Accept", "application/json");

    m_currentReply = m_networkManager->get(request);
    m_timeoutTimer->start();

    connect(m_currentReply, &QNetworkReply::finished, this, &GeolocationService::onLocationDataReceived);
}

void GeolocationService::onNetworkTimeout()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    qDebug() << "Geolocation timeout, using default";
    emit locationFailed();
}

void GeolocationService::onLocationDataReceived()
{
    m_timeoutTimer->stop();

    if (!m_currentReply) return;

    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString countryCode = obj["country_code"].toString().toLower();
        QString languageCode = mapCountryToLanguage(countryCode);

        qDebug() << "Detected location:" << countryCode << "->" << languageCode;
        emit locationDetected(countryCode, languageCode);
    } else {
        qDebug() << "Geolocation failed:" << m_currentReply->errorString();
        emit locationFailed();
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

QString GeolocationService::mapCountryToLanguage(const QString &countryCode)
{
    static const QHash<QString, QString> countryToLanguage = {
        {"nl", "NL"}, {"be", "NL"}, {"us", "EN"}, {"gb", "EN"}, {"ca", "EN"},
        {"au", "EN"}, {"nz", "EN"}, {"ie", "EN"}, {"de", "DE"}, {"at", "DE"},
        {"ch", "DE"}, {"fr", "FR"}, {"es", "ES"}, {"mx", "ES"}, {"ar", "ES"},
        {"co", "ES"}, {"it", "IT"}, {"pt", "PT"}, {"br", "PT"}, {"ru", "RU"},
        {"cn", "CN"}, {"tw", "CN"}, {"hk", "CN"}, {"jp", "JP"}, {"kr", "KR"}
    };

    return countryToLanguage.value(countryCode, Config::DEFAULT_LANGUAGE);
}

// ModernLanguageDropdown - Fully optimized with dynamic sizing and checkmarks
ModernLanguageDropdown::ModernLanguageDropdown(QWidget *parent)
    : QPushButton(parent)
    , m_isHovered(false)
    , m_dropdownVisible(false)
    , m_currentLanguageCode("EN")
{
    setFixedSize(Config::DROPDOWN_WIDTH, 45);
    setCursor(Qt::PointingHandCursor);

    setupLanguageOptions();

    m_currentLanguage = "English (UK)";
    m_currentFlagUrl = "https://hatscripts.github.io/circle-flags/flags/gb.svg";

    m_currentFlag.reset(new CrispCircleFlagWidget(m_currentFlagUrl, this));
    // Better vertical centering for the flag in the button
    m_currentFlag->move(16, (height() - Config::FLAG_SIZE) / 2);

    m_animatedArrow.reset(new AnimatedArrowWidget(this));
    m_animatedArrow->move(width() - 32, (height() - 24) / 2);

    setStyleSheet("background: transparent; border: none;");

    m_geolocationService.reset(new GeolocationService(this));
    connect(m_geolocationService.get(), &GeolocationService::locationDetected,
            this, &ModernLanguageDropdown::onLocationDetected);
    connect(m_geolocationService.get(), &GeolocationService::locationFailed,
            this, &ModernLanguageDropdown::onLocationFailed);

    createModernDropdown();

    connect(this, &QPushButton::clicked, this, &ModernLanguageDropdown::showDropdown);

    // Auto-detect location with delay
    QTimer::singleShot(Config::GEOLOCATION_DELAY_MS,
                       m_geolocationService.get(),
                       &GeolocationService::detectUserLocation);
}

ModernLanguageDropdown::~ModernLanguageDropdown()
{
    if (m_dropdownWidget) {
        m_dropdownWidget->hide();
    }
}

void ModernLanguageDropdown::setupLanguageOptions()
{
    m_languages = {
        {"Nederlands", "NL", "nl"},
        {"English (US)", "EN", "us"},
        {"English (UK)", "EN", "gb"},
        {"Deutsch", "DE", "de"},
        {"Français", "FR", "fr"},
        {"Español", "ES", "es"},
        {"Italiano", "IT", "it"},
        {"Português", "PT", "pt"},
        {"Русский", "RU", "ru"},
        {"中文", "CN", "cn"},
        {"日本語", "JP", "jp"},
        {"한국어", "KR", "kr"}
    };
}

int ModernLanguageDropdown::calculateDropdownHeight() const
{
    int itemCount = m_languages.size();
    int totalHeight = itemCount * Config::DROPDOWN_ITEM_HEIGHT + 10;
    return std::min(totalHeight, Config::DROPDOWN_MAX_HEIGHT);
}

void ModernLanguageDropdown::createModernDropdown()
{
    m_dropdownWidget.reset(new QWidget(nullptr));
    m_dropdownWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_dropdownWidget->setAttribute(Qt::WA_TranslucentBackground);

    int dropdownHeight = calculateDropdownHeight();
    m_dropdownWidget->setFixedSize(Config::DROPDOWN_WIDTH, dropdownHeight);
    m_dropdownWidget->hide();

    auto* layout = new QVBoxLayout(m_dropdownWidget.get());
    layout->setContentsMargins(0, 0, 0, 0);

    m_languageList.reset(new QListWidget(m_dropdownWidget.get()));
    m_languageList->setFixedSize(Config::DROPDOWN_WIDTH, dropdownHeight);

    // Load styles
    QString dropdownStyle = ResourceManager::instance().getStyleSheet("dropdown");
    if (!dropdownStyle.isEmpty()) {
        m_languageList->setStyleSheet(dropdownStyle);
    }

    m_languageList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_languageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    createDropdownItems();
    layout->addWidget(m_languageList.get());
}

void ModernLanguageDropdown::createDropdownItems()
{
    for (const auto &lang : m_languages) {
        auto* item = new QListWidgetItem();

        auto* itemWidget = new QWidget();
        itemWidget->setFixedHeight(Config::DROPDOWN_ITEM_HEIGHT);
        itemWidget->setContentsMargins(0, 0, 0, 0);

        // ADD POINTER CURSOR TO THE ITEM WIDGET
        itemWidget->setCursor(Qt::PointingHandCursor);

        auto* itemLayout = new QHBoxLayout(itemWidget);
        // Increased vertical margins for better flag centering
        itemLayout->setContentsMargins(12, 15, 35, 15);
        itemLayout->setSpacing(12);

        // Flag widget - Better vertical alignment
        auto* flagWidget = new CrispCircleFlagWidget("https://hatscripts.github.io/circle-flags/flags/" + lang.countryCode + ".svg", itemWidget);
        flagWidget->setFixedSize(Config::FLAG_SIZE, Config::FLAG_SIZE);
        flagWidget->setCursor(Qt::PointingHandCursor);  // ADD CURSOR TO FLAG
        itemLayout->addWidget(flagWidget, 0, Qt::AlignVCenter);

        // Language name label
        auto* nameLabel = new QLabel(QString("%1 (%2)").arg(lang.name, lang.code), itemWidget);
        nameLabel->setStyleSheet(
            "QLabel {"
            "    color: #1a1a1a;"
            "    font-size: 15px;"
            "    font-weight: 500;"
            "    font-family: 'Segoe UI', Arial, sans-serif;"
            "    margin: 0px; padding: 0px;"
            "}"
            );
        nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        nameLabel->setCursor(Qt::PointingHandCursor);  // ADD CURSOR TO LABEL
        itemLayout->addWidget(nameLabel, 0, Qt::AlignVCenter);

        // Reduced stretch to bring checkmark more to the left
        itemLayout->addStretch(1);

        // Checkmark widget - Better vertical alignment
        auto* checkmarkWidget = new CrispSvgWidget(":/check.svg", itemWidget);
        checkmarkWidget->setFixedSize(22, 22);
        checkmarkWidget->setStyleSheet("background: transparent; margin-right: 10px;");
        checkmarkWidget->setVisible(false); // Initially hidden
        checkmarkWidget->setCursor(Qt::PointingHandCursor);  // ADD CURSOR TO CHECKMARK
        itemLayout->addWidget(checkmarkWidget, 0, Qt::AlignVCenter);

        // Store data
        item->setData(Qt::UserRole, lang.code);
        item->setData(Qt::UserRole + 1, lang.name);
        item->setData(Qt::UserRole + 2, lang.countryCode);
        item->setData(Qt::UserRole + 3, QVariant::fromValue(checkmarkWidget));
        item->setSizeHint(QSize(Config::DROPDOWN_WIDTH, Config::DROPDOWN_ITEM_HEIGHT));

        m_languageList->addItem(item);
        m_languageList->setItemWidget(item, itemWidget);
    }

    connect(m_languageList.get(), &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        QString code = item->data(Qt::UserRole).toString();
        QString name = item->data(Qt::UserRole + 1).toString();
        onLanguageSelected(name, code);
    });
}

void ModernLanguageDropdown::updateCheckmarks()
{
    // Hide all checkmarks first
    for (int i = 0; i < m_languageList->count(); ++i) {
        QListWidgetItem* item = m_languageList->item(i);
        if (item) {
            QVariant checkmarkData = item->data(Qt::UserRole + 3);
            if (checkmarkData.isValid()) {
                auto* checkmarkWidget = checkmarkData.value<CrispSvgWidget*>();
                if (checkmarkWidget) {
                    checkmarkWidget->setVisible(false);
                }
            }
        }
    }

    // Show checkmark for selected language
    for (int i = 0; i < m_languageList->count(); ++i) {
        QListWidgetItem* item = m_languageList->item(i);
        if (item) {
            QString itemCode = item->data(Qt::UserRole).toString();
            if (itemCode == m_currentLanguageCode) {
                QVariant checkmarkData = item->data(Qt::UserRole + 3);
                if (checkmarkData.isValid()) {
                    auto* checkmarkWidget = checkmarkData.value<CrispSvgWidget*>();
                    if (checkmarkWidget) {
                        checkmarkWidget->setVisible(true);
                    }
                }
                break;
            }
        }
    }
}

void ModernLanguageDropdown::onLocationDetected(const QString &countryCode, const QString &languageCode)
{
    qDebug() << "Setting language based on location:" << countryCode << "->" << languageCode;
    setLanguageByCode(languageCode);
}

void ModernLanguageDropdown::onLocationFailed()
{
    qDebug() << "Location detection failed, using default UK English";
    setLanguageByCode(Config::DEFAULT_LANGUAGE);
}

void ModernLanguageDropdown::setLanguageByCode(const QString &languageCode)
{
    for (const auto &lang : m_languages) {
        if (lang.code == languageCode) {
            m_currentLanguage = lang.name;
            m_currentLanguageCode = languageCode;
            m_currentFlagUrl = "https://hatscripts.github.io/circle-flags/flags/" + lang.countryCode + ".svg";
            m_currentFlag->setFlag(m_currentFlagUrl);
            update();
            updateCheckmarks();

            emit languageChanged(languageCode);
            return;
        }
    }
}

void ModernLanguageDropdown::positionDropdownBelowButton()
{
    QPoint buttonGlobalPos = mapToGlobal(QPoint(0, 0));
    int buttonCenterX = buttonGlobalPos.x() + (width() / 2);
    int dropdownX = buttonCenterX - (m_dropdownWidget->width() / 2);
    int dropdownY = buttonGlobalPos.y() + height() + 5;

    QPoint position(dropdownX, dropdownY);

    // Boundary checking
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();

        if (position.x() + m_dropdownWidget->width() > screenGeometry.right()) {
            position.setX(screenGeometry.right() - m_dropdownWidget->width());
        }
        if (position.x() < screenGeometry.left()) {
            position.setX(screenGeometry.left());
        }
        if (position.y() + m_dropdownWidget->height() > screenGeometry.bottom()) {
            position.setY(screenGeometry.bottom() - m_dropdownWidget->height());
        }
    }

    m_dropdownWidget->move(position);
}

void ModernLanguageDropdown::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // REMOVED HOVER EFFECT - Always use the same colors
    QColor backgroundColor = QColor(255, 255, 255, 255);  // Always white
    QColor borderColor = QColor(230, 230, 230, 180);     // Always light gray

    // Draw background
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(rect(), 12, 12);
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(backgroundPath);

    // Draw border
    QPainterPath borderPath;
    QRectF borderRect = rect().adjusted(0.75, 0.75, -0.75, -0.75);
    borderPath.addRoundedRect(borderRect, 11.25, 11.25);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(borderColor, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(borderPath);

    // Draw language name
    painter.setPen(QPen(QColor(26, 26, 26), 1));
    QFont textFont("Segoe UI", 14, QFont::Medium);
    painter.setFont(textFont);
    painter.drawText(QRect(55, 0, width() - 85, height()), Qt::AlignVCenter, m_currentLanguage);

    QPushButton::paintEvent(event);
}

void ModernLanguageDropdown::showDropdown()
{
    if (m_dropdownVisible) {
        m_dropdownWidget->hide();
        m_dropdownVisible = false;
        m_animatedArrow->animateToDown();
    } else {
        positionDropdownBelowButton();
        updateCheckmarks(); // Update checkmarks when showing dropdown
        m_dropdownWidget->show();
        m_dropdownWidget->raise();
        m_dropdownVisible = true;
        m_animatedArrow->animateToUp();
    }
}

void ModernLanguageDropdown::onLanguageSelected(const QString &language, const QString &code)
{
    for (const auto &lang : m_languages) {
        if (lang.code == code && lang.name == language) {
            m_currentLanguage = lang.name;
            m_currentLanguageCode = code;
            m_currentFlagUrl = "https://hatscripts.github.io/circle-flags/flags/" + lang.countryCode + ".svg";
            m_currentFlag->setFlag(m_currentFlagUrl);
            break;
        }
    }

    updateCheckmarks();

    m_dropdownWidget->hide();
    m_dropdownVisible = false;
    m_animatedArrow->animateToDown();

    emit languageChanged(code);
}

void ModernLanguageDropdown::resizeEvent(QResizeEvent *event)
{
    if (m_animatedArrow) {
        m_animatedArrow->move(width() - 32, (height() - 24) / 2);
    }

    // Update flag position when resizing
    if (m_currentFlag) {
        m_currentFlag->move(16, (height() - Config::FLAG_SIZE) / 2);
    }

    QPushButton::resizeEvent(event);
}

void ModernLanguageDropdown::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        showDropdown();
    }
    QPushButton::mousePressEvent(event);
}

void ModernLanguageDropdown::enterEvent(QEvent *event)
{
    m_isHovered = true;
    update();
    QPushButton::enterEvent(event);
}

void ModernLanguageDropdown::leaveEvent(QEvent *event)
{
    m_isHovered = false;
    update();
    QPushButton::leaveEvent(event);
}

// WelcomeCard - Optimized with properly sized panda
WelcomeCard::WelcomeCard(QWidget *parent)
    : QFrame(parent)
    , m_darkMode(false)
{
    setFixedSize(Config::CARD_WIDTH, Config::CARD_HEIGHT);
    setFrameStyle(QFrame::NoFrame);

    setupUI();
}

void WelcomeCard::setupUI()
{
    setupWindowControls();

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(85, 75, 75, 75);
    mainLayout->setSpacing(60);

    // Left side - illustration with PROPERLY SIZED PANDA
    m_illustrationContainer.reset(new QWidget(this));
    m_illustrationContainer->setFixedSize(400, 500); // Increased size for better fit
    m_illustrationContainer->setStyleSheet("background: transparent; border: none;");

    // Create a proper container for the panda SVG
    auto* pandaContainer = new QWidget(m_illustrationContainer.get());
    pandaContainer->setFixedSize(380, 480); // Slightly smaller than container
    pandaContainer->move(10, 10); // Center in container
    pandaContainer->setStyleSheet("background: transparent; border: none;");

    // Use your actual panda.svg file with proper aspect ratio
    m_pandaSvg.reset(new CrispSvgWidget("panda.svg", pandaContainer));
    m_pandaSvg->setStyleSheet("background: transparent; border: none;");

    // Set a reasonable size that maintains aspect ratio
    m_pandaSvg->setFixedSize(380, 480);
    m_pandaSvg->move(0, 0);

    // Right side - content
    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);
    contentLayout->setAlignment(Qt::AlignVCenter);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Title
    m_titleLabel.reset(new QLabel("Welcome to\nPandaBlur", this));
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "    color: #000000;"
        "    font-size: 42px;"
        "    font-weight: 900;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "    line-height: 1.1;"
        "}"
        );
    m_titleLabel->setAlignment(Qt::AlignLeft);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setFixedWidth(400);

    // Subtitle
    m_subtitleLabel.reset(new QLabel("PandaBlur is a Security Software\nto protect your devices!", this));
    m_subtitleLabel->setStyleSheet(
        "QLabel {"
        "    color: #5a6c7d;"
        "    font-size: 22px;"
        "    font-weight: normal;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "    line-height: 1.4;"
        "    margin-top: 5px;"
        "}"
        );
    m_subtitleLabel->setAlignment(Qt::AlignLeft);
    m_subtitleLabel->setWordWrap(true);
    m_subtitleLabel->setFixedWidth(400);

    // Continue button
    m_continueButton.reset(new SimpleButton("Continue", this));

    // Language dropdown
    m_languageDropdown.reset(new ModernLanguageDropdown(this));

    // Auto-translate label
    m_autoTranslateLabel.reset(new QLabel("Detects and translates language automatically", this));
    m_autoTranslateLabel->setStyleSheet(
        "QLabel {"
        "    color: #888888;"
        "    font-size: 13px;"
        "    font-weight: normal;"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "    margin-top: 3px;"
        "}"
        );
    m_autoTranslateLabel->setAlignment(Qt::AlignLeft);
    m_autoTranslateLabel->setFixedWidth(400);

    // Connect language change
    connect(m_languageDropdown.get(), &ModernLanguageDropdown::languageChanged,
            this, &WelcomeCard::onLanguageChanged);

    // Layout
    contentLayout->addWidget(m_titleLabel.get());
    contentLayout->addWidget(m_subtitleLabel.get());
    contentLayout->addSpacing(10);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(m_continueButton.get());
    buttonLayout->addStretch();
    contentLayout->addLayout(buttonLayout);

    contentLayout->addSpacing(10);

    auto* languageLayout = new QHBoxLayout();
    languageLayout->setContentsMargins(0, 0, 0, 0);
    languageLayout->addWidget(m_languageDropdown.get());
    languageLayout->addStretch();
    contentLayout->addLayout(languageLayout);

    auto* autoTranslateLayout = new QHBoxLayout();
    autoTranslateLayout->setContentsMargins(0, 0, 0, 0);
    autoTranslateLayout->addWidget(m_autoTranslateLabel.get());
    autoTranslateLayout->addStretch();
    contentLayout->addLayout(autoTranslateLayout);

    contentLayout->addStretch(1);

    mainLayout->addWidget(m_illustrationContainer.get(), 0, Qt::AlignCenter);
    mainLayout->addWidget(contentWidget, 1);

    // Connect to main window
    auto* mainWindow = qobject_cast<MainWindow*>(parent());
    if (mainWindow) {
        connect(m_continueButton.get(), &QPushButton::clicked,
                mainWindow, &MainWindow::onContinueClicked);
    }
}

void WelcomeCard::setupWindowControls()
{
    m_minimizeButton.reset(new WindowControlButton("minimize.svg", this));
    m_closeButton.reset(new WindowControlButton("close.svg", this));

    constexpr int buttonY = 20;
    constexpr int buttonSpacing = Config::BUTTON_SPACING;

    m_closeButton->move(Config::CARD_WIDTH - 20 - 32, buttonY);
    m_minimizeButton->move(Config::CARD_WIDTH - 20 - 32 - buttonSpacing - 32, buttonY);

    auto* mainWindow = qobject_cast<MainWindow*>(parent());
    if (mainWindow) {
        connect(m_minimizeButton.get(), &QPushButton::clicked,
                mainWindow, &MainWindow::onMinimizeClicked);
        connect(m_closeButton.get(), &QPushButton::clicked,
                mainWindow, &MainWindow::onCloseClicked);
    }

    m_minimizeButton->raise();
    m_closeButton->raise();
}

void WelcomeCard::updateLanguage(const QString &languageCode)
{
    ResourceManager& rm = ResourceManager::instance();

    QString title = rm.getTranslation("title", languageCode);
    QString subtitle = rm.getTranslation("subtitle", languageCode);
    QString continueText = rm.getTranslation("continue", languageCode);
    QString autoTranslate = rm.getTranslation("autoTranslate", languageCode);

    m_titleLabel->setText(title);
    m_subtitleLabel->setText(subtitle);
    m_continueButton->updateText(continueText);
    m_autoTranslateLabel->setText(autoTranslate);
}

void WelcomeCard::setDarkMode(bool enabled)
{
    m_darkMode = enabled;
    setProperty("darkMode", enabled);

    if (enabled) {
        setStyleSheet("QFrame { background-color: #2b2b2b; }");
        m_titleLabel->setStyleSheet(
            "QLabel { color: #ffffff; font-size: 42px; font-weight: 900; "
            "font-family: 'Segoe UI', Arial, sans-serif; line-height: 1.1; }"
            );
        m_subtitleLabel->setStyleSheet(
            "QLabel { color: #cccccc; font-size: 22px; font-weight: normal; "
            "font-family: 'Segoe UI', Arial, sans-serif; line-height: 1.4; margin-top: 5px; }"
            );
        m_autoTranslateLabel->setStyleSheet(
            "QLabel { color: #999999; font-size: 13px; font-weight: normal; "
            "font-family: 'Segoe UI', Arial, sans-serif; margin-top: 3px; }"
            );
    } else {
        setStyleSheet("");
        setupUI();
    }
}

void WelcomeCard::onLanguageChanged(const QString &languageCode)
{
    updateLanguage(languageCode);
}

void WelcomeCard::resizeEvent(QResizeEvent *event)
{
    adjustLayout();
    QFrame::resizeEvent(event);
}

void WelcomeCard::adjustLayout()
{
    // Responsive layout adjustments
}

void WelcomeCard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addRoundedRect(rect(), Config::CARD_RADIUS, Config::CARD_RADIUS);

    QColor backgroundColor = m_darkMode ? QColor(43, 43, 43, 255) : QColor(255, 255, 255, 255);
    QColor borderColor = m_darkMode ? QColor(85, 85, 85, 255) : QColor(224, 224, 224, 255);

    painter.setBrush(QBrush(backgroundColor));
    painter.setPen(QPen(borderColor, 1));
    painter.drawPath(path);

    QFrame::paintEvent(event);
}

// MainWindow - Optimized with better window management
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isDragging(false)
{
    setupUI();
    centerWindow();
}

void MainWindow::setupUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setStyleSheet("background: transparent;");

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(50, 60, 50, 60);
    mainLayout->setAlignment(Qt::AlignCenter);

    m_welcomeCard.reset(new WelcomeCard(this));

    auto* cardShadow = new QGraphicsDropShadowEffect();
    cardShadow->setBlurRadius(50);
    cardShadow->setColor(QColor(0, 0, 0, 60));
    cardShadow->setOffset(0, 20);
    m_welcomeCard->setGraphicsEffect(cardShadow);

    mainLayout->addWidget(m_welcomeCard.get(), 0, Qt::AlignCenter);
}

void MainWindow::centerWindow()
{
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}

void MainWindow::onMinimizeClicked()
{
    setWindowState(Qt::WindowMinimized);
}

void MainWindow::onCloseClicked()
{
    close();
}

void MainWindow::onContinueClicked()
{
    QMessageBox::information(this, "PandaBlur",
                             "Welcome to PandaBlur Security Software!\n\n"
                             "Click OK to continue to the main application.");
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        QPoint newPos = event->globalPos() - m_dragPosition;

        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            newPos.setX(std::clamp(newPos.x(), -width()/2, screenGeometry.width() - width()/2));
            newPos.setY(std::clamp(newPos.y(), 0, screenGeometry.height() - height()/2));
        }

        move(newPos);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        event->accept();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    centerWindow();
    QMainWindow::resizeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 0));
    QMainWindow::paintEvent(event);
}
