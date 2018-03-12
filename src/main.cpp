// OpenGL Renderer
#include "main.h"


/** \brief 640x480 is the minimum fullscreen resolution on Vista or higher,
            as opposed to using 320x240 on older machines
    \return true if Vista+
*/
/*bool isWindowsVistaOrHigher()
{
   OSVERSIONINFO osvi;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&osvi);
   return osvi.dwMajorVersion >= 6;
}*/


/** \brief A simple find & replace function for strings.
    \param source The source string
    \param find The string of text to find
    \param replace The string of text to replace the found text with
*/
void findAndReplace(std::string& source, std::string const& find, std::string const& replace)
{
    for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;) {
        source.replace(i, find.length(), replace);
        i += replace.length();
    }
}


/** \brief Sets a new key value to Settings.ini
    \param keyName The key to set
    \param value The new value
*/
void saveToIni(const char *keyName, const char *value, bool noSet)
{
    // Sync both the ini to an RPG Maker variable from the keyMap
    int varId = configMap[keyName];
    if (varId >= 0) RPG::variables[varId] = atoi(value);

    // Set the new ini value
    if (!noSet) {
        filePath.str(std::string());
        filePath << gamePath << "\\" << gameIniFilename;
        WritePrivateProfileString(headerName, keyName, value, filePath.str().c_str());
    }
}


/** \brief Gets the value from a key in Settings.ini
    \param keyName The key to get
*/
int getFromIni(const char *keyName)
{
    // get the ini value
    filePath.str(std::string());
    filePath << gamePath << "\\" << gameIniFilename;
    return GetPrivateProfileInt(headerName, keyName, 0, filePath.str().c_str());
}

void getDesktopWidth()
{
    desktopWidth = sf::VideoMode::getDesktopMode().width;
    desktopHeight = sf::VideoMode::getDesktopMode().height;
}


/** \brief This is where the proverbial magic happens. It takes the 16-bit RM2k3 canvas at
            the current frame, converts it to 24bit and copies the pixel values to a Uint8
            array that SFML can then read and update it's texture with
    \param texture An SFML texture object to copy the RM2k3 pixels to
*/
bool SFMLLoadScreenAsImage(sf::Texture *&texture)
{
    // Store the canvas data of the current frame to a buffer
    // The buffer is a pointer to RPG::screen->canvas->getScanline(239) that is initialized when the game boots

    // This is a pointer to each rgba value in the new 24-bit buffer
    int lpPixPointer = 0;
    for(int y=0; y<RM_HEIGHT; y++) {
        for(int x=0; x<RM_WIDTH; x++) {
            // Convert the pixel values of the 2k3 canvas to an SFML usable format
            unsigned int hexValue = RPG::screen->canvas->convert16To24Bit(buffer[x + RM_WIDTH * (239 - y)]);
            sfmlImageBuffer[lpPixPointer]   = hexValue;        // red
            sfmlImageBuffer[lpPixPointer+1] = hexValue >> 8;   // green
            sfmlImageBuffer[lpPixPointer+2] = hexValue >> 16;  // blue
            sfmlImageBuffer[lpPixPointer+3] = 255;             // alpha
            lpPixPointer+=4;
        }
    }

    // Update the SFML texture
    // This new texture is still 320x240, and will get scaled up later using SFML functions
    texture->update(sfmlImageBuffer);
    return true;
}


/** \brief Toggles RM2k3's original DirectDraw mode (experimental)
    \param useDirectDraw true will turn DirectDraw back on, false will switch to OpenGL
*/
void setDirectDraw(bool useDirectDraw)
{
    if (useDirectDraw) {
        // Close out the OpenGL renderer
        if (useOpenGl) {
            if (m_win->isOpen()) {
                m_win->close();
                ShowWindow(GetParent(RPG::screen->getCanvasHWND()),SW_SHOW);
                SetActiveWindow(GetParent(GetParent(RPG::screen->getCanvasHWND())));
            }
            confUseDirectDraw = true;
        }
        // Reset the window size closest to the OpenGL one
        originalSetLargeWindow(confWindowScale >= 2);
    } else {
        // Force windowed mode & initialize OpenGL
        originalSetFullscreen(false);
        confUseDirectDraw = false;
        //_setWindowed(confWindowScale);
    }

    useOpenGl = !confUseDirectDraw;
    openGlStartup = false;

    saveToIni("UseDirectDraw", confUseDirectDraw ? "1" : "0");
}


/** \brief This is how the new renderer goes into fullscreen mode
*/
void _setFullscreen()
{
    // Max resolution is more crisp, but adheres to scaling rules (uneven pixels at some resolutions, etc)
    if (confUseDesktopResolution) {

        // Recreate the window object based on the fullscreen resolution
        m_win->create(sf::VideoMode(desktopWidth, desktopHeight), gameTitle, sf::Style::Fullscreen);
    } else {

        //bool large = isWindowsVistaOrHigher();
        bool large = true;
        int scale = large ? 2 : 1;

        int width = m_wintexture->getSize().x*scale; // 320 or 640
        int height = m_wintexture->getSize().y*scale; // 240 or 480

        // Recreate the window based on the minimum fullscreen resolution
        m_win->create(sf::VideoMode(width, height), gameTitle, sf::Style::Fullscreen);
    }

    ShowWindow(GetParent(GetParent(RPG::screen->getCanvasHWND())),SW_HIDE);

    // Hide the mouse cursor & turn on V-Sync
    m_win->setMouseCursorVisible(false);
    m_win->setVerticalSyncEnabled(confEnableVSync);

    // Save Settings.ini
    confFullscreen = true;
    saveToIni("Fullscreen", "1");

    // Reset the shader if necessary
    setCurrentShader();
}


