# clang-compose
## Features
Merge every project header (excluding STL headers) by running:
```
./compose path-to-leaf.cpp
```

otherwise move the `#include` directive to the top

## Setup
* Create `build` directory and build it there 
* Enjoy using

The only thing that is required is `compile_commands.json` in the root directory of your project

## Testing
An example project is in `tests` directory.

## Limitations
Some strange C syntax cannot be used for top-level **user code** (non-STL) declarations, e.g.:
```
class Cls {
} obj;
```

```
class {
} anon_obj;
```
The tool throws unhandled exception on discovery of such things

Also, any macro definition that is not `#include ...` is ignored (e.g., `#pragma pack...`, `#define INEEDMYOWNPI 3.14`), but it is out of the intended use scope

## ToDo
* Maybe provide better project discovery (for now, the included file is merged iff it is in `/home` directory)
