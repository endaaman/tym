# tym

[![CircleCI](https://circleci.com/gh/endaaman/tym.svg?style=svg)](https://circleci.com/gh/endaaman/tym)
[![Gitter chat](https://badges.gitter.im/tym-terminal/gitter.png)](https://gitter.im/tym-terminal/Lobby)

`tym` is a tiny VTE-based terminal emulator, which configurable by Lua.

## Motivation

I wanted a terminal emulator that is

- simple (no menu bar, no GUI setting window etc..)
- flexiblly configurable by text
- original VTE-based (not a forked one like 'vte-ng')

but I could not find it so I made it.

## Dependencies

- [GTK+3](https://www.gtk.org/)
- [VTE](https://github.com/GNOME/vte)
- [Lua](https://www.lua.org/)

## Configration

When `$XDG_CONFIG_HOME/tym/config.lua` exists, it is executed.

```lua
-- At first you need to require tym module
local tym = require('tym')

-- set config individually
tym.set('width', 100)

-- set by table
tym.set_config({
  shell = '/bin/bash',
  cursor = 'underline',
  autohide = true,
})
```

All available options are shown below.

| field name | type | default value | description |
| --- | --- | --- | --- |
| `title` | string | `'tym'` | Window title. |
| `shell` | string | `$SHELL` -> `vte_get_user_shell()` -> `/bin/sh` | Shell to excute. |
| `font` | string | `''` (empty string) | You can specify it like `'FAMILY-LIST [SIZE]'`, for example `'Ubuntu Mono 12'`. The value specified here is internally passed to [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If you set empty string, the system default fixed width font will be used. |
| `icon` | string | `'terminal'` | Name of icon. cf. [Icon Naming Specification](https://developer.gnome.org/icon-naming-spec/) |
| `cursor_shape` | string | `'block'` | `'block'`, `'ibeam'` or `'underline'` are available. |
| `cursor_blink_mode` | string | `'system'` | `'system'`, `'on'` or `'off'` are available. |
| `term` | string | `'xterm-256color'` | Default value of `$TERM`. |
| `role` | string | `''` | Unique identifier for the window. If empty string set, no value set. (cf. [gtk_window_set_role()](https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-set-role)) |
| `cjk_width` | string | `'narrow'` | `'narrow'` or `'wide'` are available. There are complicated problems about this, so if you are not familiar with it, it's better to use the default. |
| `width` | integer | `80` | Initial columns. |
| `height` | integer | `22` | Initial rows. |
| `ignore_default_keymap` | boolean | `false` | Whether to use default keymap. |
| `ignore_bold` | boolean | `false` | Whether to allow drawing bold text(cf. [vte-terminal-set-allow-bold()](https://developer.gnome.org/vte/unstable/VteTerminal.html#vte-terminal-set-allow-bold)). |
| `autohie` | boolean | `false` | Whether to hide mouse cursor when the user presses a key. |
| `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground`, `color_bold`, `color_0` ... `color_15` | string | `''` (empty string) | You can specify standard color string, for example `'#f00'`, `'#ff0000'` or `'red'`. These will be parsed with [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If you set empty string, the VTE default color will be used. |


## Color scheming

Please refer to the correspondence table of each color below.

```
color_0  : black (background)
color_1  : red
color_2  : green
color_3  : brown
color_4  : blue
color_5  : purple
color_6  : cyan
color_7  : light gray (foreground)
color_8  : gray
color_9  : light red
color_10 : light green
color_11 : yellow
color_12 : light blue
color_13 : pink
color_14 : light cyan
color_15 : white
```

## Keymap

### Default

| Key             | Action                       |
| :-------------- | :--------------------------- |
| Ctrl Shift c    | Copy selection to clipboard. |
| Ctrl Shift v    | Paste from clipboard.        |
| Ctrl Shift r    | Reload config file.          |
| Ctrl +          | Increase font scale,         |
| Ctrl -          | Decrease font scale.         |
| Ctrl =          | Reset font scale.            |

### Customize

You can register functions in a table named `keymap` (defaultly defined like `config` table) in a format parsable by [gtk_accelerator_parse()](https://developer.gnome.org/gtk3/stable/gtk3-Keyboard-Accelerators.html#gtk-accelerator-parse).

```lua

-- also can set keymap
tym.set_keymap('<Ctrl><Shift>o', function()
  local h = tym.get('height')
  tym.set('height', h + 1)
  tym.apply() -- needed for applying config value
  tym.notify('Set window height :' .. h)
end)

-- set by table
tym.set_keymaps({
  ['<Ctrl><Shift>y'] = function()
    tym.reload()
    tym.notify('reload config')
  end,
  ['<Ctrl><Shift>v'] = function()
    tym.notify("Overwrite pasting event")
  end,
})

-- disable default keymap
keymap['<Ctrl>='] = 0
```

## Lua API

| name                                 | return value   | description                                     |
| ------------------------------------ | -------------- | ----------------------------------------------- |
| `tym.get_version()`                  | string         | Get version string.                             |
| `tym.get_config_file_path()`         | string         | Get path of config file currently being read.   |
| `tym.notify(message, title = 'tym')` | void           | Show desktop notification.                      |
| `tym.put(text)`                      | void           | Feed text.                                      |
| `tym.reload()`                       | void           | Reload config file.                             |
| `tym.copy()`                         | void           | Copy current selection.                         |
| `tym.paste()`                        | void           | Paste clipboard.                                |
| `tym.increase_font_scale()`          | void           | Increase font scale.                            |
| `tym.decrease_font_scale()`          | void           | Decrease font scale.                            |
| `tym.reset_font_scale()`             | void           | Reset font scale.                               |


## Options

### `--help` `-h`

You can find all options.

```
$ tym -h
```

### `--use` `-u`

Pass `<path>` you want to execute as config.

```
$ tym --use=/path/to/config.lua
```

Pass `NONE` to start with default config.

```
$ tym -u NONE
```

### `--theme` `-t`

Pass `<path>` you want to load as theme.

```
$ tym --use=/path/to/config.lua
```

Pass `NONE` to start with default config same as `--use`.

```
$ tym -t NONE
```

### `--<config option>`

You can pass config value via command line option.

```
$ tym --shell=/bin/bash --color_background=red --width=40 --ignore_default_keymap
```

## Compilation

Download source code from [release page](https://github.com/endaaman/tym/releases), unarchive it and do

```
$ ./configure
$ make
```

### Build dependencies

#### Arch Linux

```
$ sudo pacman -S lua vte3
```

#### Ubuntu

```
$ sudo apt install libgtk-3-dev libvte-2.91-dev liblua5.3-dev
```

#### Other distros or OS

I did not check which packeges are needed to build on other distros or OS. Awaiting your contribution ;)

## Installation

#### Arch Linux

```
$ yay -S tym
```

#### Other distros or OS

Just compile and do

```
$ sudo make install
```

## Development

Clone this repo and do

```
$ autoreconf -fvi
$ ./configure --enable-debug
```

## Tips

If you want to know more about configuration, see my [config.lua](https://github.com/endaaman/dotfiles/blob/master/tym/config.lua)

## License

MIT