/** \brief This is how the new renderer goes into windowed mode
    \param scale The window's "scale" is determined by toggleWindowScale function
*/
void _setWindowed(int scale)
{
    // Set the size of the window based on the scale. It'll always be based on square pixels (1x, 2x, 3x, etc)
    m_win->setSize(sf::Vector2u(m_wintexture->getSize().x*scale, m_wintexture->getSize().y*scale));

    // Creates the new SFML/OpenGL window inside of the old 2k3 one
    m_win->create(GetParent(GetParent(RPG::screen->getCanvasHWND())));

    // Make the mouse visible in windowed mode
    m_win->setMouseCursorVisible(true);
    m_win->setVerticalSyncEnabled(confEnableVSync);
    // Sets the game title
    m_win->setTitle(gameTitle);

    // This centers the window onto the screen (based on the updated size)
    // GetSystemMetrics() gets the width of the window border features (titlebar, etc)
    int posX = (desktopWidth/2)-((m_win->getSize().x + GetSystemMetrics(SM_CXBORDER)*2)/2);
    int posY = (desktopHeight/2)-((m_win->getSize().y + GetSystemMetrics(SM_CYBORDER)*4 + GetSystemMetrics(SM_CYCAPTION))/2);
    m_win->setPosition(sf::Vector2i(posX, posY));

    if (openGlStartup) {
        confFullscreen = false;
        saveToIni("Fullscreen", "0", confStartFullscreen);
    }
}


/** \brief Toggles between fullscreen & windowed mode
    \param fullscreen The new mode. true if fullscreen, false if windowed
*/
void setFullscreen(bool fullscreen)
{
    if (useOpenGl) {
        // OpenGL version
        confFullscreen = fullscreen;
        if (confFullscreen) {
            _setFullscreen();
        } else {
            _setWindowed(confWindowScale);
        }

        // Update the video options menu immediately after
        if (videoOptions->visible) {
            videoOptions->update(false);
        }

    } else {
        // DirectDraw version (could be called from a comment command
        directDrawIsFullscreen = fullscreen;
        originalSetFullscreen(directDrawIsFullscreen);

        int varId = configMap["Fullscreen"];
        if (varId >= 0) RPG::variables[varId] = directDrawIsFullscreen;
    }
}


/** \brief This is how the new renderer goes into windowed mode. The window's "scale" is determined
            by the maximum resolution the display can support, so if you have a 1920x1080 monitor,
            the max pixel-perfect scale is 4x (1280x1024), wheras 1920x1200 is 5x (1600x1200),
            4K (untested) is 9x (2880x2160), etc.
*/
void toggleWindowScale()
{
    if (useOpenGl) {
        // Fullscreen mode uses different config options as windowed mode, so act accordingly
        if (confFullscreen) {
            if (confUseDesktopResolution) {
                // Toggle between pixel-perfect scale (1x, 2x, 3x, etc.), scale with correct aspect ratio (ex: 4.5x in 1080p screens),
                // and "ignore aspect ratio" aka fill the entire resolution with no black bars (which is just plain evil)
                if (!confAllowUnevenPixelSizes) {
                    confAllowUnevenPixelSizes = true;
                    confIgnoreAspectRatio = false;
                } else {
                    if (!confIgnoreAspectRatio) confIgnoreAspectRatio = true;
                    else {
                        confIgnoreAspectRatio = false;
                        confAllowUnevenPixelSizes = false;
                    }
                }

                // Save the "allow uneven pixel sizes" boolean to Settings.ini
                saveToIni("AllowUnevenPixelSizes", confAllowUnevenPixelSizes ? "1" : "0");

                // Save the "ignore aspect ratio" boolean to Settings.ini
                saveToIni("IgnoreAspectRatio", confIgnoreAspectRatio ? "1" : "0");
            }

        } else {
            // Get the maximum allowed pixel-perfect scale from the desktop resolution
            // Having this here and not on startup ensures it's always up-to-date based on the current desktop resolution
            int maxScale = 1;
            if (desktopWidth >= desktopHeight) {
                maxScale = desktopHeight/m_wintexture->getSize().y;
            } else {
                // If the monitor is sideways (ex: 1080x1920) like some kind of monster, base the maxScale on the width
                maxScale = desktopWidth/m_wintexture->getSize().y;
            }

            // Cycle between scales accordingly based on the maxScale
            if (reverseDir) {
                if (confWindowScale > 1) confWindowScale--;
                else confWindowScale = maxScale;
            } else {
                if (confWindowScale < maxScale) confWindowScale++;
                else confWindowScale = 1;
            }
            _setWindowed(confWindowScale);

            // Save the last used zoom mode to the Settings.ini
            std::stringstream temp;
            temp << confWindowScale;
            saveToIni("WindowScale", temp.str().c_str());
        }

    } else {
        // DirectDraw mode detected, so use the original functions
        if (directDrawIsLargeWindow) originalSetLargeWindow(false);
        else originalSetLargeWindow(true);
    }
}


