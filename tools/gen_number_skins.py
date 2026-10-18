#!/usr/bin/env python3
"""Generate the generic number SVG skins (data/svg/number_*.svg).

Usage: tools/gen_number_skins.py [output_dir]

The output directory defaults to data/svg. Run svgo afterwards
(pre-commit run svgo --all-files) and regenerate the manual images
(./build/tests/tests "[screenshots]").

The binding attributes are described in
manual/modules/ROOT/pages/svginstrument.adoc. Text elements get placeholder
content because svgo removes empty ones; the plugin always replaces it.
"""

import pathlib
import sys

OUT = pathlib.Path(
    sys.argv[1] if len(sys.argv) > 1
    else pathlib.Path(__file__).resolve().parent.parent / "data" / "svg")

BACKGROUND = "#20252b"  # Same as rudder.svg
LINE = "#2e343b"
DIM = "#a8a8a8"
BRIGHT = "#f2f2f2"


def text(x, y, size, fill, anchor, attrs, body, bold=False):
    """A <text> element, using only what the plugin's text drawing supports."""
    weight = ' font-weight="bold"' if bold else ""
    return (f'<text x="{x}" y="{y}" fill="{fill}" font-size="{size}"{weight} '
            f'text-anchor="{anchor}" {attrs}>{body}</text>')


def value(x, y, size, slot, anchor="middle"):
    """Value readout, zone colored."""
    return text(x, y, size, BRIGHT, anchor,
                f'data-dsk-text="v{slot}" data-dsk-fmt="%.1f" '
                f'data-dsk-zone-fill="v{slot}"', "---", True)


def caption(x, y, slot, anchor):
    """User label of a value."""
    return text(x, y, 12, DIM, anchor, f'data-dsk-caption="v{slot}"', "Label")


def unit(x, y, slot, anchor):
    """User unit of a value."""
    return text(x, y, 12, DIM, anchor, f'data-dsk-unit="v{slot}"', "Unit")


def line(d):
    """Separator between cells."""
    return f'<path stroke="{LINE}" stroke-width="2" d="{d}"/>'


def cell(cx, top, slot, size=30):
    """Caption, value and unit stacked, for cells side by side."""
    base = top + 15 + size + 6
    return [caption(cx, top + 15, slot, "middle"), value(cx, base, size, slot),
            unit(cx, base + 18, slot, "middle")]


def row(top, slot):
    """Caption top left, value and unit on the right, for stacked rows."""
    return [caption(12, top + 18, slot, "start"),
            value(156, top + 50, 30, slot, "end"),
            unit(162, top + 50, slot, "start")]


def skin(name, w, h, labels, body):
    """Write one skin: background, body elements and the title on top."""
    names = " ".join(f'data-dsk-label-v{i}="{label}"'
                     for i, label in enumerate(labels))
    title = text(w // 2, 22, 14, DIM, "middle", 'data-dsk-title="1"', "Title")
    svg = (f'<svg xmlns="http://www.w3.org/2000/svg" {names} '
           f'viewBox="0 0 {w} {h}">'
           f'<rect width="{w}" height="{h}" fill="{BACKGROUND}" rx="10"/>'
           + "".join(body) + title + "</svg>\n")
    (OUT / f"{name}.svg").write_text(svg)


skin("number_1", 200, 120, ["Value"],
     [value(100, 80, 44, 0), caption(12, 108, 0, "start"),
      unit(188, 108, 0, "end")])
skin("number_2h", 200, 120, ["Left value", "Right value"],
     [line("M100 32v78")] + cell(50, 32, 0) + cell(150, 32, 1))
skin("number_2v", 200, 160, ["Top value", "Bottom value"],
     [line("M10 96h180")] + row(32, 0) + row(96, 1))
skin("number_3v", 200, 220, ["Top value", "Middle value", "Bottom value"],
     [line("M10 96h180M10 160h180")] + row(32, 0) + row(96, 1)
     + row(160, 2))
skin("number_4", 200, 180,
     ["Top left value", "Top right value", "Bottom left value",
      "Bottom right value"],
     [line("M100 32v140M10 102h180")] + cell(50, 32, 0, 26)
     + cell(150, 32, 1, 26) + cell(50, 102, 2, 26) + cell(150, 102, 3, 26))
