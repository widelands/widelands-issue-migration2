#!/usr/bin/env python3
# encoding: utf-8

from glob import glob
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import time
import datetime

#Python2/3 compat code for iterating items
try:
    dict.iteritems
except AttributeError:
    # Python 3
    def itervalues(d):
        return iter(d.values())
    def iteritems(d):
        return iter(d.items())
    def bytes_to_str(bytes):
        return str(bytes, 'utf-8')
else:
    # Python 2
    def itervalues(d):
        return d.itervalues()
    def iteritems(d):
        return d.iteritems()
    def bytes_to_str(bytes):
        return str(bytes)

has_timeout = "TimeoutExpired" in dir(subprocess)

if "monotonic" in dir(time):
    get_time = time.monotonic
elif "perf_counter" in dir(time):
    get_time = time.perf_counter
else:
    get_time = time.time

def datadir():
    return os.path.join(os.path.dirname(__file__), "data")

def datadir_for_testing():
    return os.path.relpath(".", os.path.dirname(__file__))

def out(string):
    sys.stdout.write(string)
    sys.stdout.flush()


def mark_line(severity, line, file=None, line_no=None, title=None):
    """Add an annotation on github or color on console."""
    if os.getenv("GITHUB_ACTION"):
        ref_list = []
        for k, val in iteritems({"file": file, "line": line_no, "title": title}):
            if val:
                ref_list.append(f"{k}={val}")
        refs = " " + ",".join(ref_list) if ref_list else ""
        return f"::{ severity }{ refs }::{ line }"
    else:
        sev_to_col = {
            "info": 36,  # cyan
            "warning": 33,  # yellow
            "error": 31,  # red
        }
        color = sev_to_col.get(severity, 35)  # fallback to violet
        line = f"\033[{ color }m{ line }\033[{ 39 }m"  # line has colour
        if file:
            line = line.replace(file, f"\033[4m{ file }\033[24m", 1)  # underline filename
        return line


def mark_failures(stdout, test_script):
    """find failure lines in stdout of a test run and mark them

    finds: failure lines and test summary from lunit, failure messages from widelands

    Examples of detected lines are shown in code.

    Failures are marked on github or colored on console.

    returns the modified stdout
    """
    map_dir = os.path.dirname(os.path.dirname(test_script))
    test_title = ""
    test_titles = set()
    last_wl_err_idx = -2
    lines = stdout.split("\n")
    for idx, line in enumerate(lines):
        if line.startswith("#### Running "):
            m = re.search("'(.*)'", line)
            test_title = m.group(1) if m else line.split("Running", 1)[1].strip()
            warn = ""
            if "(0 Test" in line:  # to detect usage of wrong test variable (copy/paste)
                warn = "no tests"
            if test_title in test_titles:  # to detect copied test name
                warn = (warn + ", duplicate test name").lstrip(", ")
            test_titles.add(test_title)
            if warn:
                lines[idx] = mark_line("warning", line, title=warn)
        elif ":" in line and line.startswith(("FAIL: ", "ERROR: ", "WARNING: ")):
            data = line.split(':', 4)
            severity = "warning" if data[0] == "WARNING" else "error"
            if len(data) > 4 and data[3].isnumeric():
                # format:
                # FAIL: test_name: [string "scripting/flag.lua"]:22: expected 'flag' but was 'ware'!
                test_name = data[1].lstrip()
                m = re.search(r'"(.*)"', data[2])
                if m:  # found file name in double quotes
                    file = m.group(1)
                else:
                    m = re.search(r"\[(.*)\]", data[2])  # filename in brackets
                    file = m.group(1) if m else data[2].strip()
                if file.startswith("scripting"):
                    file = f"{map_dir}/{file}"
                line_no = data[3]
                title = f"{test_title} - {test_name}"
                err = mark_line(severity, line, file=file, line_no=line_no, title=title)
            elif len(data) > 1:
                # format:
                # FAIL: test_road_crosses_another: Couldn't build flag!
                test_name = data[1].lstrip()
                # could try to find file and line in following traceback
                err = mark_line(severity, line, title=f"{test_title} - {test_name}")
            else:
                # format:
                # WARNING: teardown() failed!
                err = mark_line(severity, line, title=test_title)
            lines[idx] = err
        elif "Assertions checked." in line and "Tests passed" in line:
            # summary of lua test run
            lines[idx] = mark_line("warning", line, title="test summary")
        elif "] ERROR: " in line:
            # error message of widelands, like:
            # [00:00:00.000 real] ERROR: [] config:0: RealFSImpl::load: ...
            if "could not open file for reading" in line and "/config" in line or \
               test_title == "string.bformat test":
                # no config is no problem, or triggered by testrun
                continue
            if "lua_errors.cc" in line:
                lines[idx] = mark_line("error", line, title="lua error")
            elif last_wl_err_idx + 1 != idx:  # to mark each block only once
                lines[idx] = mark_line("info", line, title="potential problem")
            last_wl_err_idx = idx
    return "\n".join(lines) + "\n"