/** \brief Sets the window to a particular scale.
    \param scale The new scale
*/
void setWindowScale(int scale)
{
    if (useOpenGl) {
        if (!confFullscreen) {
            int maxScale = sf::VideoMode::getDesktopMode().height/RM_HEIGHT;

            if (scale > 0 && scale <= maxScale) {
                confWindowScale = scale;
                _setWindowed(confWindowScale);
            }

            // Save the last used zoom mode to the Settings.ini
            std::stringstream temp;
            temp << confWindowScale;
            saveToIni("WindowScale", temp.str().c_str());
        }
    }
}


/** \brief Sets the "use desktop resolution" config to a new value
    \param useDesktop The new "use desktop resolution" setting
*/
void setUseDesktopResolution(bool useDesktop)
{
    if (useOpenGl) {
        confUseDesktopResolution = useDesktop;

        // Refresh the fullscreen mode
        _setFullscreen();

        // Save the max resolution setting to the Settings.ini
        saveToIni("UseDesktopResolution", confUseDesktopResolution ? "1" : "0");
    }
}


/** \brief Sets the "ignore aspect ratio" config to a new value
    \param ignoreAspect The new "ignore aspect ratio" setting
*/
void setIgnoreAspectRatio(bool ignoreAspect)
{
    if (useOpenGl) {
        confIgnoreAspectRatio = ignoreAspect;

        // If the new ignore aspect value is true, set uneven pixels to true as well
        if (confFullscreen && confIgnoreAspectRatio) {
            confAllowUnevenPixelSizes = true;
        }

        // Save both updated settings to the Settings.ini
        saveToIni("IgnoreAspectRatio", confIgnoreAspectRatio ? "1" : "0");
        saveToIni("AllowUnevenPixelSizes", confAllowUnevenPixelSizes ? "1" : "0");
    }
}


/** \brief Sets the "allow uneven pixels" config to a new value
    \param allowUneven The new "allow uneven pixels" setting
*/
void setAllowUnevenPixels(bool allowUneven)
{
    if (useOpenGl) {
        confAllowUnevenPixelSizes = allowUneven;

        // If the new uneven pixels value is false, set ignore aspect to false as well
        if (confFullscreen && !confAllowUnevenPixelSizes) {
            confIgnoreAspectRatio = false;
        }

        // Save both updated settings to the Settings.ini
        saveToIni("IgnoreAspectRatio", confIgnoreAspectRatio ? "1" : "0");
        saveToIni("AllowUnevenPixelSizes", confAllowUnevenPixelSizes ? "1" : "0");
    }
}


/** \brief Sets interpolation to a new value
    \param interp The new interpolation setting
*/
void setInterpolation(bool interp)
{
    if (useOpenGl) {
        confUseInterpolation = interp;

        // Done via SFML's setSmooth
        m_wintexture->setSmooth(confUseInterpolation);

        // Save the last used interpolation mode to the Settings.ini
        saveToIni("UseInterpolation", confUseInterpolation ? "1" : "0");
    }
}


/** \brief Toggles between the shaders defined in the DynRPG.ini file
*/
void toggleShaders()
{
    if (useOpenGl) {
        if (confNumShaders > 0) {
            // reverse direction
            if (reverseDir) {
                if (confShaderId <= 0) {
                    confUseShaders = true;
                    confShaderId = confNumShaders;
                } else {
                    confShaderId--;
                    if (confShaderId == 0) confUseShaders = false;
                }
            // normal direction
            } else {
                if (confShaderId >= confNumShaders) {
                    confUseShaders = false;
                    confShaderId = 0;
                } else {
                    confShaderId++;
                    if (confShaderId == 1) confUseShaders = true;
                }
            }
            // Set the new shader here
            setCurrentShader();
        } else {
            confUseShaders = false;
        }

        // Saves whether shaders should be turned on or off
        std::stringstream temp1;
        temp1 << confUseShaders;
        saveToIni("UseShaders", temp1.str().c_str());

        // Saves the shader's ID
        std::stringstream temp2;
        temp2 << confShaderId;
        saveToIni("ShaderId", temp2.str().c_str());
    }
}


/** \brief Sets the current shader that has been selected & stores the name (without underscores)
*/
void setCurrentShader()
{
    if (confUseShaders) {
        // Create a temporary string to load things from the Settings.ini
        std::stringstream shaderString;
        shaderString << "Shader" << confShaderId;

        // Initialization of the current shader
        // Store the name (with underscores at this point
        shaderName = configuration[shaderString.str()];

        // Find out if a fragment & vertex shader has been defined
        bool shaderFrag = configuration[shaderString.str() + "frag"] == "true";
        bool shaderVert = configuration[shaderString.str() + "vert"] == "true";

        // This is the SFML way of loading each shader type
        // Both fragment & vertex
        if (shaderFrag && shaderVert) {
            m_winshader->loadFromFile("Shaders\\" + shaderName + ".vert", "Shaders\\" + shaderName + ".frag");
        // Fragment
        } else if (shaderFrag) {
            m_winshader->loadFromFile("Shaders\\" + shaderName + ".frag", sf::Shader::Fragment);
        // Vertex
        } else if (shaderVert) {
            m_winshader->loadFromFile("Shaders\\" + shaderName + ".vert", sf::Shader::Vertex);
        }

        // A "texture_dimensions" uniform is required for pixel shaders, so it will almost always be needed
        bool shaderUseTexDims = configuration[shaderString.str() + "useTexDims"] == "true";
        if (shaderUseTexDims) {
            m_winshader->setUniform("texture_dimensions", sf::Glsl::Vec2(m_wintexture->getSize().x, m_wintexture->getSize().y));
        }

        // Get the rest of the uniforms that are set in the Settings.ini (lots of room for improvement here since shaders are their own animal)
        int shaderNumFloatParams = atoi(configuration["numFloatParams"].c_str());
        for (int i=1; i <= shaderNumFloatParams; i++) {
            std::stringstream temp2name;
            temp2name << shaderString.str() << "paramFloat" << i << "name";
            m_winshader->setUniform(configuration[temp2name.str()], shaderParams[configuration[temp2name.str()]]);
        }

        // Replace the underscores with spaces in the shader name so it can be referenced by menus
        findAndReplace(shaderName, "_", " ");
    }
}


