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


// Standard library header includes -------------------------------------------
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Third party header includes ------------------------------------------------
#include <raylib.h>
#include <rcamera.h>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

// Project header includes ----------------------------------------------------
#include "viewport.h"
#include "util.h"

/*****************************************************************************
 * Macros and Constants
 *****************************************************************************/
#define LOG(X) printf("%s: %d", __FILE__, __LINE__, X)
#define MOUSE_ACTIVE_CLOCK_TICKS    15
#define MAX_OBJECTS                 256
#define MINOR_GRIDLINE_DISTANCE     100
#define COLLISION_MARGIN_PX         2
#define TRUE                        1
#define FALSE                       0

/*****************************************************************************
 * Structs and Typedefs
 *****************************************************************************/


enum objectType {
    DOT,
    SPRITE
};

enum dragState {
    VIEWPORT,
    OBJECT
};

struct object {

    enum objectType type;

    // sticky = 1: Should be drawn at edge when offscreen. 
    // sticky = 0: Hidden when offscreen.
    int sticky: 1;

    // A label that can be drawn next to this object
    char * label;

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

// Objects can be dragged and so can the viewport itself.
// In either case, the starting point should be saved for calculations.
Vector2 mouseDragPoint;
Vector2 dragViewportFrom;
Vector2 dragObjectFrom;
clock_t lastMouseActivity;
clock_t currentTime;
int mouseMoving = 0;
enum dragState ds;

// Objects can be connected by right clicking one object and then the other.
// Right clicking where there is not an object will clear both places.
struct object * connectionSource;
struct object * connectionDestination;

// 0 if selecting Source; 1 if selecting Destination.
int connectionSelected = 0;

// Objects array
struct object objs[MAX_OBJECTS];
struct object * recentlyGrabbedObject;
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

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Adds an object to the object array and returns its index.
*/
int createObject(struct object newObj) {
    objs[objsLen] = newObj;
    objsLen++;
    return objsLen - 1;
}

/**
 * Creates an edge between two nodes and returns its index.
*/
int createConnection(struct object * src, struct object * dest) {
    printf("IMPLEMENT: Create a connection between %s at (%d,%d) and %s at (%d,%d)\n",
        connectionSource->label, connectionSource->y, connectionSource->y,
        connectionDestination->label, connectionDestination->y, connectionDestination->y
    );
    return -1;
}

/**
 * Draw a line from source node to cursor. Shows user they're making a 
 * connection.
*/
void drawTempLine(void) {
    if (connectionSource != NULL && connectionDestination == NULL) {
        DrawLine(
            clampProjectX(&vp, connectionSource->x, 0), 
            clampProjectY(&vp, connectionSource->y, 0), 
            GetMouseX(),vp.h - GetMouseY(),
            BLACK
        );
    }
}

/**
 * Determines if the mouse is colliding with an object.
 * Sets the recentlyGrabbedObject as a side effect if it is.
*/
int collidingWithPoint() {
    int my = GetMouseY();
    int mx = GetMouseX();
    
    // Object y, x
    int oy;
    int ox;
    
    // y and x distance
    int yDelta;
    int xDelta;
    int distance;

    for (int i = 0; i < objsLen; i++) {
        // Adjust object coordinates for viewport
        oy = vp.h - clampProjectY(&vp, objs[i].y, 0);
        ox = clampProjectX(&vp, objs[i].x, 0);
        yDelta = abs(oy - my);
        xDelta = abs(ox - mx);

        // Calculate straight line distance with pythagorean theorem:
        // Is it less than the radius of the object?
        distance = sqrt( (yDelta*yDelta) + (xDelta*xDelta) );

        // Check if within that distance allowing a certain margin of error
        if (distance < (objs[i].radius + COLLISION_MARGIN_PX)) {
            recentlyGrabbedObject = &objs[i];
            return TRUE;
        }
    }

    return FALSE;
}

void handleInput() {
    if (IsKeyDown(KEY_W) && vp.y > 0) vp.y--;
    if (IsKeyDown(KEY_S)) vp.y++;
    if (IsKeyDown(KEY_A) && vp.x > 0) vp.x--;
    if (IsKeyDown(KEY_D)) vp.x++;

    // save if we are colliding with a point 
    // so we don't check again in one input frame
    int hittingPoint = collidingWithPoint();

    // Set drag point when LMB is pressed
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (hittingPoint) {
            // TODO: any other drag behaviour
            ds = OBJECT;
        } else {
            ds = VIEWPORT;
        }
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

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        // Set selected object
        if (hittingPoint) {
            if (connectionSelected == 0) {
                connectionSource = recentlyGrabbedObject;
            } else if (connectionSelected == 1) {
                connectionDestination = recentlyGrabbedObject;
                // Create connection
                createConnection(connectionSource, connectionDestination);
                // TODO: Why does this cause a segfault / core dump when triggered?
                // connectionSource = NULL; 
                // connectionDestination = NULL;
                // connectionSelected = 0;
            }
            connectionSelected = connectionSelected ^ 1;
        } else {
            // Clear selected object
            connectionSource = NULL; 
            connectionDestination = NULL;
            connectionSelected = 0;
        }
    }

