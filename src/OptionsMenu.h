#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include <SFML/Graphics.hpp>

#include <windows.h>
#include <sstream>

#define KEY_DOWN(vkey) (GetKeyState(vkey) & 0x8000)

#define RM_WIDTH 320
#define RM_HEIGHT 240


// Grab a bunch of shit from main.h
extern std::map<std::string, std::string> translationMap;
extern int confWindowScale;
extern bool confUseDirectDraw;
extern bool confFullscreen;
extern bool confEnableVSync;
extern bool confUseDesktopResolution;
extern bool confAllowUnevenPixelSizes;
extern bool confUseInterpolation;
extern bool confIgnoreAspectRatio;
extern bool confUseShaders;
extern int confShaderId;

extern bool reverseDir;

extern sf::RenderWindow *m_win;
extern sf::Texture *m_wintexture;
extern sf::Sprite *m_winsprite;

extern std::string systemGraphic;
extern std::string shaderName;

//extern bool isWindowsVistaOrHigher();

extern void setDirectDraw(bool useDirectDraw);
extern void setFullscreen(bool fullscreen);
extern void toggleWindowScale();
//extern void setWindowScale(int scale);

extern void setUseDesktopResolution(bool useDesktop);
extern void setIgnoreAspectRatio(bool ignoreAspect);
extern void setAllowUnevenPixels(bool allowUneven);
extern void setInterpolation(bool interp);
extern void toggleShaders();

class OptionsMenu {
    struct Option {
        sf::Text label;
        sf::Text pick;
        bool disabled;

        Option() {
            disabled = false;
        }
    };

    enum OptionsMenuConfigs {
        OCFG_WINDOWED,
        OCFG_FULL_MIN,
        OCFG_FULL_MAX,
        NUM_CONFIGS
    };

    typedef unsigned char OptionsMenuConfigs_T;

    enum OptionsMenuActions {
        OMEN_TITLE,
        OMEN_USE_DIRECT_DRAW,
        OMEN_DISPLAY_MODE,
        OMEN_WIN_RES,
        OMEN_USE_DESKTOP_RES,
        OMEN_ALLOW_UNEVEN_PIX,
        OMEN_IGNORE_ASPECT,
        OMEN_USE_INTERPOLATION,
        OMEN_USE_SHADERS,
        NUM_TEXT_LAYERS
    };
    typedef unsigned char OptionsMenuActions_T;


    public:
		OptionsMenu();

		sf::RenderTexture rendTex;
	    sf::Sprite spr;

	    int action;

	    OptionsMenuConfigs_T config;

	    bool visible;
	    bool exitScene;
	    bool keyDown;

		void init();
		void update(bool withInput = true);
		void draw();
		void resetSprite();
		void updateMenu();

	private:
	    sf::RectangleShape _rect;
	    sf::RectangleShape _selector;
	    sf::Image _systemGraphicImage;
	    sf::Font _font;
	    sf::Color _color;
	    sf::Color _shadow;

	    Option _options[NUM_TEXT_LAYERS];

        std::vector<OptionsMenuActions_T> _menus[NUM_CONFIGS];

	    std::string _minFullscreenText;
	    std::stringstream _optionsString;

	    int _textLayers;
	    int _visibleTimer;
	    int _numOptions;
	    int _fontSize;
	    int _textItr;
	    unsigned char _alpha;

	    void _buildMenuOption(OptionsMenuActions_T &option);
	    void _getAction();
	    void _drawShadowed(sf::Text &text);
};

#endif // OPTIONS_MENU_H
