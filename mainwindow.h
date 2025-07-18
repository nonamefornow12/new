#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSvgRenderer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <memory>

// Configuration constants
namespace Config {
constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;
constexpr int CARD_WIDTH = 1100;
constexpr int CARD_HEIGHT = 700;
constexpr int CARD_RADIUS = 24;
constexpr int FLAG_SIZE = 24;
constexpr int DROPDOWN_WIDTH = 220;
constexpr int DROPDOWN_ITEM_HEIGHT = 54;
constexpr int DROPDOWN_MAX_HEIGHT = 300;
constexpr int BUTTON_SPACING = 10;
constexpr int NETWORK_TIMEOUT_MS = 5000;
constexpr int GEOLOCATION_DELAY_MS = 1000;
constexpr int MIN_RENDER_SCALE = 1;
constexpr int MAX_RENDER_SCALE = 4;
const QString DEFAULT_LANGUAGE = "EN";
}

// Forward declarations
class CrispSvgWidget;
class SimpleButton;
class WindowControlButton;
class AnimatedArrowWidget;
class CrispCircleFlagWidget;
class ModernLanguageDropdown;
class WelcomeCard;
class ResourceManager;
class GeolocationService;

// ClickableLabel class for footer links
class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void clicked();

private:
    bool m_isHovered;
};

// CrispSvgWidget for optimized SVG rendering
class CrispSvgWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CrispSvgWidget(const QString &file, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
};

// SimpleButton with custom styling
class SimpleButton : public QPushButton
{
    Q_OBJECT

public:
    explicit SimpleButton(const QString &text, QWidget *parent = nullptr);
    void updateText(const QString &text);
};

// WindowControlButton for minimize/close buttons
class WindowControlButton : public QPushButton
{
    Q_OBJECT

public:
    explicit WindowControlButton(const QString &svgPath, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_filePath;
    bool m_isHovered;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
};

// AnimatedArrowWidget for dropdown arrow
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

// CrispCircleFlagWidget for flag rendering with caching
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

    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<QTimer> m_timeoutTimer;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    QNetworkReply* m_currentReply;
    QString m_currentFlagUrl;
    QPixmap m_cachedPixmap;
    bool m_isLoading;
    bool m_pixmapCached;

    static QHash<QString, QPixmap> s_flagCache;
};

// ResourceManager singleton
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

// GeolocationService for automatic location detection
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
    QNetworkReply* m_currentReply;
};

// Language option structure
struct LanguageOption {
    QString name;
    QString code;
    QString countryCode;
};

// ModernLanguageDropdown with optimized dropdown
class ModernLanguageDropdown : public QPushButton
{
    Q_OBJECT

public:
    explicit ModernLanguageDropdown(QWidget *parent = nullptr);
    ~ModernLanguageDropdown();

    void setLanguageByCode(const QString &languageCode);

signals:
    void languageChanged(const QString &languageCode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void showDropdown();
    void onLanguageSelected(const QString &language, const QString &code);
    void onLocationDetected(const QString &countryCode, const QString &languageCode);
    void onLocationFailed();

private:
    void setupLanguageOptions();
    void createModernDropdown();
    void createDropdownItems();
    void updateCheckmarks();
    void positionDropdownBelowButton();
    int calculateDropdownHeight() const;

    // Member variables in initialization order
    bool m_isHovered;
    bool m_dropdownVisible;
    QString m_currentLanguageCode;
    QString m_currentLanguage;
    QString m_currentFlagUrl;

    std::vector<LanguageOption> m_languages;
    std::unique_ptr<CrispCircleFlagWidget> m_currentFlag;
    std::unique_ptr<AnimatedArrowWidget> m_animatedArrow;
    std::unique_ptr<QWidget> m_dropdownWidget;
    std::unique_ptr<QListWidget> m_languageList;
    std::unique_ptr<GeolocationService> m_geolocationService;
};

// WelcomeCard main content area
class WelcomeCard : public QFrame
{
    Q_OBJECT

public:
    explicit WelcomeCard(QWidget *parent = nullptr);
    void updateLanguage(const QString &languageCode);
    void setDarkMode(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLanguageChanged(const QString &languageCode);
    void onLearnMoreClicked();
    void onPrivacyPolicyClicked();

private:
    void setupUI();
    void setupWindowControls();
    void setupFooter(QVBoxLayout* parentLayout);
    void adjustLayout();

    // Member variables
    bool m_darkMode;
    QString m_currentLanguageCode;

    std::unique_ptr<QWidget> m_illustrationContainer;
    std::unique_ptr<CrispSvgWidget> m_pandaSvg;
    std::unique_ptr<QLabel> m_titleLabel;
    std::unique_ptr<QLabel> m_subtitleLabel;
    std::unique_ptr<SimpleButton> m_continueButton;
    std::unique_ptr<ModernLanguageDropdown> m_languageDropdown;
    std::unique_ptr<QLabel> m_autoTranslateLabel;
    std::unique_ptr<ClickableLabel> m_learnMoreLabel;
    std::unique_ptr<ClickableLabel> m_privacyPolicyLabel;
    std::unique_ptr<QLabel> m_copyrightLabel;
    std::unique_ptr<WindowControlButton> m_minimizeButton;
    std::unique_ptr<WindowControlButton> m_closeButton;
};

// MainWindow class
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

    bool m_isDragging;
    QPoint m_dragPosition;
    std::unique_ptr<WelcomeCard> m_welcomeCard;
};

#endif // MAINWINDOW_H
