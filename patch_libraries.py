from os.path import join, isfile

Import("env")

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduino-mbed")
patchflag_path = join(FRAMEWORK_DIR, ".patching-done")

replace_dict = [
    {
        "dest": join("libraries", "SPI", "SPI.cpp"),
        "src": "SPI.cpp-replace"
    }
]

# patch file only if we didn't do it before
if not isfile(join(FRAMEWORK_DIR, ".patching-done")):
    for replace in replace_dict:
        original_file = join(FRAMEWORK_DIR, replace["dest"])
        patched_file = join("patches", replace["src"])

        assert isfile(original_file) and isfile(patched_file)

        env.Execute(f"cp {patched_file} {original_file}")
    env.Execute("touch " + patchflag_path)


    # def _touch(path):
    #     with open(path, "w") as fp:
    #         fp.write("")

    # env.Execute(lambda *args, **kwargs: _touch(patchflag_path))