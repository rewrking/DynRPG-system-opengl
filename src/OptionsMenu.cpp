#include "OptionsMenu.h"

// The OptionsMenu class controls the F5 menu (Video Options)
// From this point on, you're looking at almost purely SFML code as opposed to DynRPG
OptionsMenu::OptionsMenu() {
    visible = false;
    exitScene = false;
    _visibleTimer = 0;
    _alpha = 0;

    // The menu lives as long as the plugin lives, so most of these static values can be set via the constructor

    // The render texture is an image buffer where all drawing of the menu happens
	rendTex.create(RM_WIDTH, RM_HEIGHT);
    spr.setTexture(rendTex.getTexture());
    // Clear & display to initialize
    rendTex.clear(sf::Color::Transparent);
    rendTex.display();

    _font.loadFromFile("DynPlugins\\miranda_nbp.ttf");
    _fontSize = 8;

    int lineHeight = _fontSize*2;



    _rect.setSize(sf::Vector2f(240, 4+(lineHeight*(NUM_TEXT_LAYERS-1))));
    _rect.setPosition((rendTex.getSize().x/2) - (_rect.getSize().x/2), (rendTex.getSize().y/2) - (_rect.getSize().y/2));
    _rect.setOutlineColor(sf::Color(0,0,0,255));
    _rect.setOutlineThickness(1.0);
    _rect.setFillColor(sf::Color(25,25,25,225));

    _selector.setSize(sf::Vector2f(_rect.getSize().x, lineHeight));
    _selector.setPosition(_rect.getPosition());
    _selector.setFillColor(sf::Color(50, 75, 100, 255));

    _textLayers = NUM_TEXT_LAYERS;
    for (int i=0; i<_textLayers; i++) {
        _options[i].label.setFont(_font);
        _options[i].label.setCharacterSize(_fontSize*2);
        _options[i].pick.setFont(_font);
        _options[i].pick.setCharacterSize(_fontSize*2);
    }
    _options[0].label.setPosition(_rect.getPosition().x + 4, _rect.getPosition().y - 2);
    _options[0].label.setFillColor(sf::Color(150, 175, 200, 255));

    for (int i=1; i<_textLayers; i++) {
        _options[i].label.setPosition(_options[0].label.getPosition().x, _options[0].label.getPosition().y + (lineHeight*i));
        _options[i].pick.setPosition(_options[0].label.getPosition().x + 4 + static_cast<int>(_rect.getSize().x*0.62), _options[0].label.getPosition().y + (lineHeight*i));
    }

    _options[OMEN_USE_DIRECT_DRAW].disabled = true;


    config = OCFG_WINDOWED;

    _menus[OCFG_WINDOWED] = {
        OMEN_USE_DIRECT_DRAW,
        OMEN_DISPLAY_MODE,
        OMEN_WIN_RES,
        OMEN_USE_INTERPOLATION,
        OMEN_USE_SHADERS
	};

	_menus[OCFG_FULL_MIN] = {
	    OMEN_USE_DIRECT_DRAW,
        OMEN_DISPLAY_MODE,
        OMEN_USE_DESKTOP_RES,
        OMEN_USE_INTERPOLATION,
        OMEN_USE_SHADERS
	};

	_menus[OCFG_FULL_MAX] = {
	    OMEN_USE_DIRECT_DRAW,
        OMEN_DISPLAY_MODE,
        OMEN_USE_DESKTOP_RES,
        OMEN_ALLOW_UNEVEN_PIX,
        OMEN_IGNORE_ASPECT,
        OMEN_USE_INTERPOLATION,
        OMEN_USE_SHADERS
	};

	_numOptions = _menus[config].size()-1;
}

void OptionsMenu::init()
{
    // Initialize the main variables
    visible = true;
    exitScene = false;
    action = 0;
    _visibleTimer = 0;
    _alpha = 0;

    // Update the action if one of the options is disabled
    while (_options[action+1].disabled) {
        action++;
    }

    // Prevent any undesired keychecking to occur
    keyDown = true;

    //_minFullscreenText = isWindowsVistaOrHigher() ? "640 x 480" : "320 x 240";
    _minFullscreenText = "640 x 480";
    updateMenu();
}

