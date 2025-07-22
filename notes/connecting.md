# Thoughts on connecting nodes

- Need a way to find the nearest element when we click the viewport.
    - So that we can determine whether or not we are dragging an element:
        - Is mouse within radius of position?
    - Also so that if we're dragging an element, we do not drag the viewport.
- Some kind of data structure for quick access by location is called for.
- Should it be a hash table or a tree?
- OK, it should not be a quad tree, even though that is a really cool sounding data structure, damn.
- I will implement brute force for now.
- After that, it could be really intuitive to use the same grid cells in a spatial hash grid.