class WidelandsTestCase(unittest.TestCase):
    do_use_random_directory = True
    path_to_widelands_binary = None
    keep_output_around = False
    ignore_error_code = False
    timeout = 600

    def __init__(self, test_script, **wlargs):
        unittest.TestCase.__init__(self)
        self._test_script = test_script
        self._wlargs = wlargs

    def __str__(self):
        return self._test_script

    def setUp(self):
        if self.do_use_random_directory:
            self.run_dir = tempfile.mkdtemp(prefix="widelands_regression_test")
        else:
            self.run_dir = os.path.join(tempfile.gettempdir(), "widelands_regression_test", self.__class__.__name__)
            if os.path.exists(self.run_dir):
                if not self.keep_output_around:
                    shutil.rmtree(self.run_dir)
                    os.makedirs(self.run_dir)
            else:
                os.makedirs(self.run_dir)
        self.widelands_returncode = 0
        self.wl_timed_out = False

    def run(self, result=None):
        self.currentResult = result # remember result for use in tearDown
        unittest.TestCase.run(self, result)

    def tearDown(self):
        if self.currentResult.wasSuccessful() and not self.keep_output_around:
            shutil.rmtree(self.run_dir)

    def run_widelands(self, wlargs, which_time):
        """Run Widelands with the given 'wlargs'. 'which_time' is an integer
        defining the number of times Widelands has been run this test case
        (i.e. because we might load a saved game from an earlier run. This will
        impact the filenames for stdout.txt.

        Returns the stdout filename."""
        stdout_filename = os.path.join(self.run_dir, "stdout_{:02d}.txt".format(which_time))
        if (os.path.exists(stdout_filename)):
            os.unlink(stdout_filename)

        with open(stdout_filename, 'a') as stdout_file:
            args = [self.path_to_widelands_binary,
                    '--verbose=true',
                    '--datadir={}'.format(datadir()),
                    '--datadir_for_testing={}'.format(datadir_for_testing()),
                    '--homedir={}'.format(self.run_dir),
                    '--nosound',
                    '--fail-on-lua-error',
                    '--language=en_US' ]
            args += [ "--{}={}".format(key, value) for key, value in iteritems(wlargs) ]

            stdout_file.write("Running widelands binary: ")
            for anarg in args:
              stdout_file.write(anarg)
              stdout_file.write(" ")
            stdout_file.write("\n")

            start_time = get_time()
            widelands = subprocess.Popen(
                    args, shell=False, stdout=stdout_file, stderr=subprocess.STDOUT)
            if has_timeout:
                try:
                    widelands.communicate(timeout = self.timeout)
                except subprocess.TimeoutExpired:
                    widelands.kill()
                    widelands.communicate()
                    self.wl_timed_out = True
                    stdout_file.write("\nTimed out.\n")
            else:
                widelands.communicate()
            end_time = get_time()
            stdout_file.flush()
            self.duration = datetime.timedelta(seconds = end_time - start_time)
            stdout_file.write("\nReturned from Widelands in {}, return code is {:d}\n".format(
                self.duration, widelands.returncode))
            self.widelands_returncode = widelands.returncode
        return stdout_filename

    def runTest(self):
        out("\nStarting test case {}\n".format(self._test_script))
        out("  Running Widelands ...\n")
        stdout_filename = self.run_widelands(self._wlargs, 0)
        stdout = open(stdout_filename, "r").read()
        self.verify_success(stdout, stdout_filename)

        find_saves = lambda stdout: re.findall("Script requests save to: (\w+)$", stdout, re.M)
        savegame_done = { fn: False for fn in find_saves(stdout) }
        which_time = 1
        while not all(savegame_done.values()):
            for savegame in sorted(savegame_done):
                if not savegame_done[savegame]: break
            out("  Loading savegame: {} ...\n".format(savegame))
            stdout_filename = self.run_widelands({ "loadgame": os.path.join(
                self.run_dir, "save", "{}.wgf".format(savegame))}, which_time)
            which_time += 1
            stdout = open(stdout_filename, "r").read()
            for new_save in find_saves(stdout):
                if new_save not in savegame_done:
                    savegame_done[new_save] = False
            savegame_done[savegame] = True
            self.verify_success(stdout, stdout_filename)

    def verify_success(self, stdout, stdout_filename):
        out("    Elapsed time: {}\n".format(self.duration))
        skipped_msg = None
        # Catch instabilities with SDL in CI environment
        if self.widelands_returncode == 2:
            print("SDL initialization failed. TEST SKIPPED.")
            if os.getenv("GITHUB_ACTION"):
                print("\n::group::stdout")  # visual group for stdout on github
            out(stdout)
            if os.getenv("GITHUB_ACTION"):
                print("\n::endgroup::")
            out("  SKIPPED.\n")
            skipped_msg = "SDL initialization failed"
        else:
            class CommonFailMsg:
                """To only create error message on failure"""

                def __init__(self_msg, intro):
                    self_msg.intro = intro

                def __str__(self_msg):
                    start_stdout = end_stdout = ""
                    stdout_txt = stdout  # stdout is a nonlocal variable
                    if os.getenv("GITHUB_ACTION"):
                        start_stdout = "::group::stdout\n"
                        end_stdout = "::endgroup::\n"

                    if os.getenv("GITHUB_ACTION") or \
                            sys.stdout.isatty() and os.getenv("TERM") != "dumb":
                        stdout_txt = mark_failures(stdout, self._test_script)

                    return (f"{self_msg.intro} Analyze the files in {self.run_dir} to see why "
                            f"this test case failed. Stdout is\n  {stdout_filename}\n\n"
                            f"{start_stdout}stdout:\n{stdout_txt}{end_stdout}")

            if self.wl_timed_out:
                out("  TIMED OUT.\n")
                self.fail(CommonFailMsg("The test timed out."))
            if self.widelands_returncode == 1 and self.ignore_error_code:
                out("  IGNORING error code 1\n")
            else:
                self.assertEqual(self.widelands_returncode, 0,
                                 CommonFailMsg("Widelands exited abnormally."))
            longMessage = self.longMessage
            self.longMessage = False  # only show custom message
            self.assertTrue("All Tests passed" in stdout,
                            CommonFailMsg("Not all tests pass."))
            self.assertFalse("lua_errors.cc" in stdout,
                             CommonFailMsg("Not all tests pass (output has error message)."))
                             # mainly happens if error code is ignored
            self.longMessage = longMessage  # reset to combined messages for next call
            out("  done.\n")
        if self.keep_output_around:
            out("    stdout: {}\n".format(stdout_filename))
        if skipped_msg:
            self.skipTest(skipped_msg)  # reports this clearly and aborts this TestCase here

