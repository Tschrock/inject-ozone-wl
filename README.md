inject-ozone-wl
===============

A hack to force certain electron processes to run with wayland support. It uses `LD_PRELOAD` to override `__libc_start_main()` so we can wrap the `main()` method to inject the `--ozone-platform=wayland` and `--enable-features=WaylandWindowDecorations` flags into the running process.

## Compiling
```sh
gcc inject-ozone-wl.c -o inject-ozone-wl.so -fPIC -shared -ldl
```

## Installing
```sh
sudo cp inject-ozone-wl.so /usr/local/lib/
```

## Usage
```sh
LD_PRELOAD=/usr/local/lib/inject-ozone-wl.so myprogram
```

## ~~Installing globally~~ Breaking your computer
(Why would you do this?)
```sh
echo '/usr/local/lib/inject-ozone-wl.so' | sudo tee -a /etc/ld.so.preload
```

## FAQ

**Couldn't you just write a wrapper script?**

Yes

**Will this hurt my computer?**

Most likely

**Can I use this in production?**

No
