# Program flow scratchpads

## Item dowsing

```
u8 bitfield for chosen grass
substates:
    entry: just to set up and draw
    choosing: user can move aroudn and choose grass
    selected: shake users chosen grass
    intermediate: in between choices, display near/far away and check if we should quit
    check correct: check choice, either send to intermediate, give item or quit
    give item: check if inv full and try to give the item
    replace item: prompt user to replace item if inv full
    quitting: quit

init:
    get steps
    get available items
    choose item
    choose position
    set tries left
    set substate choosing

loop:
    entry: set substate choosing
    choosing: nothing
    selected:
        after 4 frames, set substate check correct
    substate check correct:
        decrement choices left
        if item is in chosen:
            set substate give item
        else:
            display nothing found
            set substate choosing
    substate intermediate:
        display near/far
        set substate choosing
    substate give item:
        if inv full:
            set substate replace item
        else:
            give item
            set substate quit
    quit:
        clean up
        set to main menu

draw update:
    substate choosing:
        draw cursor
    substate selected:
        shake grass
    substate replace item:
        redraw item name, cursor

draw init:
    substate first:
        draw everything
    substate check correct:
        set grey grass
        if incorrect:
            draw nothing found
    substate intermediate:
        draw near/far
    substate give item:
        draw item found
    substate replace item:
        redraw all


```
