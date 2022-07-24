# janet-termios

A basic wrapper library for certain functions found in <termios.h>. Used as a dependency in [Joule editor](https://www.github.com/CFiggers/joule-editor).

# Getting Started

Requires [Janet](https://www.github.com/janet-lang/janet) and [JPM](https://www.github.com/janet-lang/jpm).

1. Clone this repo (for e.g., with the GitHub CLI, `$ gh repo clone CFiggers/janet-termios`.)

2. Change directories into the cloned repo: `$ cd janet-termios`

3. Build the shared library by running `$ jpm build`.

4. Verify correct function by running `$ jpm test`.

5. To make `janet-termios` available as a Janet dependency anywhere on your system, run `$ jpm install` (on Ubuntu and similar systems, may required elevated permissions, e.g. `$ sudo jpm install`)

Now janet-termios can be imported at the Janet REPL or in a Janet source file using `(import janet-termios)`.

Alternately, just follow the Getting Started instructions in [Joule editor](https://www.github.com/CFiggers/joule-editor). This library will be automatically fetched, built, and installed during the `$ jpm deps` step.
