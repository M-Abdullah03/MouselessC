# Mouseless

Mouseless is a C++ project that provides an overlay for keyboard-based mouse control.

## Features

- Keyboard-based mouse control
- Drag and drop functionality
- Customizable keyboard layout

## Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/yourusername/mouseless.git
   cd mouseless
   ```
2. Build the project:
   ```sh 
    make
    ```
3. Run the executable:
    ```sh
    ./mouseless
    ```

### The overlay

- To **show the overlay**, `Hold` the `Alt` and press backtick ` key
- To **hide the overlay**, `Hold` the `Alt` and press backtick ` key again

### Mouse actions

- To **click**:
  - while the overlay is showing, type the two characters of any given cell to choose that cell
  - then press `Space` to click at the center of the cell, where the little green cursor is
    - OR press any character you see in a sub-cell to click at that sub-cell
  - you can also press space before entering any characters, or after entering one, to click where the cursor is

- To **right click**, hold the `Alt` key while pressing the final key of a click action

- To **double-click or triple-click**, either:
  - press the final key of a click action multiple times within `CLICK_DELAY_THRESHOLD` threshold (default 300ms)

- To **click-and-drag / drag-and-drop**, use the `hold for drag` action, i.e.:
  - hold the `Shift` key while pressing the final key to begin the drag
  - the overlay will remain up for you to choose the point you want to drag to or drop at
  - to release/drop, enter in a click-coordinate as normal
  - to drag to a point (without release/drop), hold the `Shift` key on the final press again

- To simply **move the mouse cursor**, hold `Ctrl` during the final keypress of a a mouse action.

### Multiple monitors

- To **move the overlay between monitors**, `tap` the `ShiftLeft` or `ShiftRight` keys while the overlay is visible.

### Menu/gui actions

- To **open the config editor**, press the `Tab` key while the overlay is up
- To **close dialogs**, press the the `AltRight` key
