# -*- coding: utf-8 -*-
import shutil
import pathlib


def _copy_license(project_root: pathlib.Path) -> None:
    shutil.copy(project_root.joinpath("LICENSE.md"), "docs/LICENSE.md")


def _copy_doxygen(project_root: pathlib.Path, docs_dir: pathlib.Path) -> None:
    out_dir = docs_dir.joinpath("doxygen")
    if out_dir.exists():
        shutil.rmtree(out_dir)
    shutil.copytree(
        project_root.joinpath("docs", "doxygen", "html"),
        out_dir,
        dirs_exist_ok=True,
    )


def copy_files(*_, **kwargs):
    docs_dir = pathlib.Path(kwargs["config"]["docs_dir"]).resolve()
    project_root = pathlib.Path(__file__).resolve().parent.parent.parent.parent

    if not docs_dir.joinpath("LICENSE.md").exists():
        _copy_license(project_root)

    _copy_doxygen(project_root, docs_dir)