def parse_args():
    p = argparse.ArgumentParser(description=
        "Run the regression tests suite."
    )

    p.add_argument("-r", "--regexp", type=str,
        help = "Run only the tests from the files which filename matches."
    )
    p.add_argument("-n", "--nonrandom", action="store_true", default = False,
        help = "Do not randomize the directories for the tests. This is useful "
        "if you want to run a test more often than once and not reopen stdout.txt "
        "in your editor."
    )
    p.add_argument("-k", "--keep-around", action="store_true", default = False,
        help = "Keep the output files around even when a test terminates successfully."
    )
    p.add_argument("-b", "--binary", type=str,
        help = "Run this binary as Widelands. Otherwise some default paths are searched."
    )
    p.add_argument("-i", "--ignore-error-code", action="store_true", default = False,
        help = "Assume success on return code 1, to allow running the tests "
        "without ASan reporting false positives."
    )
    if has_timeout:
        p.add_argument("-t", "--timeout", type=float, default = "10",
            help = "Set the timeout duration for test cases in minutes. Default is 10 minutes."
        )
    else:
        p.epilog = "Python version does not support timeout. -t, --timeout is disabled. " \
                   "Python >=3.3 is required for timeout support."

    args = p.parse_args()

    if args.binary is None:
        potential_binaries = (
            glob(os.path.join(os.curdir, "widelands")) +
            glob(os.path.join(os.path.dirname(__file__), "widelands")) +
            glob(os.path.join("src", "widelands")) +
            glob(os.path.join("..", "*", "src", "widelands"))
        )
        if potential_binaries:
            args.binary = potential_binaries[0]
        elif "which" in dir(shutil):
            args.binary = shutil.which("widelands")

        if args.binary is None:
            p.error("No widelands binary found. Please specify with -b.")

    return args


