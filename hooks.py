# -*- coding: utf-8 -*-
import shutil
import pathlib


def _copy_license(project_root: pathlib.Path) -> None:
    shutil.copy(project_root.joinpath("LICENSE.md"), "docs/LICENSE.md")


def _copy_doxygen(project_root: pathlib.Path, docs_dir: pathlib.Path) -> None:
    shutil.copytree(
        project_root.joinpath("docs", "doxygen", "html"), docs_dir.joinpath("doxygen")
    )


def copy_files(*_, **kwargs):
    docs_dir = pathlib.Path(kwargs["config"]["docs_dir"]).resolve()
    project_root = pathlib.Path(__file__).resolve().parent.parent.parent.parent

    if not docs_dir.joinpath("LICENSE.md").exists():
        _copy_license(project_root)

    if not docs_dir.joinpath("doxygen").exists():
        _copy_doxygen(project_root, docs_dir)
