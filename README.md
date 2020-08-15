# tym

[![CircleCI](https://circleci.com/gh/endaaman/tym.svg?style=svg)](https://circleci.com/gh/endaaman/tym) [![Gitter chat](https://badges.gitter.im/tym-terminal/gitter.png)](https://gitter.im/tym-terminal/Lobby)

`tym` is a tiny VTE-based terminal emulator, which is configurable by Lua.

## Installation

### Arch Linux

```
$ yay -S tym
```

### Other distros

Download the latest release from [Releases](https://github.com/endaaman/tym/releases), extract it and run as below

```
$ ./configure
$ sudo make install
```

<details><summary>Build dependencies</summary>
<p>

#### Arch Linux

```
$ sudo pacman -S vte3 lua53
```

#### Ubuntu

```
$ sudo apt install libgtk-3-dev libvte-2.91-dev liblua5.3-dev libpcre2-dev
```

#### Other distros / macOS / Windows

I did not check which packages are needed to build on other distros or OS. I'm waiting for your contribution ;)

</p>
</details>

## Configuration

When `$XDG_CONFIG_HOME/tym/config.lua` exists, it is executed. You can change the path by `--use`/`-u` option.

```lua
-- Firstly, you need to require tym module
local tym = require('tym')

-- set config individually
tym.set('width', 100)

tym.set('font', 'DejaVu Sans Mono 11')

-- set by table
tym.set_config({
  shell = '/usr/bin/fish',
  cursor_shape = 'underline',
  autohide = true,
  color_foreground = 'red',
})
```

All available config values are shown below.

| field name | type | default value | description |
| --- | --- | --- | --- |
| `shell` | string | `$SHELL` → `vte_get_user_shell()` → `'/bin/sh'` | Shell to excute. |
| `term` | string | `'xterm-256color'` | Value to assign to `$TERM` |
| `title` | string | `'tym'` | Window title. |
| `font` | string | `''` | You can specify font with `'FAMILY-LIST [SIZE]'`, for example `'Ubuntu Mono 12'`. The value is parsed by [`pango_font_description_from_string()`](https://developer.gnome.org/pango/stable/pango-Fonts.html#pango-font-description-from-string). If empty string is set, the system default fixed width font will be used. |
| `icon` | string | `'utilities-terminal'` | Name of icon. cf. [Icon Naming Specification](https://developer.gnome.org/icon-naming-spec/) |
| `role` | string | `''` | Unique identifier for the window. If empty string is set, no value set. (cf. [gtk_window_set_role()](https://developer.gnome.org/gtk3/stable/GtkWindow.html#gtk-window-set-role)) |
| `cursor_shape` | string | `'block'` | `'block'`, `'ibeam'` or `'underline'` can be used. |
| `cursor_blink_mode` | string | `'system'` | `'system'`, `'on'` or `'off'` can be used. |
| `cjk_width` | string | `'narrow'` | `'narrow'` or `'wide'` can be used. |
| `background_image` | string | `''` | Path to background image file. |
| `uri_schemes` | string | `'http https file mailto'` | Space-separated list of URI schemes to be highlighted and clickable. Specify empty string to disable highlighting. Specify `'*'` to accept any strings valid as schemes (according to RFC 3986). |
| `width` | integer | `80` | Initial columns. |
| `height` | integer | `22` | Initial rows. |
| `scale` | integer | `100` | Font scale in **percent(%)** |
| `padding_horizontal`  | integer | `0` | Horizontal padding. |
| `padding_vertical`  | integer | `0` | Vertical padding. |
| `scrollback_length` | integer | `512` | Length of the scrollback buffer. |
| `ignore_default_keymap` | boolean | `false` | Whether to use default keymap. |
| `ignore_bold` | boolean | `false` | Whether to allow drawing bold text. (cf. [vte_terminal_set_allow_bold()](https://developer.gnome.org/vte/unstable/VteTerminal.html#vte-terminal-set-allow-bold)). |
| `autohide` | boolean | `false` | Whether to hide mouse cursor when the user presses a key. |
| `silent` | boolean | `false` | Whether to beep when bell sequence is sent. |
| `color_window_background` | string | `''` | Color of the terminal window. It is seen when `'padding_horizontal'` `'padding_vertical'` is not `0`. If you set `'NONE'`, the window background will not be drawn. |
| `color_foreground`, `color_background`, `color_cursor`, `color_cursor_foreground`, `color_highlight`, `color_highlight_foreground`, `color_bold`, `color_0` ... `color_15` | string | [See next section](#user-content-theme-customization) | You can specify standard color string such as `'#f00'`, `'#ff0000'`, `'rgba(22, 24, 33, 0.7)'` or `'red'`. It will be parsed by [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). If empty string is set, the VTE default color will be used. If you set `'NONE'` for `color_background`, the terminal background will not be drawn.|


## Theme customization

When `$XDG_CONFIG_HOME/tym/theme.lua` exists, it is loaded before config is loadig. You can change the path by `--theme`/`-t` option. The following is an example and it shows builtin default values, which are ported from [iceberg](https://cocopon.github.io/iceberg.vim/).

```lua
local bg = '#161821'
local fg = '#c6c8d1'
return {
  color_background = bg,
  color_foreground = fg,
  color_bold = fg,
  color_cursor = fg,
  color_cursor_foreground = bg,
  color_highlight = fg,
  color_highlight_foreground = bg,
  color_0  = bg,
  color_1  = '#e27878',
  color_2  = '#b4be82',
  color_3  = '#e2a478',
  color_4  = '#84a0c6',
  color_5  = '#a093c7',
  color_6  = '#89b8c2',
  color_7  = fg,
  color_8  = '#6b7089',
  color_9  = '#e98989',
  color_10 = '#c0ca8e',
  color_11 = '#e9b189',
  color_12 = '#91acd1',
  color_13 = '#ada0d3',
  color_14 = '#95c4ce',
  color_15 = '#d2d4de',
}
```

You need to return the color map as table.

<details><summary>Color correspondence</summary>
<div>

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

</div>
</details>


## Keymap

### Default keymap

| Key             | Action                       |
| :-------------- | :--------------------------- |
| Ctrl Shift c    | Copy selection to clipboard. |
| Ctrl Shift v    | Paste from clipboard.        |
| Ctrl Shift r    | Reload config file.          |

### Customizing keymap

You can register keymap(s) with `tym.set_keymap(accelerator, func)` or `tym.set_keymaps(table)`. `accelerator` must be in a format parsable by [gtk_accelerator_parse()](https://developer.gnome.org/gtk3/stable/gtk3-Keyboard-Accelerators.html#gtk-accelerator-parse). If turethy value is returned, the key input event propagation will **not be stopped**.

```lua
-- also can set keymap
tym.set_keymap('<Ctrl><Shift>o', function()
  local h = tym.get('height')
  tym.set('height', h + 1)
  tym.notify('Set window height :' .. h)
end)

-- set by table
tym.set_keymaps({
  ['<Ctrl><Shift>t'] = function()
    tym.reload()
    tym.notify('reload config')
  end,
  ['<Ctrl><Shift>v'] = function()
    -- reload and notify
    tym.send_key('<Ctrl><Shift>t')
  end,

  ['<Shift>y'] = function()
    tym.notify('Y has been pressed')
    return true -- notification is shown and `Y` will be inserted
  end,
  ['<Shift>w'] = function()
    tym.notify('W has been pressed')
    -- notification is shown but `W` is not inserted
  end,
})
```

## Lua API

| Name                                 | Return value | Description |
| ------------------------------------ | ------------ | ----------- |
| `tym.get(key)`                       | any      | Get config value. |
| `tym.set(key, value)`                | void     | Set config value. |
| `tym.get_default_value(key)`         | any      | Get default config value. |
| `tym.get_config()`                   | table    | Get whole config. |
| `tym.set_config(table)`              | void     | Set config by table. |
| `tym.reset_config()`                 | void     | Reset all config. |
| `tym.set_keymap(accelerator, func)`  | void     | Set keymap. |
| `tym.unset_keymap(accelerator)`      | void     | Unset keymap. |
| `tym.set_keymaps(table)`             | void     | Set keymaps by table. |
| `tym.reset_keymaps()`                | void     | Reset all keymaps. |
| `tym.set_hook(hook_name, func)`      | void     | Set a hook. |
| `tym.set_hooks(table)`               | void     | Set hooks. |
| `tym.reload()`                       | void     | Reload config file.|
| `tym.reload_theme()`                 | void     | Reload theme file. |
| `tym.send_key()`                     | void     | Send key press event. |
| `tym.set_timeout(func, interval=0)`  | int(tag) | Set timeout. return true in func to execute again. |
| `tym.clear_timeout(tag)`             | void     | Clear the timeout. |
| `tym.put(text)`                      | void     | Feed text. |
| `tym.bell()`                         | void     | Sound bell. |
| `tym.open(uri)`                      | void     | Open URI via your system default app like `xdg-open(1)`. |
| `tym.notify(message, title='tym')`   | void     | Show desktop notification. |
| `tym.copy(text, target='clipboard')` | void     | Copy text to clipboard. As `target`, `'clipboard'`, `'primary'` or `secondary` can be used. |
| `tym.copy_selection(target='clipboard')` | void | Copy current selection. |
| `tym.paste(target='clipboard')`      | void     | Paste clipboard. |
| `tym.check_mod_state(accelerator)`   | bool     | Check if the mod key(such as `'<Ctrl>'` or `<Shift>`) is being pressed. |
| `tym.color_to_rgba(color)`           | r, g, b, a | Convert color string to RGB bytes and alpha float using [`gdk_rgba_parse()`](https://developer.gnome.org/gdk3/stable/gdk3-RGBA-Colors.html#gdk-rgba-parse). |
| `tym.rgba_to_color(r, g, b, a)`      | string   | Convert RGB bytes and alpha float to color string like `rgba(255, 128, 0, 0.5)` can be used in color option such as `color_background`. |
| `tym.rgb_to_hex(r, g, b)`            | string   | Convert RGB bytes to 24bit HEX like `#ABCDEF`. |
| `tym.get_monitor_model()`            | string   | Get monitor model on which the window is shown. |
| `tym.get_cursor_position()`          | int, int | Get where column and row the cursor is. |
| `tym.get_clipboard(target='clipboard')` | string | Get content in the clipboard. |
| `tym.get_selection()`                | string   | Get selected text. |
| `tym.get_text(start_row, start_col, end_row, end_col)` | string | Get text on the terminal screen. If you set `-1` to `end_row` and `end_col`, the target area will be the size of termianl. |
| `tym.get_config_path()`              | string   | Get full path to config file. |
| `tym.get_theme_path()`               | string   | Get full path to theme file. |
| `tym.get_version()`                  | string   | Get version string. |

### Hooks

| Name | Param | Default action | Description |
| --- | --- | --- | --- |
| `title`       | title  | changes title | If string is returned, it will be used as the new title. |
| `bell`        | nil    | makes the window urgent when it is inactive. | If true is returned, the window will not be urgent. |
| `clicked`     | button, uri | If URI exists under cursor, opens it | Triggered when mouse button is pressed. |
| `scroll`      | delta_x, delta_x, mouse_x, mouse_y  | scroll buffer | Triggered when mouse wheel is scrolled. |
| `drag`        | filepath  | feed filepath to the console | Triggered when files are dragged to the screen. |
| `activated`   | nil    | nothing | Triggered when the window is activated. |
| `deactivated` | nil    | nothing | Triggered when the window is deactivated. |
| `selected`    | string | nothing | Triggered when the text in the terminal screen is selected. |
| `unselected`  | nil    | nothing | Triggered when the selection is unselected. |

If turethy value is returned in a callback function, the default action is will **be canceled**.

```lua
tym.set_hooks({
  title = function(t)
    tym.set('title', 'tym - ' .. t)
    return true -- this is needed to cancenl default title application
  end,
})

tym.set_hook('clicked', function(button, uri)
  print('you pressed button:', button) -- 1:left, 2:middle, 3:right...
  if uri then
    print('you clicked URI: ', uri)
    if button == 2 then
      -- disable URI open when middle button is clicked
      return true
    end
  end
end)
```

## Options

### `--help` `-h`

```
$ tym -h
```

### `--use=<path>` `-u <path>`

```
$ tym --use=/path/to/config.lua
```

If `NONE` is provided, all config will be default (user-defined config file will not be loaded).

```
$ tym -u NONE
```

### `--theme=<path>` `-t <path>`

```
$ tym --use=/path/to/theme.lua
```

If `NONE` is provided, default theme will be used.

```
$ tym -t NONE
```

### `--<config option>`

You can set config value via command line option.

```console
$ tym --shell=/bin/zsh --color_background=red --width=40 --ignore_default_keymap
```

## Development

Clone this repo and run as below

```console
$ autoreconf -fvi
$ ./configure --enable-debug
$ make && ./src/tym -u ./path/to/config.lua   # for debug
$ make check; cat src/tym-test.log            # for unit tests
```

Run tests in docker container

```console
$ docker build -t tym .
$ docker run tym
```

## Pro tips

<details><summary>Scroll mouse wheel to set window transparency/font scale</summary>
<div>

You can increase/decrease window transparency by pressing `Ctrl + Shift + Down` / `Ctrl + Shift + Up` or `Shift` + mouse wheel and increase/decrease font scale by `Ctrl` + mouse wheel.

```lua
local tym = require('tym')

function update_alpha(delta)
  r, g, b, a = tym.color_to_rgba(tym.get('color_background'))
  a = math.max(math.min(1.0, a + delta), 0.0)
  bg = tym.rgba_to_color(r, g, b, a)
  tym.set('color_background', bg)
  tym.notify(string.format('%s alpha to %f', (delta > 0 and 'Inc' or 'Dec'), a))
end

tym.set_keymaps({
  ['<Ctrl><Shift>Up'] = function()
    update_alpha(0.05)
  end,
  ['<Ctrl><Shift>Down'] = function()
    update_alpha(-0.05)
  end,
})

tym.set_hook('scroll', function(dx, dy, x, y)
  if tym.check_mod_state('<Ctrl>') then
    if dy > 0 then
      s = tym.get('scale') - 10
    else
      s = tym.get('scale') + 10
    end
    tym.set('scale', s)
    tym.notify('set scale: ' .. s .. '%')
    return true
  end
  if tym.check_mod_state('<Shift>') then
    update_alpha(dy < 0 and 0.05 or -0.05)
    return true
  end
end)
```

</div>
</details>


<details><summary>Automated</summary>
<div>

```lua
local tym = require('tym')

tym.set_config({
  shell = '/bin/bash',
})

tym.set_timeout(coroutine.wrap(function()
  local i = 0
  local message = 'echo "hello World!"'
  while i < #message do
    i = i + 1
    tym.put(message:sub(i, i))
    coroutine.yield(true)
  end
  coroutine.yield(true)
  tym.send_key('<Ctrl>m')
  coroutine.yield(false)
end), 100)
```

</div>
</details>

<details><summary>Prevent tmux's key inputs delay</summary>
<div>

Key inputs alt-h,j,k,l for switching tmux's pane is often delayed when tmux is busy. This is the fix for it.

```lua
local tym = require('tym')

local remap = function (a, b)
  tym.set_keymap(a, function()
    tym.send_key(b)
  end)
end
remap('<Alt>h', '<Alt>Left') -- remap as meta key inputs
remap('<Alt>l', '<Alt>Right')
remap('<Alt><Shift>h', '<Alt><Shift>Left')
remap('<Alt><Shift>l', '<Alt><Shift>Right')
```

</div>
</details>

## License

MIT
