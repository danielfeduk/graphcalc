# graphcalc

This project allows you to display scalar fields using a live heatmap.
Currently this is at a very early stage. It takes an expression in terms of x, y, and z, and displays it on a x, y Cartesian heatmap where z is time.

This project is a work in progress.

## Building

**Build dependencies**: `re2c`, `lemon`.

**Runtime dependencies**: libepoxy, SDL2.

To build the program, run the following:
```
make
```

## Running

You run the command as so:
```
./graphcalc [formula]
```