/** \brief Creates a screenshot
    \param texture The SFML texture to be set as the screenshot
*/
void printScreen(sf::Texture *&texture)
{
    if (useOpenGl) {

        // Supports 10000 screenshots (0000 to 9999), but won't actually loop this many times unless you have that many
        for (int i=0; i<10000; i++) {
            std::stringstream screenFilename;
            screenFilename << confScreenshotPath;
            // Add a backslash to the filepath if it's not present
            if (confScreenshotPath[confScreenshotPath.length()-1] != '\\') screenFilename << "\\";
            screenFilename << "Screenshot";
            if (i < 1000) screenFilename << "0";
            if (i < 100) screenFilename << "0";
            if (i < 10) screenFilename << "0";
            screenFilename << i << ".png";
            // FindFirstFile function load
            WIN32_FIND_DATA lpFindFileData;
            HANDLE hFind;
            // Find the file defined in char "saveGlobalPath"
            hFind = FindFirstFile(screenFilename.str().c_str(), &lpFindFileData);
            if(hFind != INVALID_HANDLE_VALUE) {
                FindClose(hFind);
            } else {
                // Use SFML's copyToImage & saveToFile functions to do the hard stuff
                texture->copyToImage().saveToFile(screenFilename.str());
                break;
            }
        }
    }
}


/** \brief Calls the OpenGL Video Options menu, but excludes certain scenes
*/
void openGlMenuCall()
{
    // Exclude the GameJolt menu (DynRPG plugin) since it causes some game-breaking bugs from scene switching
    if (RPG::system->scene != RPG::SCENE_GAMEJOLT) {
        // Also exclude the battle scene in certain phases for the similar reasons
        if (RPG::system->scene == RPG::SCENE_BATTLE) {
            if ((!RPG::battleData->isVictory) && RPG::battleData->battlePhase == RPG::BPHASE_BATTLE)
                startOpenGlMenu();
        } else startOpenGlMenu();
    }
}


/** \brief Initializes the OpenGL Video Options menu
*/
void startOpenGlMenu()
{
    if (useOpenGl) {
        // Copies a freeze frame of the current scene before switching to the new scene
        memcpy(sceneBuffer, RPG::screen->canvas->getScanline(239), RM_WIDTH * RM_HEIGHT * 2);
        videoOptions->init();

        // Store the old scene & switch to the new one
        lastScene = RPG::system->scene;
        RPG::system->scene = sceneOpenGl;
    }
}


/** \brief Function used to halt OpenGL stuff before quitting
*/
void exitOpenGl()
{
    if (useOpenGl) {
        // Switch to a window
        if (confFullscreen) _setWindowed(confWindowScale);
        // Prevent updating
        isQuitting = true;
        // Close
        if (m_win->isOpen()) m_win->close();
        useOpenGl = false;
    }
}


/** \brief Original setFullscreen function
    \param fullscreen
*/
void originalSetFullscreen(bool fullscreen) {
    asm volatile(
        "call 1f; \
         jmp 2f; \
         1:; \
         push %%ebx; \
         push %%esi; \
         addl $-8, %%esp; \
         movl %%edx, %%ebx; \
         movl %%eax, %%esi; \
         cmpb 0x3C(%%esi), %%bl; \
         jmp *%%edi; \
         2:"
        : "=a" (RPG::_eax), "=d" (RPG::_edx)
        : "D" (0x46B2A8), "a" (RPG::screen), "d" (fullscreen)
        : "ecx", "cc", "memory"
    );
    RPG::screen->fullScreen = fullscreen;
    directDrawIsFullscreen = fullscreen;
};


/** \brief Hooked setFullscreen function
    \param fullscreen
*/
void __stdcall hookedSetFullscreen(bool fullscreen) {
    // Only use the original function if in DirectDrawMode
    if (confUseDirectDraw) originalSetFullscreen(fullscreen);
};


/** \brief Original setLargeWindow function
    \param largeWindow
*/
void originalSetLargeWindow(bool largeWindow) {
    asm volatile(
        "call 1f; \
         jmp 2f; \
         1:; \
         push %%esi; \
         push %%edi; \
         addl $-16, %%esp; \
         movl %%eax, %%esi; \
         cmpb 0x3D(%%esi), %%bl; \
         jmp *%%ebx; \
         2:"
        : "=a" (RPG::_eax), "=d" (RPG::_edx)
        : "b" (0x46B40A), "a" (RPG::screen), "d" (largeWindow)
        : "ecx", "cc", "memory"
    );
    RPG::screen->largeWindow = largeWindow;
    directDrawIsLargeWindow = largeWindow;
};


