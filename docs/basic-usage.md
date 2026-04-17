\mainpage Logger: Basic Usage

`logger` is a small C++ logging library centered on `DV::Logger`.

Use the module interface:

```cpp
import logger;
#include <iostream>

int main() {
    DV::Logger log("Terminal Log", std::cout);
    float pi = 3.14f;
    log.info("Simple test:", 5, pi, 'a', "b c");
}
```

Example output:

```text
[TZONE YYYY-MM-DD HH:MM:SS:NS] [Terminal Log:INFO] Simple test: 5 3.14 a b c
```

For log-level semantics and intended usage, see @ref log_levels.
