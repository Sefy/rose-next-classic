#pragma once
class ColorUtil {

public:
    enum {
        RED = D3DCOLOR_ARGB(255, 0, 0, 255),
        GREEN = D3DCOLOR_ARGB(255, 0, 255, 0),
        BLUE = D3DCOLOR_ARGB(255, 0, 0, 255),
        BLACK = D3DCOLOR_ARGB(255, 0, 0, 0),
        WHITE = D3DCOLOR_ARGB(255, 255, 255, 255),
        YELLOW = D3DCOLOR_ARGB(255, 255, 255, 0),
        GRAY = D3DCOLOR_ARGB(255, 150, 150, 150),
        VIOLET = D3DCOLOR_ARGB(255, 255, 0, 255),
        ORANGE = D3DCOLOR_ARGB(255, 255, 128, 0),
        PINK = D3DCOLOR_ARGB(255, 255, 136, 200)
    };

    static D3DCOLOR getColorByNamePrefix(std::string name) {
        if (name.rfind("[GM]", 0) == 0) {
            return BLUE;
        }
        if (name.rfind("[DEV]", 0) == 0) {
            return PINK;
        }
        if (name.rfind("[EVENT]", 0) == 0) {
            return GREEN;
        }

        return WHITE;
    }
};
