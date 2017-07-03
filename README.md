# tym

`tym` is a tiny VTE-based terminal emulator, which is written in C and configurable by lua.

## Motivation

[`termite`](https://github.com/thestinger/termite) is very good but `vte-ng` is conflict with `vte` and its selection mode feature is not needed for me because tmux has almost same (or a bit better) one. I wanted a terminal emulator that is

- original VTE-based
- configurable with text file, which can be managed by personal so called `dotfiles` repository like [this](https://github.com/endaaman/dotfiles)

but such a terminal did not exist so I created.

## Dependencies

- [GTK+3](https://www.gtk.org/)
- [VTE](https://github.com/GNOME/vte)
- [lua](https://www.lua.org/)

## Configration

When started, `tym` reads `$XDG_CONFIG_HOME/tym/config.lua` if it exists. Available options are below.

### String fields

- `title`  
  Set your prefered title. (default: `'tym'`)

- `shell`  
  Set your prefered shell. (default: check `$SHELL`, if not set, check `vte_get_user_shell()` and if it NULL use `'/bin/sh'`  )

- `font`  
  Set `'FAMILY-LIST [SIZE]'` like `'Ubuntu Mono 12'`. This option value is internally passed to [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If set `''`(empty string), system default fixed width font will be used. (default: `''`)

- `cursor_blink`  
  `'system'`, `'on'` or `'off'` are available. (default: `'system'`)

- `cjk_width`  
  `'narrow'` or `'wide'` are available (default: `'narrow'`)

- `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground` and `color_0` ... `color_255`  
  you can specify standard color string like `'#f00'`, `'#ff0000'` or `'red'`. These will be parsed with [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If set `''`(empty string), VTE default color will be used.  (default: `''`)


### Integer fields

- `width`  
  Set prefered yours columns. (default: `80`)

- `height`  
  Set prefered yours rows. (default: `22`)


### Example confing

Here is an example.

```lua
config.shell = '/bin/fish'
config.font = 'DejaVu Sans Mono 10'
config.cursor_blink_mode = 'system'
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

## Key bindings

| Key            | Action                      |
|:-------------- |:--------------------------- |
| Ctrl Shift c   | Copy selection to clipboard |
| Ctrl Shift v   | Paste from clipboard        |
| Ctrl Shift r   | Reload config file          |
| Ctrl -         | Decrease font scale         |
| Ctrl +         | Increase font scale         |
| Ctrl =         | Reset font scale            |

## Install

If you are an Arch linux user, just run

```console
$ yaourt -S tym
```

[the AUR package](https://aur.archlinux.org/packages/tym/) is maintained by me.

## Compile

Download source code from [release page](https://github.com/endaaman/tym/releases), unarchive it and

```console
$ ./configure
$ make
$ sudo make install
```

## TODOs

- Configurable features
  - Default geometry
  - Custom key bindings

## License

MIT