/** \brief Hooked setLargeWindow function
    \param largeWindow
*/
void __stdcall hookedSetLargeWindow(bool largeWindow) {
    // Only use the original function if in DirectDrawMode
    if (confUseDirectDraw) originalSetLargeWindow(largeWindow);
};


/** ===================================================================================================
    RM2003/DynRPG Callbacks
*/
bool onStartup(char *pluginName)
{
    // Get configuration settings for key variables
    configuration = RPG::loadConfiguration(pluginName);
    GetCurrentDirectory(MAX_PATH, gamePath);
    gameIniFilename = configuration["GameIniFilename"];
    confNumShaders = atoi(configuration["NumShaders"].c_str());
    confDisableVideoMenu = configuration["DisableVideoMenu"] == "true";

    // Settings.ini
    configGame = RPG::loadConfiguration(headerName, &gameIniFilename[0]);
    confUseDirectDraw = atoi(configGame["UseDirectDraw"].c_str());
    confFullscreen = atoi(configGame["Fullscreen"].c_str());
    confStartFullscreen = atoi(configGame["StartFullscreen"].c_str());
    confEnableVSync = atoi(configGame["EnableVSync"].c_str());
    confUseDesktopResolution = atoi(configGame["UseDesktopResolution"].c_str());
    confAllowUnevenPixelSizes = atoi(configGame["AllowUnevenPixelSizes"].c_str());
    confUseInterpolation = atoi(configGame["UseInterpolation"].c_str());
    confIgnoreAspectRatio = atoi(configGame["IgnoreAspectRatio"].c_str());
    confWindowScale = atoi(configGame["WindowScale"].c_str());
    confScreenshotPath = configGame["ScreenshotPath"];
    confUseShaders = atoi(configGame["UseShaders"].c_str()) == 1;
    confShaderId = atoi(configGame["ShaderId"].c_str());
    confTranslation = configGame["Translation"].c_str();

    // Handle translations
    std::stringstream translationFile;
    translationFile << "DynPlugins\\" << pluginName << "_translation.ini";
    translationMap = RPG::loadConfiguration(&confTranslation[0], &translationFile.str()[0]);;

    // These basically serve the same purpose, but useOpenGl is less confusing to read eveywhere
    useOpenGl = !confUseDirectDraw;

    if (confUseShaders) {
        // Make sure the current shader ID is no larger than the maximum
        if (confShaderId > confNumShaders)
            confShaderId = confNumShaders;
    }

    // Get the desktop width/height before anything else has loaded
    getDesktopWidth();

    // Make sure the scale is no greater than the max scale
    int maxScale = desktopHeight/RM_HEIGHT;
    if (confWindowScale > 0) {
        if (confWindowScale > maxScale) confWindowScale = maxScale;
    }

    // HERE BE DRAGONS!!!
    //********************************************************************
    // the following cast forces RPG_RT.exe to start in a window
    //   (which is later fullscreened based on the ini settings
    *reinterpret_cast<short *>(0x48FA57) = 0x9090;
    //********************************************************************
    // This hides the gray border
    *reinterpret_cast<uint8_t *>(0x46A91C) = 0x00;
    *reinterpret_cast<uint8_t *>(0x46B8CA) = 0x00;
    *reinterpret_cast<uint8_t *>(0x46B38B) = 0x00;
    *reinterpret_cast<uint8_t *>(0x46B3A5) = 0x00;
    //********************************************************************

    // --Debugging console--
    /*AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    printf("%s\n",pluginName);*/

    return true;
}

void onInitFinished() {
    // HERE BE MORE DRAGONS!!!
    //********************************************************************
    // Hiding the window parents are generally for presentation purposes, but this one in particular speeds up the initial fullscreen
    if (useOpenGl) ShowWindow(GetParent(RPG::screen->getCanvasHWND()),SW_HIDE);

    buffer = static_cast<unsigned short*>(malloc(RM_WIDTH * RM_HEIGHT * 2));
    sceneBuffer = static_cast<unsigned short*>(malloc(RM_WIDTH * RM_HEIGHT * 2));

    // Prevent the old F4 + F5 behavior (Thanks Cherry)
    // Install toggleFullscreen hook
    *reinterpret_cast<uint32_t *>(0x46B29C) = 0xB8515259;
    *reinterpret_cast<void **>(0x46B2A0) = (void*)&hookedSetFullscreen;
    *reinterpret_cast<uint16_t *>(0x46B2A4) = 0xE0FF;

    // Install toggleLargeWindow hook
    *reinterpret_cast<uint32_t *>(0x46B400) = 0xB8515259;
    *reinterpret_cast<void **>(0x46B404) = (void*)&hookedSetLargeWindow;
    *reinterpret_cast<uint16_t *>(0x46B408) = 0xE0FF;
    //********************************************************************

    // Get the game title from the map tree to use for the window's title bar
    gameTitle = RPG::mapTree->properties[0]->name.s_str();

    // Create SFML pointers
    m_win = new sf::RenderWindow();
    m_winevent = new sf::Event();
    m_wintexture = new sf::Texture();
    m_winsprite = new sf::Sprite();
    m_winshader = new sf::Shader();

    // Custom Video Options menu
    videoOptions = new OptionsMenu();

    m_wintexture->create(RM_WIDTH, RM_HEIGHT);
    m_winsprite->setTexture(*m_wintexture);
    m_winsprite->setOrigin(m_wintexture->getSize().x/2,m_wintexture->getSize().y/2);

    int pixelArraySize = (RPG::screen->canvas->width()*RPG::screen->canvas->height())*4;
    sfmlImageBuffer = new sf::Uint8[pixelArraySize];

    systemGraphic = RPG::system->systemGraphicFilename.s_str();
}

