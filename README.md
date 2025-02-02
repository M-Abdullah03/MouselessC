# Mouseless

Mouseless is a C++ project that provides an overlay for keyboard-based mouse control mainly for Windows. Inspired by the Mac Version by the same name by [croian](https://github.com/croian)

## Features

- Keyboard-based mouse control
- Drag and drop functionality
- Customizable keyboard layout

## Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/M-Abdullah03/mouseless.git
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

### Overlay Actions

- To **show the overlay**, `Hold` the `Alt` and press backtick ` key
- To **hide the overlay**, `Hold` the `Alt` and press backtick ` key again

### Mouse actions
### Mouse Actions

- To **click**:
  - While the overlay is visible, type the two characters of any cell to select it.
  - Press `Space` to click at the center of the cell, where the green cursor is located.
    - Alternatively, press any character in a sub-cell to click at that sub-cell.
  - You can also press `Space` before entering any characters, or after entering one, to click where the cursor is.

- To **right-click**, hold the `Alt` key while pressing the final key of a click action.

- To **double-click or triple-click**, press the final key of a click action multiple times within the `CLICK_DELAY_THRESHOLD` (default 300ms).

- To **click-and-drag / drag-and-drop**, use the `drag` action:
  - Hold the `Shift` key while pressing the final key to begin the drag.
  - The overlay will remain visible for you to choose the point to drag to or drop at.
  - To release/drop, enter a click-coordinate as normal.
  - To drag to a point (without releasing/dropping), hold the `Shift` key on the final press again.

### Multiple Monitors (Under Development)
 
- To **move the overlay between monitors**, tap the `PageUp` or `PageDown` keys while the overlay is visible.
