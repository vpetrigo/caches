#!/usr/bin/env python
# -*- coding: utf-8 -*-
import pathlib
import tempfile
import shutil
import subprocess
import argparse
from typing import Union


def copy_docs_output_dir(
    docs_dir: Union[str, pathlib.Path], out: Union[str, pathlib.Path]
) -> None:
    shutil.copytree(pathlib.Path(docs_dir).resolve(), out, dirs_exist_ok=True)


def github_pages_nojekyll(out: Union[str, pathlib.Path]) -> None:
    with pathlib.Path(out, ".nojekyll").open("w+"):
        pass


def git_init(repo: str, branch: str, working_dir: Union[str, pathlib.Path]) -> None:
    try:
        subprocess.run(
            ["git", "clone", "-b", branch, repo, "."], cwd=working_dir, check=True
        )
    except subprocess.CalledProcessError:
        subprocess.run(["git", "clone", repo, "."], cwd=working_dir, check=True)
        subprocess.run(
            ["git", "switch", "--orphan", branch], cwd=working_dir, check=True
        )


def git_add_documentation_files(working_dir: Union[str, pathlib.Path]) -> None:
    subprocess.run(
        ["git", "add", "."], cwd=working_dir, stdout=subprocess.DEVNULL, check=True
    )


def git_commit(message: str, working_dir: Union[str, pathlib.Path]) -> None:
    subprocess.run(["git", "commit", "-m", message], cwd=working_dir, check=True)


def git_push(
    repository: str, branch: str, token: str, working_dir: Union[str, pathlib.Path]
) -> None:
    repo = repository.replace(r"//", rf"//{token}@")
    subprocess.run(
        ["git", "push", repo, branch, "--quiet"], cwd=working_dir, check=True
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Upload generated documentation to GitHub Pages"
    )
    parser.add_argument("-n", action="store_true")
    parser.add_argument("-r", "--repo", type=str)
    parser.add_argument("-b", "--branch", type=str)
    parser.add_argument("-t", "--token", type=str)
    parser.add_argument("--commit", type=str)
    options = parser.parse_args()

    with tempfile.TemporaryDirectory() as docudir:
        git_init(options.repo, options.branch, docudir)
        copy_docs_output_dir(pathlib.Path("docs", "html"), docudir)
        github_pages_nojekyll(docudir)
        git_add_documentation_files(docudir)
        git_commit(f"Update {options.commit} documentation", docudir)
        git_push(options.repo, options.branch, options.token, docudir)


if __name__ == "__main__":
    main()
