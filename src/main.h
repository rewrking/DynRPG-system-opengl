#ifndef MAIN_H
#define MAIN_H

#include <DynRPG/DynRPG.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cstdio>
#include <sstream>

#include "OptionsMenu.h"

#define KEY_DOWN(vkey) (GetKeyState(vkey) & 0x8000)


std::map<std::string, std::string> configuration;
std::map<std::string, std::string> configGame;
char headerName[] = "Settings";
TCHAR gamePath[MAX_PATH];
std::string gameIniFilename;
std::stringstream filePath;

std::map<std::string, std::string> translationMap;
std::map<std::string, float> shaderParams;
std::map<std::string, int> configMap;

std::string confScreenshotPath;
std::string confTranslation;
std::string gameTitle;
std::string shaderName;

int desktopWidth;
int desktopHeight;

int confWindowScale = 1;
int confShaderId = 0;
int confNumShaders = 4;
bool confDisableVideoMenu = false;
bool confUseDirectDraw = false;
bool confFullscreen = false;
bool confStartFullscreen = true;
bool confEnableVSync = true;
bool confUseDesktopResolution = true;
bool confAllowUnevenPixelSizes = true;
bool confUseInterpolation = false;
bool confIgnoreAspectRatio = false;
bool confUseShaders = false;

bool useOpenGl = false;
bool openGlStartup = false;
bool keyDown = false;
bool altDown = false;
bool isQuitting = false;
bool reverseDir = false;
bool directDrawIsLargeWindow = false;
bool directDrawIsFullscreen = false;

// Movie stuff
bool playingMovie = false;

// SFML objects
sf::RenderWindow *m_win;
sf::Event *m_winevent;
sf::Texture *m_wintexture;
sf::Sprite *m_winsprite;
sf::Shader *m_winshader;
sf::Uint8 *sfmlImageBuffer;

OptionsMenu *videoOptions;
std::string systemGraphic;

RPG::Scene_T lastScene;
const RPG::Scene_T sceneOpenGl = static_cast<RPG::Scene_T>(77);

uint16_t *buffer;
uint16_t *sceneBuffer;

#define RM_WIDTH 320
#define RM_HEIGHT 240

//bool isWindowsVistaOrHigher();
void findAndReplace(std::string& source, std::string const& find, std::string const& replace);
void saveToIni(const char *keyName, const char *value, bool noSet = false);
int getFromIni(const char *keyName);
bool SFMLLoadScreenAsImage(sf::Texture *&texture);

void setDirectDraw(bool useDirectDraw);
void _setFullscreen();
void _setWindowed(int scale);
void setFullscreen(bool fullscreen);
void toggleWindowScale();
void setWindowScale(int scale);

void setUseDesktopResolution(bool useDesktop);
void setIgnoreAspectRatio(bool ignoreAspect);
void setAllowUnevenPixels(bool allowUneven);
void setInterpolation(bool interp);
void toggleShaders();
void setCurrentShader();

void printScreen(sf::Texture *&texture);
void openGlMenuCall();
void startOpenGlMenu();
void exitOpenGl();

void originalSetFullscreen(bool fullscreen);
void __stdcall hookedSetFullscreen(bool fullscreen);
void originalSetLargeWindow(bool largeWindow);
void __stdcall hookedSetLargeWindow(bool largeWindow);

#endif // MAIN_H
