#!/bin/env python3

'''
validates github workflow files

- checks if the called local workflows exist
- checks if the files checked for changes exist
- checks if listed files are in valid yaml string (happens while loading it)
'''

import glob
import io
import os
import subprocess
import sys
import yaml


def file_in_git(file):
    'returns True if the file is tracked by git'
    cmd = ['git', 'ls-files', '--error-unmatch', file]
    fp = subprocess.run(cmd, stdout=subprocess.DEVNULL)
    return fp.returncode == 0


class CheckGithubYaml:
    def __init__(self):
        self.failures = 0

    def _check_path_valid(self, file, ref):
        exists = False
        if os.path.exists(file) or glob.glob(file):
            exists = True
            if file_in_git(file):
                return
        elif '!(' in file:
            # TODO replace r'!([^()]*)' with '*', maybe check each part (separated by |)
            print('can not check extended glob', file, 'from', ref)
            return
        if exists:
            print('untracked file:', file, 'from', ref)
        else:
            print('missing file:', file, 'from', ref)
        self.failures += 1

    def _check_steps(self, steps: iter, yaml_path: str, path: str):
        for step in steps:
            if step.get('uses', '').startswith('dorny/paths-filter'):
                for k, files in yaml.safe_load(io.StringIO(step['with']['filters'])).items():
                    ref = {'file': path, 'in': f'{yaml_path}.steps.with.filters'}
                    for file in files:
                        if type(file) == list:
                            files = file
                            for file in files:
                                self._check_path_valid(file, ref)
                        else:
                            self._check_path_valid(file, ref)
            elif (step.get('uses', '').startswith('./')):
                # check use of local action
                ref = {'file': path, 'in': f'{yaml_path}.steps.uses'}
                self._check_path_valid(step['uses'], ref)

    def run(self, path):
        with open(path) as f:
            w = yaml.safe_load(f)
        if w.get('jobs'):
            # it is a workflow file
            for j_name, j_data in w['jobs'].items():
                steps = j_data.get('steps')
                if steps:
                    self._check_steps(steps, f'jobs.{j_name}', path)
                elif (j_data['uses'].startswith('./')):
                    # check use of shared workflow
                    ref = {'file': path, 'in': 'jobs.uses'}
                    self._check_path_valid(j_data['uses'], ref)
        elif w['runs'].get('using') == 'composite':
            # it is a composite action
            self._check_steps(w['runs'].get('steps', []), 'runs', path)


if __name__ == '__main__':
    check = CheckGithubYaml()
    for arg in sys.argv[1:]:
        check.run(arg)
    if check.failures:
        sys.exit(1)