def discover_loadgame_tests(regexp, suite):
    """Add all tests using --loadgame to the 'suite'."""
    for fixture in sorted(glob(os.path.join("test", "save", "*"))):
        if not os.path.isdir(fixture):
            continue
        savegame = sorted(glob(os.path.join(fixture, "*.wgf")))[0]
        for test_script in sorted(glob(os.path.join(fixture, "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        loadgame=savegame, script=test_script))

def discover_scenario_tests(regexp, suite):
    """Add all tests using --scenario to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        scenario=wlmap, script=test_script))

def discover_editor_tests(regexp, suite):
    """Add all tests needing --editor to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "editor_test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        editor=wlmap, script=test_script))

def main():
    args = parse_args()

    WidelandsTestCase.path_to_widelands_binary = args.binary
    print("Using '{}' binary.".format(args.binary))
    WidelandsTestCase.do_use_random_directory = not args.nonrandom
    WidelandsTestCase.keep_output_around = args.keep_around
    WidelandsTestCase.ignore_error_code = args.ignore_error_code
    if has_timeout:
        WidelandsTestCase.timeout = args.timeout * 60
    else:
        out("Python version does not support timeout on subprocesses,\n"
            "test cases may run indefinitely.\n\n")

    suite = unittest.TestSuite()
    discover_loadgame_tests(args.regexp, suite)
    discover_scenario_tests(args.regexp, suite)
    discover_editor_tests(args.regexp, suite)

    result = unittest.TextTestRunner(verbosity=2).run(suite)

    to_check = {
        "Failed tests": result.failures,
        "Error in test": result.errors,  # error in python code, should not happend
        "Unexpected in test": result.unexpectedSuccesses,  # we have no expected failure yet
        "Skipped tests": result.skipped,
    }
    for reason, issues in iteritems(to_check):
        if len(issues) > 0:
            out(f"{ reason }:\n")
            for issue in issues:
                info = ""
                if isinstance(issue, (list, tuple)):
                    p_issue = issue[0]
                    initial_line = True  # title of traceback
                    for line in issue[1].split("\n"):
                        if not initial_line and line[0] != " ":
                            # first line which starts with no space: AssertionXxx or ErrorXxx
                            info = "    \t" + line.split(".", 1)[0] + "."
                            break
                        initial_line = False
                else:  # unexpectedSuccess has no additional info
                    p_issue = issue
                out(f" - {p_issue}{info}\n")

    return result.wasSuccessful()

if __name__ == '__main__':
    sys.exit(0 if main() else 1)
