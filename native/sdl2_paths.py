Import("env")
import sys
import os
import glob

if sys.platform.startswith("darwin"):
    # Homebrew has shipped SDL2 under a couple of different keg names over time ("sdl2", now "sdl2-compat"), check both
    sdl_include = []
    sdl_lib = []
    for pkg in ("sdl2-compat", "sdl2"):
        if not sdl_include:
            sdl_include = glob.glob(f"/opt/homebrew/Cellar/{pkg}/*/include")
        if not sdl_lib:
            sdl_lib = glob.glob(f"/opt/homebrew/Cellar/{pkg}/**/libSDL2.a", recursive=True)
            if not sdl_lib:
                sdl_lib = glob.glob(f"/opt/homebrew/Cellar/{pkg}/**/libSDL2.dylib", recursive=True)

    if sdl_include:
        print(f"Found Homebrew SDL include path: {sdl_include[0]}")
        env.Append(CPPPATH=[sdl_include[0]])
    else:
        print("WARNING: could not find SDL2 include path under /opt/homebrew/Cellar")

    if sdl_lib:
        print(f"Found Homebrew SDL lib path: {sdl_lib[0]}")
        env.Append(LIBPATH=[os.path.dirname(sdl_lib[0])])
    else:
        print("WARNING: could not find SDL2 lib path under /opt/homebrew/Cellar")
