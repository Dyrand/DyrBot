# DyrBot [![Build Status](https://travis-ci.org/Dyrand/DyrBot.svg?branch=master)](https://travis-ci.org/Dyrand/DyrBot)
A revitalized version of IRC-Bot with support for multiple bots by utilizing multiple threads.

* [Dependencies](#dependencies)
* [Installation](#installation)

# Dependencies
__Libraries__
* [Boost](http://www.boost.org/) (minimum version 1.55)

__Build Tool__
* [CMake](https://cmake.org/) (minimum version 2.8)

# Installation
Download the source from `https://github.com/Dyrand/DyrBot/tree/master`

###  Quick Build
In the directory where the source is located:
```
mkdir build
cd build
cmake ..
make
```

# Usage
Create 1 bot using default config ./config/config.txt
```
dyrbot
```

Create 5 bots using the config file ./bot_config_files/dyrbot.txt
```
dyrbot -n 5 -d bot_config_file/dyrbot.txt
```

