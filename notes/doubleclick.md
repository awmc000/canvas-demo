# Straightening out issues with double click
- isDoubleClick() needs to run when `prevMouseActivity` and `lastMouseActivity` are actually different
- too early in input handling, they are not current
- too late in input handling, they are the same (prev has been set to last)

## TODOs for july 24
- fix this double click bullshit
    - ok, that was actually really easy
- then use double click to open node label edit popup
- fix issue with backspace in string being populated by edit popup input