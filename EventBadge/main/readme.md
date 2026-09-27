# Badge + Touch Isolation Test

This build deliberately returns to the known-good badge renderer.

Startup order:

1. Read `/badge.txt`.
2. Initialize the known-good EPD display code.
3. Draw the known-good badge.
4. Only after the badge refresh returns, initialize the GT911.
5. Enter a loop that only prints touch and IO48 activity.

Nothing in the loop changes or refreshes the display.

The touch polling follows the simple pattern used by the LilyGo demo:

- `isPressed()`
- `getPoint(&x, &y, 1)`

There is no custom touch latch inside the Touch class.

Expected result:

- The known-good badge remains visible.
- Repeated independent touches produce repeated `Touch down` and `Touch released` messages.
- IO48 produces press and release messages.
- The display never changes.

If this passes, touch and the known-good EPD path can coexist and the next build can add exactly one screen transition.
