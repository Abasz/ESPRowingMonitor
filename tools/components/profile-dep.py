Import("env")

import os
import re

import SCons.Scanner

# SCons' C/C++ scanner emulates the preprocessor with regexes and cannot expand
# macros, so `#include BOARD_PROFILE` / `#include ROWER_PROFILE`
# (src/default.settings.h) never register the profile headers as dependencies
# and editing a profile does not trigger a rebuild. (CMake does not have this
# problem because it takes its dependencies from the compiler's own -MD
# depfiles.)
#
# The scanner is wrapped so that any scanned file containing a macro include of
# one of these macros additionally reports the resolved profile header as a
# dependency. SCons scans implicit dependencies recursively, so this propagates
# to exactly the translation units that include the profile - unlike a global
# build flag, which would rebuild everything.

PROFILE_MACROS = ("BOARD_PROFILE", "ROWER_PROFILE")


def macro_include_pattern(macro):
    return re.compile(r"^\s*#\s*include\s+" + macro + r"\s*$", re.MULTILINE)


def find_profile(macro):
    for define in env.get("CPPDEFINES", []):
        if isinstance(define, (list, tuple)) and len(define) == 2 and define[0] == macro:
            return str(define[1]).strip("'\"")

        if isinstance(define, str):
            match = re.match(r"^%s\s*=\s*[\"']?([^\"'\s]+)" % macro, define)
            if match:
                return match.group(1)

    flags = env.get("BUILD_FLAGS", []) or []
    match = re.search(r"%s\s*=\s*[\"']?([^\"'\s]+)" % macro, " ".join(flags))

    return match.group(1) if match else None


def resolve_profile_node(macro):
    profile = find_profile(macro)

    if not profile:
        return None

    profile_path = os.path.join(env.subst("$PROJECT_SRC_DIR"), profile)

    if not os.path.isfile(profile_path):
        print("warning: %s not found on disk: %s" % (macro, profile_path))

        return None

    return env.File(profile_path)


def patch_scanner(extra_dependencies):
    original_scan = SCons.Scanner.ClassicCPP.scan

    if getattr(original_scan, "profile_dep_patched", False):
        return

    def scan(self, node, path=()):
        dependencies = original_scan(self, node, path)

        try:
            text = node.get_text_contents()
        except Exception:
            return dependencies

        matched = [profile_node for pattern, profile_node in extra_dependencies if pattern.search(text)]

        return list(dependencies) + matched if matched else dependencies

    scan.profile_dep_patched = True
    SCons.Scanner.ClassicCPP.scan = scan


extra_dependencies = [
    (macro_include_pattern(macro), profile_node)
    for macro, profile_node in ((macro, resolve_profile_node(macro)) for macro in PROFILE_MACROS)
    if profile_node is not None
]

if extra_dependencies:
    patch_scanner(extra_dependencies)
