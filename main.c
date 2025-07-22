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
#include <stdlib.h>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

/*****************************************************************************
 * Macros and Constants
 *****************************************************************************/
#define LOG(X) printf("%s: %d", __FILE__, __LINE__, X)
#define BORDER_MARGIN 10
#define MOUSE_ACTIVE_CLOCK_TICKS 15
#define MAX_OBJECTS 256
#define MINOR_GRIDLINE_DISTANCE 100

/*****************************************************************************
 * Structs and Typedefs
 *****************************************************************************/

struct viewport {
    int y;
    int x;
    int w;
    int h;
};

enum objectType {
    DOT,
    LINE,
    SPRITE
};

struct object {

    enum objectType type;

    // sticky = 1: Should be drawn at edge when offscreen. 
    // sticky = 0: Hidden when offscreen.
    int sticky: 1;

    // A label that can be drawn next to this object
    const char * label;

    // TODO: Generalize to more than circles!
    // Circle radius.
    int radius;

    // Line destination
    int destY, destX;

    // TODO: Implement textures, etc.
    Color color;

    // Object world position with origin at top left
    int y, x;
};

/*****************************************************************************
 * Global Variables
 *****************************************************************************/
RenderTexture rt;
Texture2D skull;
struct viewport vp;

const char* someResource = 0;

// The point clicked to drag the viewport.
Vector2 mouseDragPoint;
Vector2 dragViewportFrom;
clock_t lastMouseActivity;
clock_t currentTime;
int mouseMoving = 0;

// Objects array
struct object objs[MAX_OBJECTS];
int objsLen = 0;

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
 * Adds an object to the object array and returns its index.
*/
int createObject(struct object newObj) {
    objs[objsLen] = newObj;
    objsLen++;
    return objsLen - 1;
}

/**
 * Returns 1 if the given world position is visible
 * in the current viewport. 
*/
int positionVisible(int y, int x) {
    int yInRange = y >= 0 && y <= vp.y + vp.h;
    int xInRange = x >= 0 && x <= vp.x + vp.w;
    return yInRange && xInRange;
}

/**
 * Converts a Position Y coordinate to a screen Y coordinate.
*/
int clampProjectY(int positionY, int clamp) {
    // Case: positionY is above the screen => clamp to top border
    if (positionY < vp.y) {
        return clamp ? GetScreenHeight() - BORDER_MARGIN : -1000;
    }
    // Case: positionY is below the screen => clamp to bottom border
    else if (positionY > vp.y + vp.h) {
        return clamp ? BORDER_MARGIN : -1000;
    }
    // Case: positionY is on screen
    else {
        return GetScreenHeight() - (positionY - vp.y);
    }
}

/**
 * Converts a Position X coordinate to a screen X coordinate.
*/
int clampProjectX(int positionX, int clamp) {
    // Case: positionX is left of the screen => clamp to left border
    if (positionX < vp.x) {
        return clamp ? BORDER_MARGIN : -1000;
    }
    // Case: positionX is right of the screen => clamp to right border
    else if (positionX > vp.x + vp.w) {
        return clamp ? vp.x + vp.w - BORDER_MARGIN : -1000;
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

    // Set drag point when LMB is pressed
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouseDragPoint = GetMousePosition();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        char * label = calloc(32, sizeof(char));
        asprintf(&label, "Object %d", objsLen + 1);
        struct object dot = {
            .type = DOT,
            .sticky = 0,
            .label= label,
            .radius = 10,
            .color = CLITERAL(Color){10 + rand() % 245, 10 + rand() % 245, 10 + rand() % 245, 255},
            .y = vp.y + GetMouseY(),
            .x = vp.x + GetMouseX(),
        }; 
        createObject(dot);
    }



    // Move viewport relative to drag point while LMB is down and moving
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouseDragPoint = GetMousePosition();
        dragViewportFrom.y = vp.y;
        dragViewportFrom.x = vp.x;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        lastMouseActivity = clock() / (1000);
        vp.x = (dragViewportFrom.x - GetMouseX() + mouseDragPoint.x);
        vp.y = (dragViewportFrom.y - GetMouseY() + mouseDragPoint.y);
    }
}

