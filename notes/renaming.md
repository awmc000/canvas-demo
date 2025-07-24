# Renaming nodes will bring in an interesting new UI pattern
- Right click on a node and an overlay will grey out the screen while you pick a new name.
- Then hit enter or click on the checkmark and the object's new name is saved and the buffer is cleared.
- Also, a colour picker would be sweet.

## TODOs on July 23rd
- Just make the labels char arrays instead of this calloc business that already led to core dumps
- Create a `drawing` module that contains the drawing functions
- Break up the input function which is a monster atp
- Fix visual glitching when one vertex in a connection is offscreen clamped