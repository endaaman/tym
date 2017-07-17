# tym

`tym` is a tiny VTE-based terminal emulator, which is written in C and configurable by lua.

## Motivation

[`termite`](https://github.com/thestinger/termite) is a very simple and wonderful terminal emulator. Unfortunately `vte-ng` on which terminite depends conflicts with `vte`, making it difficult to coexist with `GNOME Terminal` and other `vte` dependents.

In the first place the feature that causes conflicts is not needed for me because tmux has similar one (copy mode). So I wanted a terminal emulator that is

- Extremely simple (no menu bar, GUI setting window etc..)
- original VTE-based
- configurable by text file

but I could not find it, so I made it.

## Dependencies

- [GTK+3](https://www.gtk.org/)
- [VTE](https://github.com/GNOME/vte)
- [lua](https://www.lua.org/)

## Configration

When `$XDG_CONFIG_HOME/tym/config.lua` exists, it is executed with the global table variable `config` defined. You can do configuration by modifying `config`. For example you can write for

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

config.color_0  = '#181818' -- overwritten by color_background
config.color_1  = '#ac4142'
config.color_2  = '#90a959'
config.color_3  = '#f4bf75'

-- SNIP

config.color_7  = '#d0d0d0' -- overwritten by color_foreground

-- SNIP
```

All available options are shown below.

| field name | type | default value | description |
|---------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|-------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `width` | integer | `80` | Initial columns. |
| `height` | integer | `22` | Initial rows. |
| `title` | string | `'tym'` | Window title |
| `shell`  | string | `$SHELL` -> `vte_get_user_shell()` ->  `/bin/sh` | Shell to excute |
| `font` | string | `''` (empty string) | You can specify it like `'FAMILY-LIST [SIZE]'`, for example `'Ubuntu Mono 12'`. The value specified here is internally passed to [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If you set empty string, the system default fixed width font will be used. |
| `cursor_blink` | string | `'system'` | `'system'`, `'on'` or `'off'` are available. |
| `cjk_width` | string | `'narrow'` | `'narrow'` or `'wide'` are available. There are complicated problems about this, so if you are not familiar with it, it's better to use the default. |
| `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground`, `color_0` ... `color_255` | string | `''` (empty string) | You can specify standard color string, for example `'#f00'`, `'#ff0000'` or `'red'`. These will be parsed with [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If you set empty string, the VTE default color will be used. |

## Key bindings

| Key            | Action                      |
|:-------------- |:--------------------------- |
| Ctrl Shift c   | Copy selection to clipboard |
| Ctrl Shift v   | Paste from clipboard        |
| Ctrl Shift r   | Reload config file          |
| Ctrl -         | Decrease font scale         |
| Ctrl +         | Increase font scale         |
| Ctrl =         | Reset font scale            |

## Compilation

Download source code from [release page](https://github.com/endaaman/tym/releases), unarchive it and do

```console
$ ./configure
$ make
$ sudo make install   // if you want to intall
```

Check [wiki](https://github.com/endaaman/tym/wiki) for build dependencies in other distros.

## Installation

If you are an Arch linux user, just do

```console
$ yaourt -S tym
```

## Development

Clone and do

```console
$ autoreconf -fvi
```

## License

MIT
