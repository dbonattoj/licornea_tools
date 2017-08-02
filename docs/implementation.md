# Implementation details

- Each `.cc` file one of the subdirectories of `src/` is an executable. `lib/` contains a library that may be used by all programs. `src/category/lib/` contains a library that is used only by programs in the that category. The programs never rely on libraries from programs in other categories.

- C++ modules are named `.cc`, headers are `.h`. `.tcc` are also headers, included by the `.h`, for template implementations.

- [json.hpp](https://github.com/nlohmann/json) is used to parse JSON, and [fmt](https://github.com/fmtlib/fmt) to parse Python-style format stirngs. They are included in `src/external/`.

- To read the command line argument (and ensure the expected argument are there), `lib/args.h` provides `get_args()` and functions like `int_arg()` that will be called in order.

- `out_filename_arg()` checks if files already exist before continuing, and also creates underlying directories. That process it such that there can be no race conditions when it is done from multiple threads concurrently.

- There is a part to access the file system (check if files exist, create directories, etc.) It is the only OS-dependent part, and is implemented for POSIX (Linux, macOS) and for Windows, in `lib/filesystem.h`.
 
- `lib/common.h` contains the definition of the `real` type used everywhere. It can be changed from `double` to `float`. It also defines keycodes (for _Enter_ and _Esc_, for the viewer GUI), which may need to be changed.

- For assertions, `Assert()` or `Assert_crit()` (defined in `lib/assert.h`) are used. `Assert_crit` is not checked when compiling in release mode, like `assert()` from `<cassert>`.

- Libraries for the Python scripts are in `lib/pylib`. They will be copies into every subdirectory for the installed scripts, so that they can be found.

- There is a facility `lib/pylib/temporary.py` for the Python scripts to generate temporary files.

- The class reading the [dataset parameters](data/dataset.html) is implemented for both C++ (`lib/dataset.h`) and Python (`lib/pylib/dataset.py`), and both are kept exactly the same.

- The batch process facility `lib/pylib/batch.py` uses [joblib](https://pythonhosted.org/joblib/) for the parallelization. It uses the `threading` backend, so that it creates separate threads and not processes. It is important because shared (atomic) variables are used for the progress estimation and other places.
