# Introduction
libqrcode is C++20 header-only library for generating QR/Micro QR Codes.

Features include:
* Header-only library
* Completely constexpr
* Code is (unit-)tested with `static_asserts` during compile-time
* Utilizes standard library only
* Uses Ranges
* No C++ exceptions (however)

# QR support
Supported: 
* Versions: 1-40
* Error correction levels: L, M, Q, H
* Encodings: Numeric, alphanumeric, byte and kanji
* ECI mode (Enhanced channel interpretation)
* (Simple) Bit stream optimization

Not supported
* Structured append mode
* FNC1 mode

# Micro QR support
Supported:
* Versions: M1-M4
* Error correction levels: L, M, Q (where possible)
* Encodings: Numeric, alphanumeric, byte and kanji encoding (where possible)

# Usage
In your project, just in include the header [`qrcode/qrcode.h`](include/qrcode/qrcode.h). 
Micro QR API is located within namespace `micro_qr`.
QR API is located within namespace `qr`.

#### Simple example
```
#include <qrcode/qrcode.h>

int main()
{
    using namespace qrcode;

    auto qr_symbol = qr::make_symbol("Foo!", qr::error_correction::level_H);
    auto micro_qr_symbol = micro_qr::make_symbol("Bar!", micro_qr::error_correction::level_L);
}
```

#### ECI example
```
#include <qrcode/qrcode.h>

int main()
{
    using namespace qrcode;
    using namespace std::literals;

    auto symbol = qr::make_symbol(
        eci::view{eci::assignment_number{9}, "\xC1\xC2\xC3\xC4\xC5"sv}, 
        qr::error_correction::level_H);
}
```

#### Accessing modules
```
#include <qrcode/qrcode.h>
#include <iostream>

int main()
{
    using namespace qrcode;
    using namespace std::literals;
    
    auto symbol = qr::make_symbol("Hello World!"sv, qr::error_correction::level_L).value();

    auto count = 0;
    for (auto i : views::horizontal(symbol))
    {
        std::cout << (i ? '#' : ' ');
        ++count;
        if (count % width(symbol) == 0)
            std::cout << '\n';
    }
}
```

# Requirements
* C++ compiler and standard library
* Supported:
   * gcc10 and libstdc++-10-dev or higher
   * msvc: Visual Studio 2019 16.10.0
* Not supported: 
  * clang doesn't seem to support all featured of C++20 needed to make this work
  * macOS (due to lack of C++20 features), support will be added in the future

# How to build
* Since the library is header-only, building is actually not necessary.
* Just put the `include` folder of this project into your header search path and it should just work.
* However, if you'd like to run the tests, please select the top-level of the project in order to run _CMake_.

## How to run the tests
If you would like to run the tests, one has to add the following cmake flag: `-DQRCODE_TESTS_ENABLED:BOOL=True`

## Debug builds using MSVC
Building with tests enabled in _Debug_ configuration won't build due to an issue in _Microsoft_'s STL implementation. _Release_ configuration works though.

# CI builds
CI builds consist of a test build for _Windows_ and _Linux_. 
If and only if all builds were successful, the CI state is green.
