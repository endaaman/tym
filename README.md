# tym

[![CircleCI](https://circleci.com/gh/endaaman/tym.svg?style=svg)](https://circleci.com/gh/endaaman/tym)

`tym` is a tiny VTE-based terminal emulator, which is written in C and configurable by Lua.

## Motivation

I wanted a teminal emulator that is

- simple (no menu bar, no GUI setting window etc..)
- text based configuration
- original VTE-based (not a forked one like vte-ng)

but I could not find it so I made it.

## Dependencies

- [GTK+3](https://www.gtk.org/)
- [VTE](https://github.com/GNOME/vte)
- [Lua](https://www.lua.org/)

## Configration

When `$XDG_CONFIG_HOME/tym/config.lua` exists, it is executed with the global table `config` defined. You can do configuration by modifying `config` table. For example you can write for

```lua
config.height = 30
config.width = 100

config.shell = '/bin/fish'
config.font = 'DejaVu Sans Mono 10'
config.cursor_blink_mode = 'on'
config.cjk_width = 'narrow'

config.color_foreground        = '#d0d0d0'
config.color_foreground_bold   = '#d0d0d0'
config.color_cursor            = '#d0d0d0'
config.color_cursor_foreground = '#181818'
config.color_background        = '#181818'

config.color_0  = '#181818'
config.color_1  = '#ac4142'
config.color_2  = '#90a959'
config.color_3  = '#f4bf75'

-- SNIP
```

All available options are shown below.

| field name | type | default value | description |
| --- | --- | --- | --- |
| `width` | integer | `80` | Initial columns. |
| `height` | integer | `22` | Initial rows. |
| `title` | string | `'tym'` | Window title |
| `shell` | string | `$SHELL` -> `vte_get_user_shell()` -> `/bin/sh` | Shell to excute |
| `font` | string | `''` (empty string) | You can specify it like `'FAMILY-LIST [SIZE]'`, for example `'Ubuntu Mono 12'`. The value specified here is internally passed to [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If you set empty string, the system default fixed width font will be used. |
| `cursor_blink` | string | `'system'` | `'system'`, `'on'` or `'off'` are available. |
| `cjk_width` | string | `'narrow'` | `'narrow'` or `'wide'` are available. There are complicated problems about this, so if you are not familiar with it, it's better to use the default. |
| `use_default_keymap` | boolean | `true` | Use default keymap or not |
| `allow_bold_font` | boolean | `true` | Whether or not to attempt to draw bold text |
| `color_bold`, `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground`, `color_0` ... `color_15` | string | `''` (empty string) | You can specify standard color string, for example `'#f00'`, `'#ff0000'` or `'red'`. These will be parsed with [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If you set empty string, the VTE default color will be used. |


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
keymap['<Shift><Ctrl>u'] = function()
  tym.notify('Pressed C-S-u')
end

-- Override default keymap
keymap['<Shift><Ctrl>r'] = function()
  tym.reload()
  tym.notify('Config reloaded')
end

-- Disable a default keymap
-- Note: It must be non-nil value because of Lua language spec that it can
--       not distinguish between no-value registered state and `nil` registered state.
keymap['<Ctrl>='] = 0
```

## Embedded functions

| name                                 | return value   | description                                     |
| ------------------------------------ | -------------- | ----------------------------------------------- |
| `tym.get_version()`                  | string         | Get version string.                             |
| `tym.get_config_file_path()`         | string         | Get path of config file currently being read.   |
| `tym.notify(message, title = 'tym')` | void           | Show desktop notification.                      |
| `tym.put(text)`                      | void           | Feed text.                                      |
| `tym.reload()`                       | void           | Reload config file.                             |
| `tym.copy_clipboard()`               | void           | Copy current serection.                         |
| `tym.paste_clipboard()`              | void           | Paste clipboard.                                |
| `tym.increase_font_scale()`          | void           | Increase font scale.                            |
| `tym.decrease_font_scale()`          | void           | Decrease font scale.                            |
| `tym.reset_font_scale()`             | void           | Reset font scale.                               |


## Options

### `--help` `-h`

Basically, you can find all command line options by this.

```
$ tym -h
```

### `--use` `-u`

You can load preferred config file using `--use, -u`

```
$ tym --use=/path/to/config.lua
```

and also can start without loading default `config.lua`.

```
$ tym -u NONE
```

### `--c:<config option>`

Overriding config option via command line

```
$ tym --c:shell=/bin/bash --c:color_background=red --c:use_default_keymap=false
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

#### Other distro

I don't check packeges needed to build on other distros. Awaiting your comtribution!!

## Installation

If you are an Arch linux user, just do

```
$ yaourt -S tym
```

or compile and do

```
$ sudo make install
```

## Development

Clone and do

```
$ autoreconf -fvi
```

Then you will get `configure` script.

## Tips

If you want to know more about configuration, see my [config.lua](https://github.com/endaaman/dotfiles/blob/master/tym/config.lua)

## License

MIT