// I don't remember why this was here, but it's been commented out for a long time, so it's probably not needed
/*void onInitTitleScreen() {
    //uint8_t &rmWindowActive = (**reinterpret_cast<uint8_t ***>(0x4CDE78))[157];
    //rmWindowActive = 1;
}*/

void onDrawScreen() {
    // This startup section is shared between OpenGL & the old DirectDraw renderer
    // In DD mode, it will go to fullscreen if Fullscreen=1 in Settings.ini
    if (!openGlStartup) {
        // Initialize the main buffer pointer. We use this to capture the screen later on
        buffer = RPG::screen->canvas->getScanline(239);

        if (useOpenGl) {
            ShowWindow(GetParent(RPG::screen->getCanvasHWND()),SW_HIDE);
            // Calling this twice is a hacky way to force the window frame to fit to the game's edge
            // This is one of the mysteries of this plugin
            _setWindowed(1);
            _setWindowed(confWindowScale);

            // Set the shader if it's enabled
            setCurrentShader();
            setInterpolation(confUseInterpolation);
        }
        if (confFullscreen) {
            if (useOpenGl) {
                _setFullscreen();
            } else {
                originalSetFullscreen(true);
            }
        }
        openGlStartup = true;
    }


    // Main Draw section
    //********************************************************************
    if (m_win->isOpen() && useOpenGl) {

        // If in the Video Options scene, copy the main buffer to the sceneBuffer
        if (RPG::system->scene == sceneOpenGl) {
            memcpy(buffer, sceneBuffer, RM_WIDTH * RM_HEIGHT * 2);
        }

        // For Alt+F4 event
        if (m_win->pollEvent(*m_winevent)) {
            if (m_winevent->type == sf::Event::Closed) {
                RPG::quitGame();
            }
        }

        if (!isQuitting) {

            // Copy the RPG Maker frame to a SFML texture
            if(!SFMLLoadScreenAsImage(m_wintexture)) {
                printf("Error loading the image from Rm2k3!\n");
            }

            //setCurrentShader();

            // Key inputs
            // I'd rather use sf::Event for these, but 2k3's main process overrides them
            altDown = KEY_DOWN(VK_LMENU) || KEY_DOWN(VK_RMENU);
            // F4 key, or Alt+Enter
            if ((KEY_DOWN(VK_F4) && !altDown) || (altDown && KEY_DOWN(VK_RETURN))) {
                if (!keyDown) {
                    setFullscreen(!confFullscreen);
                    if (videoOptions->visible) videoOptions->updateMenu();
                    keyDown = true;
                }
            // F5 key
            } else if (KEY_DOWN(VK_F5)) {
                if (!keyDown) {
                    if (!confDisableVideoMenu && !videoOptions->visible && !playingMovie) openGlMenuCall();
                    keyDown = true;
                }
            // Perfect screenshot capturing (set to "Print Screen" key
            } else if (KEY_DOWN(VK_SNAPSHOT) && !KEY_DOWN(VK_LMENU)) {
                if (!keyDown) {
                    printScreen(m_wintexture);
                    keyDown = true;
                }
            } else if (keyDown) keyDown = false;

            // Clear the new renderer screen every frame
            m_win->clear(sf::Color::Black);

            // Set the scaling & sprite position of the main window sprite
            if (confFullscreen && confUseDesktopResolution) {
                m_winsprite->setPosition(desktopWidth*0.5, desktopHeight*0.5);
                if (!confAllowUnevenPixelSizes) {
                    // pixel perfect scaling
                    int perfectScale = desktopHeight/m_wintexture->getSize().y;
                    m_winsprite->setScale(perfectScale,perfectScale);
                } else {
                    // pleb scaling
                    float plebScale = desktopHeight/(static_cast<float>(m_wintexture->getSize().y));
                    float plebARignore = desktopWidth/(static_cast<float>(m_wintexture->getSize().x));
                    if (confIgnoreAspectRatio) {
                        m_winsprite->setScale(plebARignore,plebScale);
                    } else {
                        m_winsprite->setScale(plebScale,plebScale);
                    }
                }
            } else {
                // Windowed mode or Fullscreen mode @ 640x480
                m_winsprite->setPosition(m_win->getSize().x/2, m_win->getSize().y/2);
                if (!confFullscreen) m_winsprite->setScale(confWindowScale,confWindowScale);
                else m_winsprite->setScale(m_win->getSize().x/RM_WIDTH, m_win->getSize().y/RM_HEIGHT);
            }

            // Draw the sprite (aka what RM2k3 is being drawn onto)
            // With shader
            if (confUseShaders) m_win->draw(*m_winsprite, *&m_winshader);
            // Without shader
            else m_win->draw(*m_winsprite);


            if (playingMovie) {
                m_win->clear(sf::Color::Black);

                if (!RPG::screen->movieIsPlaying) {
                    playingMovie = false;

                    HWND winContext = GetParent(RPG::screen->getCanvasHWND());
                    if (confFullscreen) {
                        m_win->setVisible(true);
                        SetWindowPos(winContext, HWND_TOP, 0, 0, RM_WIDTH*confWindowScale, RM_HEIGHT*confWindowScale, SWP_FRAMECHANGED);
                        ShowWindow(GetParent(winContext), SW_HIDE);
                    } else {
                        ShowWindow(winContext, SW_HIDE);
                    }
                }
            }

            // Update & draw Video Options menu
            if (videoOptions->visible) {
                // 2k3 sadly doesn't differentiate between update/draw very well, so both are called here
                videoOptions->update();
                videoOptions->draw();

                if (videoOptions->exitScene && !videoOptions->keyDown) {
                    videoOptions->visible = false;
                    if (lastScene < 30) {
                        RPG::system->scene = lastScene;
                    }
                }
            }

            // end the current frame & display the render window
            m_win->display();
        }
    }
}

