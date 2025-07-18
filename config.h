#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

namespace Config {
    // UI Constants
    constexpr int WINDOW_WIDTH = 1100;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr int CARD_WIDTH = 1000;
    constexpr int CARD_HEIGHT = 600;
    constexpr int DROPDOWN_WIDTH = 280;
    constexpr int DROPDOWN_MAX_HEIGHT = 220;
    constexpr int DROPDOWN_ITEM_HEIGHT = 50;
    constexpr int FLAG_SIZE = 28;
    constexpr int BUTTON_SPACING = 10;
    constexpr int CARD_RADIUS = 30;
    constexpr int DROPDOWN_RADIUS = 16;
    
    // Network Constants
    constexpr int NETWORK_TIMEOUT_MS = 5000;
    constexpr int GEOLOCATION_DELAY_MS = 500;
    
    // Rendering Constants
    constexpr int MAX_RENDER_SCALE = 4;
    constexpr int MIN_RENDER_SCALE = 1;
    
    // Default Language
    const QString DEFAULT_LANGUAGE = "EN";
    const QString DEFAULT_COUNTRY = "gb";
    
    // Resource Paths
    const QString STYLES_QRC = ":/styles/";
    const QString FLAGS_QRC = ":/flags/";
    const QString TRANSLATIONS_QRC = ":/translations/";
}

#endif // CONFIG_H