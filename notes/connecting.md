# Thoughts on dragging and connecting nodes

- Need a way to find the nearest element when we click the viewport.
    - So that we can determine whether or not we are dragging an element:
        - Is mouse within radius of position?
    - Also so that if we're dragging an element, we do not drag the viewport.
- Some kind of data structure for quick access by location is called for.
- Should it be a hash table or a tree?
- OK, it should not be a quad tree, even though that is a really cool sounding data structure, damn.
- I will implement brute force for now.
- After that, it could be really intuitive to use the same grid cells in a spatial hash grid.
- But with < 50 nodes it might not even be worth it, lol.
- Next is to implement the node dragging while mouse is down.
- Should possibly track whether we're dragging viewport or an object with another global.
- Then on to the behaviour of connecting nodes with lines.
- OK, it's a few days later and I have nodes connecting fine.
- However, the behaviour for drawing the line should be tweaked so that it always goes in the same direction even when a node is offscreen.
- It is also crucial to get screen size checks out of everything related to viewport so that there can be other components on screen beside viewport eventually.