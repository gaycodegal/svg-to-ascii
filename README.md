# SVG to ASCII

## Use

    ./svg-to-ascii <path to svg> <width> <height>

## Build

Build it using bazel.build

```bash
bazel build //cc:svg-to-ascii
```

You'll find the built version at `bazel-bin/cc/svg-to-ascii`

## Test while developing

```bash
bazel run //cc:svg-to-ascii --run_under="cd '$(pwd)' &&" -- test/1.svg 34 34
```

## Notes

I don't intend on supporting this project it's mainly just a test for a game I'm building.