void OptionsMenu::update(bool withInput)
{
    resetSprite();

    // Fade in
    if (_visibleTimer < 17) {
        _visibleTimer++;
        _alpha = (15*_visibleTimer);
        spr.setColor(sf::Color(255, 255, 255, _alpha));
    }

    // If the fade cycle has completed
    if (_alpha == 255) {

        _numOptions = _menus[config].size()-1;

        // Update Input
        // The only non-SFML piece in this. GetKeyState has to be used instead of SFML's event handling unfortunately...
        if (withInput) {
            if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_F5)) {
                if (!keyDown) {
                    //visible = false;
                    exitScene = true;
                    keyDown = true;
                }
            } else if (KEY_DOWN(VK_LEFT)) {
                if (!keyDown) {
                    reverseDir = true;
                    _getAction();
                    keyDown = true;
                }
            } else if (KEY_DOWN(VK_RIGHT)) {
                if (!keyDown) {
                    reverseDir = false;
                    _getAction();
                    keyDown = true;
                }
            } else if (KEY_DOWN(VK_UP)) {
                if (!keyDown) {
                    while (_options[action+1].disabled || !keyDown) {
                        if (action > 0) action--;
                        else action = _numOptions;
                        keyDown = true;
                    }
                }
            } else if (KEY_DOWN(VK_DOWN)) {
                if (!keyDown) {
                    while (_options[action+1].disabled || !keyDown) {
                        if (action < _numOptions) action++;
                        else action = 0;
                        keyDown = true;
                    }
                }
            } else if (keyDown) keyDown = false;
        }

        // Update the menus if a key has been pressed
        if (keyDown) {
            if (action > _numOptions) action = _numOptions;
            updateMenu();
        }

    }
}

void OptionsMenu::draw()
{
    rendTex.clear(sf::Color::Transparent);
    rendTex.draw(_rect);

    if (_alpha == 255) {
        _selector.setPosition(_rect.getPosition().x, (_rect.getPosition().y + ((action+1)*_selector.getSize().y) + 2));
        rendTex.draw(_selector);
        for (int i=0; i<_numOptions+2; i++) {
            _drawShadowed(_options[i].label);
            _drawShadowed(_options[i].pick);
        }
    }

    rendTex.display();

    m_win->draw(spr);
}

void OptionsMenu::resetSprite() {
    spr.setScale(m_winsprite->getScale());
    spr.setOrigin(m_winsprite->getOrigin());
    spr.setPosition(m_winsprite->getPosition());
}

void OptionsMenu::updateMenu()
{
    if (confFullscreen) {
        if (confUseDesktopResolution) {
            config = OCFG_FULL_MAX;
        } else {
            config = OCFG_FULL_MIN;
        }
    } else {
        config = OCFG_WINDOWED;
    }

    // Update the main label
    _options[0].label.setString(translationMap["VideoOptions"]);

    _textItr = 1;
    for (auto it : _menus[config]) {
        _buildMenuOption(it);
    }
}

