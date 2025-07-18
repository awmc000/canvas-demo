# Implementing drag UI control like Google Maps

## First attempt

```c
void handleInput() {
    // --- rest of function snipped ---

    // Set drag point when LMB is pressed
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouseDragPoint = GetMousePosition();
    }

    // Move viewport relative to drag point while LMB is down (TODO: and moving!)
    Vector2 cursorPosition = GetMousePosition();
    Vector2 delta;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (cursorPosition.y > mouseDragPoint.y && vp.y > 0) vp.y--;
        if (cursorPosition.y < mouseDragPoint.y) vp.y++;
        if (cursorPosition.x > mouseDragPoint.x && vp.x > 0) vp.x--;
        if (cursorPosition.x < mouseDragPoint.x) vp.x++;
    }
}
```

Issues:
- Keeps moving when the mouse has stopped moving but is still pressed
- Doesn't really have the speed or directionality feeling right
- Feels janky

## Example in JS that looks and feels great

https://jsfiddle.net/vzc9brka/3/

```js
var _startX = 0; // drag point in my code
var _startY = 0;
var _offsetX = 0;
var _offsetY = 0;
var _dragElement;
document.onmousedown = OnMouseDown;
document.onmouseup = OnMouseUp;

function OnMouseDown(event){
  document.onmousemove = OnMouseMove;
    _startX = event.clientX;
  _startY = event.clientY;
  _offsetX = document.getElementById('div1').offsetLeft;
  _offsetY = document.getElementById('div1').offsetTop;
  _dragElement = document.getElementById('div1');

}

function OnMouseMove(event){
    _dragElement.style.left = (_offsetX + event.clientX - _startX) + 'px';
  _dragElement.style.top = (_offsetY + event.clientY - _startY) + 'px';
}

function OnMouseUp(event){
  document.onmousemove = null;
  _dragElement=null;
}
```