/**
 * Canvas Demo
 * 
 * awmc2000
 * 
 * started July 16th, 2025
 * 
 * Primay goal: Create a 2d Cartesian plane with a viewport you can move around.
 * Not unlike what goes on when you pan around Google Maps or a graphing
 * calculator tool like Desmos or Geogebra. Should support placing points and
 * connecting them with lines when completed.
*/
#include <raylib.h>
#include <stdio.h>
#include <rcamera.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

/*****************************************************************************
 * Macros and Constants
 *****************************************************************************/
#define LOG(X) printf("%s: %d", __FILE__, __LINE__, X)
#define BORDER_MARGIN 10

/*****************************************************************************
 * Structs and Typedefs
 *****************************************************************************/

struct viewport {
    int y;
    int x;
    int w;
    int h;
};

struct object {

    // sticky = 1: Should be drawn at edge when offscreen. 
    // sticky = 0: Hidden when offscreen.
    int sticky: 1;

    // A label that can be drawn next to this object
    char * label;

    // TODO: Generalize to more than circles!
    // Circle radius.
    int radius;

    // TODO: Implement textures, etc.
    Color color;

    // Object world position with origin at top left
    int y, x;
};

/*****************************************************************************
 * Global Variables
 *****************************************************************************/
RenderTexture rt;
struct viewport vp;

const char* someResource = 0;
Vector2 mouseDragPoint;

#ifdef __EMSCRIPTEN__
    EM_JS(void, idbfs_put, (const char* filename, const char* str), {
        FS.writeFile(UTF8ToString(filename), UTF8ToString(str));
        FS.syncfs(false,function (err) {
            assert(!err);
        });
    });
    EM_JS(char*, idbfs_get, (const char* filename), {
        var arr = FS.readFile(UTF8ToString(filename));
        var jsString = new TextDecoder().decode(arr);
        var lengthBytes = lengthBytesUTF8(jsString)+1;
        //console.log(jsString);
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
    });
#endif

/**
 * Converts a Position Y coordinate to a screen Y coordinate.
*/
int clampProjectY(int positionY) {
    // Case: positionY is above the screen => clamp to top border
    if (positionY < vp.y) {
        return GetScreenHeight() - BORDER_MARGIN;
    }
    // Case: positionY is below the screen => clamp to bottom border
    else if (positionY > vp.y + vp.h) {
        return BORDER_MARGIN;
    }
    // Case: positionY is on screen
    else {
        return GetScreenHeight() - (positionY - vp.y);
    }
}

/**
 * Converts a Position X coordinate to a screen X coordinate.
*/
int clampProjectX(int positionX) {
    // Case: positionX is left of the screen => clamp to left border
    if (positionX < vp.x) {
        return BORDER_MARGIN;
    }
    // Case: positionX is right of the screen => clamp to right border
    else if (positionX > vp.x + vp.w) {
        return vp.x + vp.w - BORDER_MARGIN;
    }
    // Case: positionX is on screen
    else {
        return positionX - vp.x;
    }
}

void handleInput() {
    if (IsKeyDown(KEY_W) && vp.y > 0) vp.y--;
    if (IsKeyDown(KEY_S)) vp.y++;
    if (IsKeyDown(KEY_A) && vp.x > 0) vp.x--;
    if (IsKeyDown(KEY_D)) vp.x++;
}

void drawObjects() {
    // DrawCircle(
    //     GetMouseX(), 
    //     GetScreenHeight() - GetMouseY(), 
    //     50, 
    //     CLITERAL(Color){0,0,0,255}
    // );
    DrawCircle(
        clampProjectX(50), 
        clampProjectY(100), 
        10, 
        CLITERAL(Color){10,10,200,255}
    );
    DrawLine(
        clampProjectX(50), 
        clampProjectY(100),
        clampProjectX(1000), 
        clampProjectY(1000),
        CLITERAL(Color){20,10,10,255}
    );
    DrawCircle(
        clampProjectX(1000), 
        clampProjectY(1000), 
        10, 
        CLITERAL(Color){200,10,20,255}
    );
}

void gameLoop() {

    handleInput();

    // Drawing to render texture
    BeginDrawing();
    BeginTextureMode(rt);
    ClearBackground(CLITERAL(Color){255, 255, 255, 255});
    drawObjects();
    EndTextureMode();
    DrawTexture(rt.texture, 0, 0, CLITERAL(Color){255,255,255,255});
    DrawFPS(0, 0);
    #ifdef __EMSCRIPTEN__
        char* text = idbfs_get("file.txt");
        DrawText(TextFormat("Dynamic file content: %s", text), 0, 30, 20, WHITE);
    #endif
    DrawText(TextFormat("Static file content: %s", someResource), 0, 60, 20, BLACK);
    DrawText(TextFormat("(%d, %d)", vp.y, vp.x), 100, 0, 20, BLACK);
    EndDrawing();
}

int main() {
    int screenWidth = 960;
    int screenHeight = 480;

    vp.y = 500;
    vp.x = 500;
    vp.w = screenWidth;
    vp.h = screenHeight;

    InitWindow(screenWidth, screenHeight, "rtextures");
    SetTargetFPS(144);
    rt = LoadRenderTexture(screenWidth, screenHeight);
    someResource = LoadFileText("resources/dummy.txt");

    #ifdef __EMSCRIPTEN__
        EM_ASM(
            FS.mkdir('/work');
            FS.mount(IDBFS, {}, '/work');
            FS.syncfs(true, function (err){
                assert(!err);
            });
        );
        struct timespec ts = {.tv_sec = 0,  .tv_nsec = 100000000};
        struct timespec ts2 = {.tv_sec = 0, .tv_nsec = 100000000};
        nanosleep(&ts, &ts2);
        idbfs_put("file.txt", "Some dynamic file contents...\n");
        emscripten_set_main_loop(gameLoop, 0, 0);
    #else
        while(!WindowShouldClose()){
            gameLoop();
        }
    #endif
    return 0;
}
