#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QSvgRenderer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QPixmap>
#include <memory>
#include "config.h"

QT_BEGIN_NAMESPACE
class QSvgRenderer;
class QPropertyAnimation;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
QT_END_NAMESPACE

// Forward declarations
class CrispSvgWidget;
class SimpleButton;
class WindowControlButton;
class AnimatedArrowWidget;
class CrispCircleFlagWidget;
class ModernLanguageDropdown;
class GeolocationService;
class ResourceManager;
class WelcomeCard;

// CrispSvgWidget - High-quality SVG rendering widget
class CrispSvgWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CrispSvgWidget(const QString &file = QString(), QWidget *parent = nullptr);

    QSvgRenderer* renderer() const { return m_svgRenderer.get(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
};

// SimpleButton - Styled button with hover effects
class SimpleButton : public QPushButton
{
    Q_OBJECT

public:
    explicit SimpleButton(const QString &text, QWidget *parent = nullptr);
    void updateText(const QString &text);

private:
         // No additional members needed for now
};

// WindowControlButton - Custom minimize/close buttons
class WindowControlButton : public QPushButton
{
    Q_OBJECT

public:
    explicit WindowControlButton(const QString &svgPath, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_filePath;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    bool m_isHovered;
};

// AnimatedArrowWidget - Rotating arrow for dropdown
class AnimatedArrowWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

public:
    explicit AnimatedArrowWidget(QWidget *parent = nullptr);

    qreal rotation() const { return m_rotation; }
    void setRotation(qreal rotation);

    void animateToUp();
    void animateToDown();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_rotation;
    std::unique_ptr<QSvgRenderer> m_arrowRenderer;
    std::unique_ptr<QPropertyAnimation> m_rotationAnimation;
};

// CrispCircleFlagWidget - High-quality flag rendering with caching
class CrispCircleFlagWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CrispCircleFlagWidget(const QString &flagUrl, QWidget *parent = nullptr);

    void setFlag(const QString &flagUrl);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onFlagDownloaded();
    void onNetworkTimeout();

private:
    void renderFlag();
    int calculateOptimalScale() const;

    QString m_currentFlagUrl;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<QTimer> m_timeoutTimer;
    QNetworkReply *m_currentReply;

    QPixmap m_cachedPixmap;
    bool m_isLoading;
    bool m_pixmapCached;

    static QHash<QString, QPixmap> s_flagCache;
};

// GeolocationService - IP-based location detection
class GeolocationService : public QObject
{
    Q_OBJECT

public:
    explicit GeolocationService(QObject *parent = nullptr);

    void detectUserLocation();

signals:
    void locationDetected(const QString &countryCode, const QString &languageCode);
    void locationFailed();

private slots:
    void onLocationDataReceived();
    void onNetworkTimeout();

private:
    QString mapCountryToLanguage(const QString &countryCode);

    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<QTimer> m_timeoutTimer;
    QNetworkReply *m_currentReply;
};

// ResourceManager - Singleton for managing resources and translations
class ResourceManager : public QObject
{
    Q_OBJECT

public:
    static ResourceManager& instance();

    QString getTranslation(const QString &key, const QString &language);
    QPixmap getFlagPixmap(const QString &countryCode);
    QString getStyleSheet(const QString &name);

private:
    explicit ResourceManager(QObject *parent = nullptr);
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};

// ModernLanguageDropdown - Advanced language selector with flags and animations
class ModernLanguageDropdown : public QPushButton
{
    Q_OBJECT

public:
    explicit ModernLanguageDropdown(QWidget *parent = nullptr);
    ~ModernLanguageDropdown();

    void setLanguageByCode(const QString &languageCode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void showDropdown();
    void onLocationDetected(const QString &countryCode, const QString &languageCode);
    void onLocationFailed();
    void onLanguageSelected(const QString &language, const QString &code);

signals:
    void languageChanged(const QString &languageCode);

private:
    struct LanguageOption {
        QString name;
        QString code;
        QString countryCode;
    };

    void setupLanguageOptions();
    void createModernDropdown();
    void createDropdownItems();
    void positionDropdownBelowButton();
    int calculateDropdownHeight() const;
    void updateCheckmarks();

    QList<LanguageOption> m_languages;
    QString m_currentLanguage;
    QString m_currentFlagUrl;
    bool m_isHovered;
    bool m_dropdownVisible;
    QString m_currentLanguageCode;  // MOVED HERE - AFTER m_dropdownVisible

    std::unique_ptr<CrispCircleFlagWidget> m_currentFlag;
    std::unique_ptr<AnimatedArrowWidget> m_animatedArrow;
    std::unique_ptr<QWidget> m_dropdownWidget;
    std::unique_ptr<QListWidget> m_languageList;
    std::unique_ptr<GeolocationService> m_geolocationService;
};

// WelcomeCard - Main welcome interface card
class WelcomeCard : public QFrame
{
    Q_OBJECT

public:
    explicit WelcomeCard(QWidget *parent = nullptr);

    void setDarkMode(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLanguageChanged(const QString &languageCode);

private:
    void setupUI();
    void setupWindowControls();
    void updateLanguage(const QString &languageCode);
    void adjustLayout();

    bool m_darkMode;

    // Window controls
    std::unique_ptr<WindowControlButton> m_minimizeButton;
    std::unique_ptr<WindowControlButton> m_closeButton;

    // Main content
    std::unique_ptr<QWidget> m_illustrationContainer;
    std::unique_ptr<CrispSvgWidget> m_pandaSvg;
    std::unique_ptr<QLabel> m_titleLabel;
    std::unique_ptr<QLabel> m_subtitleLabel;
    std::unique_ptr<SimpleButton> m_continueButton;
    std::unique_ptr<ModernLanguageDropdown> m_languageDropdown;
    std::unique_ptr<QLabel> m_autoTranslateLabel;
};

// MainWindow - Main application window
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onMinimizeClicked();
    void onCloseClicked();
    void onContinueClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void centerWindow();

    std::unique_ptr<WelcomeCard> m_welcomeCard;

    // Window dragging
    bool m_isDragging;
    QPoint m_dragPosition;
};

#endif // MAINWINDOW_H