void drawObjects() {
    struct object curr;

    for (int i = 0; i < objsLen; i++) {
        curr = objs[i];
        switch (curr.type) {
            case DOT:
                DrawCircle(
                    clampProjectX(curr.x, curr.sticky), 
                    clampProjectY(curr.y, curr.sticky), 
                    curr.radius, 
                    curr.color
                );
                break;
            case SPRITE:
                // void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
                Vector2 pos;
                pos.x = clampProjectX(curr.x, curr.sticky); 
                pos.y = clampProjectY(curr.y, curr.sticky); 
                DrawTextureEx(skull, pos, 180.0, 1.0, curr.color);
                break;
            case LINE:
                if (positionVisible(curr.destY, curr.destX)) {
                    DrawLine(
                        clampProjectX(curr.x, curr.sticky), 
                        clampProjectY(curr.y, curr.sticky), 
                        clampProjectX(curr.destX, 0), 
                        clampProjectY(curr.destY, 0), 
                        curr.color
                    );
                }
                break;
        }

    }

    
}

void drawLabels() {
    struct object curr;

    for (int i = 0; i < objsLen; i++) {
        curr = objs[i];
        DrawText(
            curr.label,
            clampProjectX(curr.x, curr.sticky) + curr.radius + 5, 
            vp.h - (clampProjectY(curr.y, curr.sticky) + curr.radius + 5),
            20,
            BLACK
        );
    }

}

void drawGridlines() {
    int offsetX = vp.x % MINOR_GRIDLINE_DISTANCE;
    int offsetY = vp.y % MINOR_GRIDLINE_DISTANCE;

    const int shade = 220;

    // Vertical lines
    for (int i = 0; i < vp.w / 10; i++) {
        DrawLine(
            i * MINOR_GRIDLINE_DISTANCE - offsetX, 
            0, 
            i * MINOR_GRIDLINE_DISTANCE - offsetX, 
            vp.h, 
            CLITERAL(Color){shade, shade, shade, 255}
        );
    }

    // FIXME: why do the horizontal lines appear to slide around
    // Horizontal lines
    for (int i = 0; i < vp.h / 10; i++) {
        DrawLine(
            0, 
            i * MINOR_GRIDLINE_DISTANCE + offsetY, 
            vp.w, 
            i * MINOR_GRIDLINE_DISTANCE + offsetY, 
            CLITERAL(Color){shade, shade, shade, 255});
    }

}

void gameLoop() {

    currentTime = clock() / (1000);
    mouseMoving = currentTime - lastMouseActivity < MOUSE_ACTIVE_CLOCK_TICKS;

    handleInput();

    // Drawing to render texture
    BeginDrawing();
    BeginTextureMode(rt);
    ClearBackground(CLITERAL(Color){255, 255, 255, 255});
    drawGridlines();
    drawObjects();
    EndTextureMode();
    DrawTexture(rt.texture, 0, 0, CLITERAL(Color){255,255,255,255});
    DrawFPS(0, 0);
    #ifdef __EMSCRIPTEN__
        char* text = idbfs_get("file.txt");
        DrawText(TextFormat("Dynamic file content: %s", text), 0, 30, 20, WHITE);
    #endif
    DrawText(TextFormat("viewport: (%d, %d); %d objects", vp.y, vp.x, objsLen), 0, 20, 20, BLACK);
    DrawText(TextFormat("drag pt: (%f, %f)", mouseDragPoint.y, mouseDragPoint.x), 0, 40, 20, BLACK);
    DrawText(TextFormat("mouse last moving: %d curr time: %d", lastMouseActivity, currentTime), 0, 60, 20, BLACK);
    drawLabels();
    if (mouseMoving)
        DrawText("Mouse moving", 300, 20, 20, BLUE);
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
    skull = LoadTexture("resources/skull-wenrexa.png");

    struct object skullObj = {
        .type = SPRITE,
        .sticky = 0,
        .label = "Evil skull",
        .color = BLACK,
        .y = 200,
        .x = 300
    };
    createObject(skullObj);

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

    for (int i = 0; i < objsLen; i++) {
        // free(objs[i].label);
    }

    return 0;
}