    // Move viewport relative to drag point while LMB is down and moving
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouseDragPoint = GetMousePosition();
        // Check if we are dragging the viewport or an object
        switch (ds) {
            case VIEWPORT:
                dragViewportFrom.y = vp.y;
                dragViewportFrom.x = vp.x;
                break;
            case OBJECT:
                dragObjectFrom.y = recentlyGrabbedObject->y;
                dragObjectFrom.x = recentlyGrabbedObject->x;
                break;
        }
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        lastMouseActivity = clock() / (1000);
        switch (ds) {
            case VIEWPORT:
                vp.x = (dragViewportFrom.x - GetMouseX() + mouseDragPoint.x);
                vp.y = (dragViewportFrom.y - GetMouseY() + mouseDragPoint.y);
                break;
            case OBJECT:
                recentlyGrabbedObject->x = (dragObjectFrom.x + GetMouseX() - mouseDragPoint.x);
                recentlyGrabbedObject->y = (dragObjectFrom.y + GetMouseY() - mouseDragPoint.y);
                break;
        }

    }
}

void drawObjects() {
    struct object curr;

    for (int i = 0; i < objsLen; i++) {
        curr = objs[i];
        switch (curr.type) {
            case DOT:
                DrawCircle(
                    clampProjectX(&vp, curr.x, curr.sticky), 
                    clampProjectY(&vp, curr.y, curr.sticky), 
                    curr.radius, 
                    curr.color
                );
                break;
            case SPRITE:
                // void DrawTextureEx(Texture2D texture, Vector2 position, 
                //     float rotation, float scale, Color tint);
                Vector2 pos;
                pos.x = clampProjectX(&vp, curr.x, curr.sticky); 
                pos.y = clampProjectY(&vp, curr.y, curr.sticky); 
                DrawTextureEx(skull, pos, 180.0, 1.0, curr.color);
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
            clampProjectX(&vp, curr.x, curr.sticky) + curr.radius + 5, 
            vp.h - (clampProjectY(&vp, curr.y, curr.sticky) + curr.radius + 5),
            20,
            BLACK
        );
    }

}

void drawGridlines() {
    int offsetX = vp.x % MINOR_GRIDLINE_DISTANCE;
    int offsetY = vp.y % MINOR_GRIDLINE_DISTANCE;

    const int shade = 190;

    // Vertical lines
    for (int i = 0; i < vp.w / 10; i++) {
        drawDottedLine(
            i * MINOR_GRIDLINE_DISTANCE - offsetX, 
            0, 
            i * MINOR_GRIDLINE_DISTANCE - offsetX, 
            vp.h, 
            CLITERAL(Color){shade, shade, shade, 255},
            2
        );
    }

    // FIXME: why do the horizontal lines appear to slide around
    // Horizontal lines
    for (int i = 0; i < vp.h / 10; i++) {
        drawDottedLine(
            0, 
            i * MINOR_GRIDLINE_DISTANCE + offsetY, 
            vp.w, 
            i * MINOR_GRIDLINE_DISTANCE + offsetY, 
            CLITERAL(Color){shade, shade, shade, 255},
            2
        );
    }

}

void printDebugInfo() {
    DrawText(
        TextFormat("vp: (%d, %d); %d objects", vp.y, vp.x, objsLen),
        0, 20, 20, BLACK
    );

    DrawText(
        TextFormat("drag pt: (%f, %f)", mouseDragPoint.y, mouseDragPoint.x),
        0, 40, 20, BLACK
    );

    DrawText(
        TextFormat(
            "mouse last moving: %d curr time: %d",
            lastMouseActivity, currentTime
        ),
        0, 60, 20, BLACK
    );

    DrawText(
        TextFormat(
            "Object last touched: %s at y,x (%d,%d)",
            recentlyGrabbedObject == NULL ? "null" : (*recentlyGrabbedObject).label,
            recentlyGrabbedObject == NULL ? -1 : (*recentlyGrabbedObject).y,
            recentlyGrabbedObject == NULL ? -1 : (*recentlyGrabbedObject).x
        ),
        0, 80, 20, BLACK
    );

    DrawText(
        TextFormat(
            "Picking: %d Src: %s Dest: %s",
            connectionSelected,
            connectionSource == NULL ? "null" : connectionSource->label,
            connectionDestination == NULL ? "null" : connectionDestination->label
        ),
        0, 100, 20, BLACK
    );
}

/*****************************************************************************
 * Game Loop
 *****************************************************************************/

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
    drawTempLine();
    EndTextureMode();
    DrawTexture(rt.texture, 0, 0, CLITERAL(Color){255,255,255,255});
    DrawFPS(0, 0);
    #ifdef __EMSCRIPTEN__
        char* text = idbfs_get("file.txt");
        DrawText(TextFormat("Dynamic file content: %s", text), 0, 30, 20, WHITE);
    #endif
    printDebugInfo();
    drawLabels();
    if (mouseMoving)
        DrawText("Mouse moving", 300, 20, 20, BLUE);
    EndDrawing();
}

/*****************************************************************************
 * Main Function (Point of Entry)
 *****************************************************************************/

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

    srand(time(NULL));

    struct object skullObj = {
        .type = SPRITE,
        .sticky = 0,
        .label = strdup("Evil skull"),
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
        if (objs[i].label != NULL) {
            free(objs[i].label);
        }
    }

    return 0;
}
