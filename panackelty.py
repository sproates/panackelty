#!/usr/bin/env python3
"""Compatibility facade for tools importing the transitional host API.

New implementation code lives under ``src``. This module remains temporarily
so existing development tools and third-party experiments do not break while
Panackelty is being bootstrapped.
"""

from src.bootstrap.panackelty import *
from src.bootstrap.panackelty import main


if __name__ == "__main__":
    raise SystemExit(main())
