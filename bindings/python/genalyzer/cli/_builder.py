# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Reflection-based click.Command builder for genalyzer.mcp tool functions.

Each MCP tool has a fully type-hinted signature. This module introspects that
signature and produces a click.Command whose options mirror the tool's kwargs
one-for-one. Output is JSON on stdout, pretty by default or compact with
--compact. Tool errors (returned dict with an "error" key) exit 0; unhandled
exceptions exit 1; click arg-parsing errors exit 2.
"""

from __future__ import annotations

import inspect
import json
import sys
import types
import typing
from typing import Any, Callable

import click


_CLICK_TYPES: dict[type, Any] = {
    int: click.INT,
    float: click.FLOAT,
    str: click.STRING,
}


def _unwrap_optional(annotation: Any) -> tuple[Any, bool]:
    """If annotation is Optional[T] / T | None, return (T, True). Else (annotation, False)."""
    origin = typing.get_origin(annotation)
    if origin is typing.Union or origin is types.UnionType:
        args = typing.get_args(annotation)
        non_none = [a for a in args if a is not type(None)]
        if len(non_none) == 1 and len(args) == 2:
            return non_none[0], True
    return annotation, False


def _translate(param: inspect.Parameter) -> tuple[Any, bool, Any, bool]:
    """Translate a function parameter into (click_type, required, default, is_flag)."""
    annotation, optional = _unwrap_optional(param.annotation)
    has_default = param.default is not inspect.Parameter.empty
    default = param.default if has_default else None
    required = not has_default and not optional

    if annotation is bool:
        return None, False, bool(default) if has_default else False, True

    click_type = _CLICK_TYPES.get(annotation)
    if click_type is None:
        raise ValueError(
            f"unsupported annotation for parameter {param.name!r}: {annotation!r}"
        )
    return click_type, required, default, False


def _json_default(obj: Any) -> Any:
    """JSON encoder fallback. Coerces numpy scalars/arrays to Python natives."""
    if hasattr(obj, "item"):
        try:
            return obj.item()
        except (ValueError, AttributeError):
            pass
    if hasattr(obj, "tolist"):
        try:
            return obj.tolist()
        except (ValueError, AttributeError):
            pass
    return str(obj)


def click_from_tool(tool_func: Callable[..., dict], name: str) -> click.Command:
    """Build a click.Command from an @mcp.tool()-decorated (or plain) function.

    The function must have type hints on every parameter. Parameters become
    kebab-cased options. A `--compact/--pretty` flag is always appended.
    """
    sig = inspect.signature(tool_func)
    try:
        hints = typing.get_type_hints(tool_func)
    except Exception:
        hints = {}
    params: list[click.Parameter] = []

    for pname, param in sig.parameters.items():
        flag_base = pname.replace("_", "-")
        if pname in hints:
            param = param.replace(annotation=hints[pname])
        click_type, required, default, is_flag = _translate(param)
        if is_flag:
            decls = [f"--{flag_base}/--no-{flag_base}"]
            params.append(click.Option(decls, default=default))
        else:
            decls = [f"--{flag_base}"]
            params.append(click.Option(decls, type=click_type, required=required, default=default))

    params.append(click.Option(["--compact/--pretty"], default=False,
                               help="Emit single-line JSON instead of pretty-printed."))

    def callback(**kwargs: Any) -> None:
        compact = kwargs.pop("compact")
        indent = None if compact else 2
        try:
            result = tool_func(**kwargs)
        except Exception as e:
            payload = {"error": f"{type(e).__name__}: {e}"}
            click.echo(json.dumps(payload, indent=indent, default=_json_default))
            sys.exit(1)
        click.echo(json.dumps(result, indent=indent, default=_json_default))

    doc = inspect.getdoc(tool_func) or ""
    return click.Command(name, params=params, callback=callback, help=doc)
