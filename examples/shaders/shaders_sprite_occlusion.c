/*******************************************************************************************
*
*   raylib [shaders] example - sprite occlusion
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3)
*
*   Example originally created with raylib 6.5, last time updated with raylib 6.5
*
*   Example contributed by Peter0x44 (@Peter0x44) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Peter0x44 (@Peter0x44)
*
********************************************************************************************
*
*   Four common techniques to keep a sprite visible when it walks behind a foreground object:
*     - Fade:       Draw the occluder semi-transparent (simple, but blends colors)
*     - Dither:     Discard occluder pixels around the sprite using an ordered dithering pattern
*     - Cutout:     Discard occluder pixels inside a circle around the sprite
*     - Silhouette: Tint the occluder with a flat color where the sprite is hidden
*
*   NOTE: Tree sprite from "Foliage Pack" by Kenney (www.kenney.nl), licensed as CC0
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    EFFECT_FADE = 0,
    EFFECT_DITHER,
    EFFECT_CUTOUT,
    EFFECT_SILHOUETTE
} OcclusionEffect;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - sprite occlusion");

    // Every effect is shown in its own panel, scene is defined at full screen size
    // and drawn at half size into every panel
    const int panelWidth = screenWidth/2;
    const int panelHeight = screenHeight/2;

    const char *effectNames[4] = { "Fade", "Dither", "Cutout", "Silhouette" };

    Texture2D texTree = LoadTexture("resources/kenney_tree.png");   // Occluder
    Texture2D texScarfy = LoadTexture("resources/scarfy.png");      // Spritesheet, 6 frames

    // Textures are drawn scaled down, smooth them
    SetTextureFilter(texTree, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texScarfy, TEXTURE_FILTER_BILINEAR);

    const int horizonY = 280;       // Where the sky meets the ground
    const int pathY = 360;          // Where scarfy walks, behind the tree
    const int treeY = 430;          // Where the tree stands, in front of the path

    Rectangle frameRec = { 0.0f, 0.0f, (float)texScarfy.width/6, (float)texScarfy.height };
    int currentFrame = 0;
    int framesCounter = 0;

    Vector2 treePosition = { (float)(screenWidth - texTree.width)/2, (float)(treeY - texTree.height) };
    Vector2 scarfyPosition = { -frameRec.width, pathY - frameRec.height };
    float scarfySpeed = 160.0f;     // Pixels per second
    float fadeAlpha = 1.0f;         // Current tree alpha for the fade effect

    // Load shaders
    // NOTE: All shaders work in tree texture pixel coordinates, so they don't depend on
    // where or how big the tree is drawn on screen
    Shader shdrDither = LoadShader(0, TextFormat("resources/shaders/glsl%i/occlusion_dither.fs", GLSL_VERSION));
    Shader shdrCutout = LoadShader(0, TextFormat("resources/shaders/glsl%i/occlusion_cutout.fs", GLSL_VERSION));
    Shader shdrSilhouette = LoadShader(0, TextFormat("resources/shaders/glsl%i/occlusion_silhouette.fs", GLSL_VERSION));

    Shader shaders[3] = { shdrDither, shdrCutout, shdrSilhouette };
    int playerPositionLocs[3] = { 0 };

    Vector2 treeSize = { (float)texTree.width, (float)texTree.height };
    for (int i = 0; i < 3; i++)
    {
        playerPositionLocs[i] = GetShaderLocation(shaders[i], "playerPosition");
        SetShaderValue(shaders[i], GetShaderLocation(shaders[i], "treeSize"), &treeSize, SHADER_UNIFORM_VEC2);
    }

    // Silhouette shader also requires the scarfy spritesheet and current frame to know where it is hidden
    Vector2 playerSize = { frameRec.width, frameRec.height };
    int playerTextureLoc = GetShaderLocation(shdrSilhouette, "texture1");
    int playerFrameLoc = GetShaderLocation(shdrSilhouette, "playerFrame");
    SetShaderValue(shdrSilhouette, GetShaderLocation(shdrSilhouette, "playerSize"), &playerSize, SHADER_UNIFORM_VEC2);

    Camera2D camera = { 0 };
    camera.zoom = 0.5f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Walk from left to right, looping back to the left side
        scarfyPosition.x += scarfySpeed*GetFrameTime();
        if (scarfyPosition.x > screenWidth) scarfyPosition.x = -frameRec.width;

        // Update walk animation frame
        framesCounter++;
        if (framesCounter >= 6)
        {
            framesCounter = 0;
            currentFrame = (currentFrame + 1)%6;
            frameRec.x = currentFrame*frameRec.width;
        }

        // Scarfy center relative to tree texture, in pixels
        Vector2 playerPosition = {
            scarfyPosition.x + frameRec.width/2 - treePosition.x,
            scarfyPosition.y + frameRec.height/2 - treePosition.y
        };

        for (int i = 0; i < 3; i++) SetShaderValue(shaders[i], playerPositionLocs[i], &playerPosition, SHADER_UNIFORM_VEC2);

        // Current frame in spritesheet, normalized
        Vector4 playerFrame = {
            frameRec.x/texScarfy.width, frameRec.y/texScarfy.height,
            frameRec.width/texScarfy.width, frameRec.height/texScarfy.height
        };
        SetShaderValue(shdrSilhouette, playerFrameLoc, &playerFrame, SHADER_UNIFORM_VEC4);

        // Fade tree smoothly while scarfy overlaps it
        Rectangle scarfyRec = { scarfyPosition.x, scarfyPosition.y, frameRec.width, frameRec.height };
        Rectangle treeRec = { treePosition.x, treePosition.y, (float)texTree.width, (float)texTree.height };
        float fadeTarget = CheckCollisionRecs(scarfyRec, treeRec)? 0.4f : 1.0f;
        fadeAlpha += (fadeTarget - fadeAlpha)*0.1f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < 4; i++)
            {
                Rectangle panel = { (float)((i%2)*panelWidth), (float)((i/2)*panelHeight), (float)panelWidth, (float)panelHeight };
                camera.offset = (Vector2){ panel.x, panel.y };

                BeginScissorMode((int)panel.x, (int)panel.y, (int)panel.width, (int)panel.height);
                BeginMode2D(camera);

                    // Draw sky, ground and path
                    DrawRectangleGradientV(0, 0, screenWidth, horizonY, (Color){ 120, 190, 240, 255 }, (Color){ 210, 235, 250, 255 });
                    DrawRectangle(0, horizonY, screenWidth, screenHeight - horizonY, (Color){ 130, 190, 50, 255 });
                    DrawRectangle(0, pathY - 24, screenWidth, 32, (Color){ 220, 190, 130, 255 });

                    // Draw scarfy first, it's behind the tree
                    DrawTextureRec(texScarfy, frameRec, scarfyPosition, WHITE);

                    // Draw tree applying the occlusion effect
                    switch (i)
                    {
                        case EFFECT_FADE: DrawTextureV(texTree, treePosition, Fade(WHITE, fadeAlpha)); break;
                        case EFFECT_DITHER:
                        {
                            BeginShaderMode(shdrDither);
                                DrawTextureV(texTree, treePosition, WHITE);
                            EndShaderMode();
                        } break;
                        case EFFECT_CUTOUT:
                        {
                            BeginShaderMode(shdrCutout);
                                DrawTextureV(texTree, treePosition, WHITE);
                            EndShaderMode();
                        } break;
                        case EFFECT_SILHOUETTE:
                        {
                            BeginShaderMode(shdrSilhouette);
                                // NOTE: Additional textures must be set after BeginShaderMode(),
                                // EndShaderMode() resets active textures
                                SetShaderValueTexture(shdrSilhouette, playerTextureLoc, texScarfy);
                                DrawTextureV(texTree, treePosition, WHITE);
                            EndShaderMode();
                        } break;
                        default: break;
                    }

                EndMode2D();
                EndScissorMode();

                // Draw panel label
                DrawRectangle((int)panel.x + 10, (int)panel.y + 10, MeasureText(effectNames[i], 20) + 16, 30, Fade(RAYWHITE, 0.8f));
                DrawText(effectNames[i], (int)panel.x + 18, (int)panel.y + 15, 20, DARKGRAY);
            }

            // Draw panels separation
            DrawLine(panelWidth, 0, panelWidth, screenHeight, DARKGRAY);
            DrawLine(0, panelHeight, screenWidth, panelHeight, DARKGRAY);

            DrawText("Tree by Kenney (www.kenney.nl)", screenWidth - 170, screenHeight - 16, 10, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shdrDither);       // Unload dither shader
    UnloadShader(shdrCutout);       // Unload cutout shader
    UnloadShader(shdrSilhouette);   // Unload silhouette shader

    UnloadTexture(texTree);         // Unload tree texture
    UnloadTexture(texScarfy);       // Unload scarfy texture

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
