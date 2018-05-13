# [alpha] Recognice

A nice and simple Python library for recognition of music streams.  
Uses Gracenote SDK (GNSDK) as a backend.  
Currently in alpha/experimental stage, so anything could change in the future.

## Building
1. `cp Makefile.config.example Makefile.config`
2. Edit the config file.
3. `make`  
   or  
   `make V=1` for more verbose output.

## Running the example (main.py)
1. `cp config.py.example config.py`
2. Edit the config file.
3. `python3 main.py *stream-url-or-file-path* | python3 viewlog.py` (a bit awkward for now).