bool onEventCommand(RPG::EventScriptLine *scriptLine, RPG::EventScriptData *scriptData,
                      int eventId, int pageId, int lineId, int *nextLineId)
{
    // HERE BE A TINY DRAGON!!
    //********************************************************************
    // If someone accidentally left a Better AEP patch, this prevents it from running (Thanks Cherry)
    if (scriptLine->command == RPG::EVCMD_CHANGE_VARIABLE) {
        if(*reinterpret_cast<uint32_t *>(0x4C9DA0) == 0x0024588B) {         // if BAEP installed
            int betterAepVariableId = *reinterpret_cast<int *>(0x4C9DA4);   // get variable ID (usually 3350)
            if (scriptLine->parameter(1) == betterAepVariableId
                    && scriptLine->parameter(5) == 2
                    && scriptData->line(lineId+1)->command == RPG::EVCMD_STOP_EVENT) {
                if (useOpenGl) {
                    if (confFullscreen) _setWindowed(confWindowScale);
                    isQuitting = true;
                    m_win->close();
                }
                // return false would prevent betterAEP's code from running
                return false;
            }
        }
    }
    //********************************************************************

    if (scriptLine->command == RPG::EVCMD_PLAY_MOVIE && useOpenGl) {
        // Hide the 2k3 canvas since it will appear when
        scriptLine->parameters[1] = 0 + scriptLine->parameters[1];
        // Amazingly, the 0.1 is enough offset to place the movie outside of the canvas's context window
        // If it's inside, it won't display at all if parameters 3 & 4 are bigger than 640x480
        scriptLine->parameters[2] = 0.1 + scriptLine->parameters[2];
        scriptLine->parameters[3] = scriptLine->parameters[3]*confWindowScale;
        scriptLine->parameters[4] = scriptLine->parameters[4]*confWindowScale;

        HWND winContext = GetParent(RPG::screen->getCanvasHWND());
        if (confFullscreen) {
            m_win->setVisible(false);
            ShowWindow(GetParent(winContext),SW_SHOW);
            SetWindowPos(winContext, HWND_TOP, 0, 0, 640, 480, SWP_SHOWWINDOW);
        } else {
            SetWindowPos(winContext, HWND_TOP, 0, 0, 640, 480, SWP_SHOWWINDOW);
        }

        // Hide the 2k3 canvas since it will appear when
        RPG::screen->canvasRect.top = 0;
        RPG::screen->canvasRect.left = 0;
        RPG::screen->canvasRect.right = 0;
        RPG::screen->canvasRect.bottom = 0;

        playingMovie = true;

        return true;
    }

    return true;
}