void OptionsMenu::_buildMenuOption(OptionsMenuActions_T &option) {
    switch (option) {
        case OMEN_USE_DIRECT_DRAW: {
            _options[_textItr].label.setString(translationMap["RenderMode"] + ":");
            _options[_textItr].pick.setString(confUseDirectDraw ? translationMap["DirectDraw"] : translationMap["OpenGL"]);
        } break;

        case OMEN_DISPLAY_MODE: {
            _options[_textItr].label.setString(translationMap["DisplayMode"] + ":");
            _options[_textItr].pick.setString(confFullscreen ? translationMap["Fullscreen"] : translationMap["Windowed"]);
        } break;

        case OMEN_WIN_RES: {
            _options[_textItr].label.setString(translationMap["WindowResolution"] + ":");
            _optionsString.str(std::string());
            _optionsString << 320*confWindowScale << " x " << 240*confWindowScale;
            _options[_textItr].pick.setString(_optionsString.str());
        } break;

        case OMEN_USE_DESKTOP_RES: {
            _options[_textItr].label.setString(translationMap["UseDesktopResolution"] + ":");
            _optionsString.str(std::string());
            if (confUseDesktopResolution) {
                _optionsString << translationMap["Yes"] << " (" << sf::VideoMode::getDesktopMode().width << " x " << sf::VideoMode::getDesktopMode().height << ")";
            } else {
                _optionsString << translationMap["No"] << " (" << _minFullscreenText << ")";
            }
            _options[_textItr].pick.setString(_optionsString.str());
        } break;

        case OMEN_ALLOW_UNEVEN_PIX: {
            _options[_textItr].label.setString(translationMap["AllowUnevenPixels"] + ":");
            _options[_textItr].pick.setString(confAllowUnevenPixelSizes ? translationMap["Yes"] : translationMap["No"]);

        } break;

        case OMEN_IGNORE_ASPECT: {
            _options[_textItr].label.setString(translationMap["IgnoreAspectRatio"] + ":");
            _options[_textItr].pick.setString(confIgnoreAspectRatio ? translationMap["Yes"] : translationMap["No"]);
        } break;

        case OMEN_USE_INTERPOLATION: {
            _options[_textItr].label.setString(translationMap["Interpolation"] + ":");
            _options[_textItr].pick.setString(confUseInterpolation ? translationMap["Yes"] : translationMap["No"]);
        } break;

        case OMEN_USE_SHADERS: {
            std::string shaderNameAdjusted = shaderName;
            if (shaderName.size() >= 17) {
                shaderNameAdjusted = shaderNameAdjusted.substr(0, 17) + "...";
            }
            _options[_textItr].label.setString(translationMap["Shader"] + ":");
            _options[_textItr].pick.setString(confUseShaders ? shaderNameAdjusted : translationMap["None"]);
        } break;

        default: {} break;
    }

    if (_options[_textItr].disabled) {
        _options[_textItr].label.setFillColor(sf::Color(100,100,100,255));
        _options[_textItr].pick.setFillColor(sf::Color(100,100,100,255));
    } else if (option == _menus[config][action]) {
        _options[_textItr].label.setFillColor(sf::Color::White);
        _options[_textItr].pick.setFillColor(sf::Color::White);
    } else {
        _options[_textItr].label.setFillColor(sf::Color(200,200,200,255));
        _options[_textItr].pick.setFillColor(sf::Color(200,200,200,255));
    }

    _textItr++;
}


void OptionsMenu::_getAction()
{
    switch(_menus[config][action]) {
        case OMEN_USE_DIRECT_DRAW: {

        } break;
        case OMEN_DISPLAY_MODE: {
            setFullscreen(!confFullscreen);
        } break;
        case OMEN_WIN_RES: {
            toggleWindowScale();
        } break;
        case OMEN_USE_DESKTOP_RES: {
            setUseDesktopResolution(!confUseDesktopResolution);
        } break;
        case OMEN_USE_INTERPOLATION: {
            setInterpolation(!confUseInterpolation);
        } break;
        case OMEN_IGNORE_ASPECT: {
            setIgnoreAspectRatio(!confIgnoreAspectRatio);
        } break;
        case OMEN_ALLOW_UNEVEN_PIX: {
            setAllowUnevenPixels(!confAllowUnevenPixelSizes);
        } break;
        case OMEN_USE_SHADERS: {
            toggleShaders();
        } break;
        default: {} break;
    }
}

void OptionsMenu::_drawShadowed(sf::Text &text) {
    float offsetX = 1.0;
    float offsetY = 1.0;

    _color = text.getFillColor();
    _shadow = sf::Color::Black;

    text.setFillColor(_shadow);
    text.move(offsetX,offsetY);
    rendTex.draw(text);

    text.move(-offsetX,-offsetY);
    text.setFillColor(_color);
    rendTex.draw(text);
}
