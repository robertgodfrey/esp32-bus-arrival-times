Import("env")

exec_name = "${BUILD_DIR}/${PROGNAME}${PROGSUFFIX}"

# "upload" doesn't mean anything for a native build -- repurpose it to just
# run the compiled simulator binary, so `pio run -t upload` launches the window
from SCons.Script import AlwaysBuild
AlwaysBuild(env.Alias("upload", exec_name, exec_name))

env.AddTarget(
    name="execute",
    dependencies=exec_name,
    actions='"{}"'.format(exec_name),
    title="Execute",
    description="Build and run the simulator",
    group="General",
)