bool onComment(const char *text, const RPG::ParsedCommentData *parsedData, RPG::EventScriptLine *nextScriptLine,
		       RPG::EventScriptData *scriptData, int eventId, int pageId, int lineId, int *nextLineId)
{
    std::string cmd = parsedData->command;

    /** \brief For building your own menu in 2k3. This will bind Settings.ini updates to a 2k3 Variable so they are always synced (less work in 2k3)
        \param (text) The Settings.ini key
        \param (number) The variable ID
    */
    if (cmd == "bind_variable" && parsedData->parametersCount == 2) {
        // Limit to these ini values:
        std::string textParam = parsedData->parameters[0].text;
        if (textParam == "UseDirectDraw" || textParam == "EnableVSync" || textParam == "Fullscreen" || textParam == "UseShaders" ||
            textParam == "ShaderId" || textParam == "AllowUnevenPixelSizes" || textParam == "UseDesktopResolution" || textParam == "IgnoreAspectRatio" ||
            textParam == "UseInterpolation" || textParam == "WindowScale") {

            // Load only
            int varId = parsedData->parameters[1].number;
            int value = getFromIni(textParam.c_str());
            RPG::variables[varId] = value;
            configMap[textParam] = varId;
        }
        return false;
    }

    /** \brief A simple Quit game command
    */
    if (cmd == "quit_game") {
        exitOpenGl();
        RPG::quitGame();
        return false;
    }

    /** \brief Loop through available shaders
        \param (number) reverse direction (1), normal direction (0)
    */
    if (cmd == "toggle_shaders") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            reverseDir = parsedData->parameters[0].number;
            toggleShaders();
        }
        return false;
    }

    /** \brief Set interpolation
        \param (number) on (1), off (0)
    */
    if (cmd == "set_interpolation") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setInterpolation(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Use desktop resolution & refresh fullscreen
        \param (number) on (1), off (0)
    */
    if (cmd == "set_use_max_resolution") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setUseDesktopResolution(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Ignore aspect ratio
        \param (number) on (1), off (0)
    */
    if (cmd == "set_ignore_aspect_ratio") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setIgnoreAspectRatio(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Allow uneven pixels
        \param (number) on (1), off (0)
    */
    if (cmd == "set_allow_uneven_pixels") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setAllowUnevenPixels(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Set fullscreen/windowed
        \param (number) fullscreen (1), windowed (0)
    */
    if (cmd == "set_fullscreen") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setFullscreen(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Toggle window scale
        \param (number) reverse direction (1), normal direction (0)
    */
    if (cmd == "toggle_window_scale") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            reverseDir = parsedData->parameters[0].number == 1;
            toggleWindowScale();
        }
        return false;
    }

    /** \brief Set window scale
        \param (number) The new scale (if out of range, will set to min/max)
    */
    if (cmd == "set_window_scale") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            // the new scale is evaluated in the function
            setWindowScale(parsedData->parameters[0].number);
        }
        return false;
    }

    /** \brief Set DirectDraw mode
        \param (number) DirectDraw (1), OpenGL (0)
    */
    if (cmd == "set_direct_draw") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            setDirectDraw(parsedData->parameters[0].number == 1);
        }
        return false;
    }

    /** \brief Treats a hero name as a string and set one to the window resolution (in "width x height" format)
        \param (number) The ID of the hero
    */
    if (cmd == "set_window_resolution_to_hero_name") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            int heroId = parsedData->parameters[0].number;
            std::stringstream temp;
            if (confUseDirectDraw) {
                if (RPG::screen->largeWindow) temp << "640 x 480";
                else temp << "320 x 240";
            } else {
                temp << m_wintexture->getSize().x*confWindowScale << " x " << m_wintexture->getSize().y*confWindowScale;
            }
            if (RPG::dbActors[heroId]) RPG::dbActors[heroId]->name = temp.str();
        }
        return false;
    }

    /** \brief Treats a hero name as a string and set one to the name of the current shader (or "None")
        \param (number) The ID of the hero
    */
    if (cmd == "set_shader_name_to_hero_name") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            int heroId = parsedData->parameters[0].number;
            std::stringstream shaderString;
            shaderString << "Shader" << confShaderId;
            shaderName = configuration[shaderString.str()];
            findAndReplace(shaderName, "_", " ");
            std::stringstream temp;
            if (confUseShaders) {
                temp << shaderName;
            } else {
                temp << translationMap["None"];
            }
            if (RPG::dbActors[heroId]) RPG::dbActors[heroId]->name = temp.str();
        }
        return false;
    }

    /** \brief Treats a hero name as a string and set one to the desktop resolution (in "Yes (width x height)" format)
        \param (number) The ID of the hero
    */
    if (cmd == "set_desktop_res_to_hero_name") {
        if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
            int heroId = parsedData->parameters[0].number;
            std::stringstream temp;
            temp << translationMap["Yes"] << " (" << desktopWidth << " x " << desktopHeight << ")";
            if (RPG::dbActors[heroId]) RPG::dbActors[heroId]->name = temp.str();
        }
        return false;
    }

    /** \brief Sets a shader's float parameter (experimental)
        \param (text) The shader's parameter
        \param (text) A textual representation of a float value (ex: "0.975")
    */
    if (cmd == "set_shader_param" && parsedData->parametersCount == 2) {
        if (parsedData->parameters[0].type == RPG::PARAM_STRING && parsedData->parameters[1].type == RPG::PARAM_STRING) {
            shaderParams[parsedData->parameters[0].text] = atof(parsedData->parameters[1].text);
        }
        return false;
    }

    /** \brief In DirectDraw mode, sets a switch to whether it's in large window mode or not
        \param (number) the ID of the switch
    */
    if (cmd == "get_direct_draw_large_window") {
        if (confUseDirectDraw) {
            if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
                RPG::switches[parsedData->parameters[0].number] = directDrawIsLargeWindow;
            }
        }
        return false;
    }

    /** \brief In DirectDraw mode, sets a switch to whether it's in fullscreen mode or not
        \param (number) the ID of the switch
    */
    if (cmd == "get_direct_draw_is_fullscreen") {
        if (confUseDirectDraw) {
            if (parsedData->parametersCount == 1 && parsedData->parameters[0].type == RPG::PARAM_NUMBER) {
                RPG::switches[parsedData->parameters[0].number] = directDrawIsFullscreen;
            }
        }
        return false;
    }

    /** \brief Call the video options menu from anywhere
    */
    if (cmd == "load_video_options") {
        openGlMenuCall();
        return false;
    }


    return true;
}


void onExit() {
    exitOpenGl();

    // Delete SFML pointers
    delete videoOptions;

    delete[] sfmlImageBuffer;
    delete m_win;
    delete m_winevent;
    delete m_wintexture;
    delete m_winsprite;
    delete m_winshader;
}
