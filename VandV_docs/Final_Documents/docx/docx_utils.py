#!/usr/bin/env python3
"""
Shared DOCX utility module for NGIC-template-styled Word document generation.

Provides helper functions for creating formatted Word documents matching
the NGIC 4.0 documentation template styling:
  - Calibri 11pt body font
  - Page margins: 0.75in left/right, 0.85in top/bottom
  - Table headers: RGB(14, 117, 188) with white bold text
  - Classification headers/footers
  - Equation boxes, code blocks, title pages

Usage:
    from docx_utils import DocxHelper
    helper = DocxHelper()
    doc = helper.create_document()
    helper.add_heading(doc, "Section Title", level=1)
    ...
"""

try:
    from docx import Document
    from docx.shared import Inches, Pt, Cm, RGBColor, Emu
    from docx.enum.text import WD_ALIGN_PARAGRAPH
    from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
    from docx.enum.section import WD_ORIENT
    from docx.oxml.ns import qn, nsdecls
    from docx.oxml import OxmlElement, parse_xml
except ImportError:
    print("ERROR: python-docx is not installed.")
    print("Install it with:")
    print("    pip install python-docx")
    print("  or:")
    print("    python3.12 -m pip install python-docx --user")
    raise SystemExit(1)


# ============================================================================
# Constants
# ============================================================================

# NGIC TableBlue: RGB(14, 117, 188)
TABLE_HEADER_COLOR = "0E75BC"
TABLE_HEADER_RGB = RGBColor(14, 117, 188)

# Alternating row shading (light blue/gray)
ALT_ROW_COLOR = "E8F0FE"

# Code block background
CODE_BG_COLOR = "F2F2F2"

# Classification marking
CLASSIFICATION_TEXT = "(U) UNCLASSIFIED"

# Font settings
BODY_FONT_NAME = "Calibri"
BODY_FONT_SIZE = Pt(11)
CODE_FONT_NAME = "Courier New"
CODE_FONT_SIZE = Pt(8)

# Heading sizes
HEADING_SIZES = {
    1: Pt(16),  # Section
    2: Pt(14),  # Subsection
    3: Pt(12),  # Subsubsection
    4: Pt(11),  # Paragraph heading
}

# Page margins
MARGIN_LEFT = Inches(0.75)
MARGIN_RIGHT = Inches(0.75)
MARGIN_TOP = Inches(0.85)
MARGIN_BOTTOM = Inches(0.85)


# ============================================================================
# Low-level XML helpers
# ============================================================================

def set_cell_shading(cell, color):
    """Set table cell background color.

    Args:
        cell: A python-docx table cell object.
        color: Hex color string without '#' (e.g., '0E75BC').
    """
    shading_elm = OxmlElement("w:shd")
    shading_elm.set(qn("w:fill"), color)
    shading_elm.set(qn("w:val"), "clear")
    shading_elm.set(qn("w:color"), "auto")
    cell._tc.get_or_add_tcPr().append(shading_elm)


def set_cell_borders(cell, top=None, bottom=None, left=None, right=None):
    """Set individual cell borders.

    Each border argument is a dict with keys: sz (eighths of a point),
    color (hex string), val (e.g., 'single').
    """
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    tcBorders = OxmlElement("w:tcBorders")
    for edge, props in [("top", top), ("bottom", bottom),
                        ("left", left), ("right", right)]:
        if props is not None:
            element = OxmlElement(f"w:{edge}")
            element.set(qn("w:val"), props.get("val", "single"))
            element.set(qn("w:sz"), str(props.get("sz", 4)))
            element.set(qn("w:color"), props.get("color", "000000"))
            element.set(qn("w:space"), "0")
            tcBorders.append(element)
    tcPr.append(tcBorders)


def set_table_borders(table, sz=4, color="000000"):
    """Set uniform borders on the entire table.

    Args:
        table: A python-docx Table object.
        sz: Border size in eighths of a point (4 = 0.5pt).
        color: Hex color string.
    """
    tbl = table._tbl
    tblPr = tbl.tblPr if tbl.tblPr is not None else OxmlElement("w:tblPr")
    borders = OxmlElement("w:tblBorders")
    for edge in ("top", "left", "bottom", "right", "insideH", "insideV"):
        element = OxmlElement(f"w:{edge}")
        element.set(qn("w:val"), "single")
        element.set(qn("w:sz"), str(sz))
        element.set(qn("w:color"), color)
        element.set(qn("w:space"), "0")
        borders.append(element)
    tblPr.append(borders)


def set_cell_width(cell, width_inches):
    """Set the width of a table cell."""
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    tcW = OxmlElement("w:tcW")
    tcW.set(qn("w:w"), str(int(width_inches * 1440)))
    tcW.set(qn("w:type"), "dxa")
    tcPr.append(tcW)


def set_cell_vertical_alignment(cell, alignment="center"):
    """Set vertical alignment for a cell ('top', 'center', 'bottom')."""
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    vAlign = OxmlElement("w:vAlign")
    vAlign.set(qn("w:val"), alignment)
    tcPr.append(vAlign)


def set_paragraph_spacing(paragraph, before=0, after=0, line=None):
    """Set paragraph spacing in points."""
    pPr = paragraph._p.get_or_add_pPr()
    spacing = OxmlElement("w:spacing")
    spacing.set(qn("w:before"), str(int(before * 20)))
    spacing.set(qn("w:after"), str(int(after * 20)))
    if line is not None:
        spacing.set(qn("w:line"), str(int(line * 20)))
        spacing.set(qn("w:lineRule"), "auto")
    pPr.append(spacing)


def _add_header_footer_text(section, text, is_header=True):
    """Add text to the header or footer of a section."""
    if is_header:
        hf = section.header
    else:
        hf = section.footer
    hf.is_linked_to_previous = False
    if hf.paragraphs:
        p = hf.paragraphs[0]
    else:
        p = hf.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = p.add_run(text)
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(9)
    run.font.bold = True
    run.font.color.rgb = RGBColor(0, 0, 0)


# ============================================================================
# Document-level helpers
# ============================================================================

def create_document():
    """Create a new Document with NGIC template margins and default font.

    Returns:
        A python-docx Document object configured with standard margins.
    """
    doc = Document()

    # Set margins for all sections
    for section in doc.sections:
        section.top_margin = MARGIN_TOP
        section.bottom_margin = MARGIN_BOTTOM
        section.left_margin = MARGIN_LEFT
        section.right_margin = MARGIN_RIGHT

    # Set default font
    style = doc.styles["Normal"]
    font = style.font
    font.name = BODY_FONT_NAME
    font.size = BODY_FONT_SIZE

    # Set default paragraph spacing
    pf = style.paragraph_format
    pf.space_before = Pt(0)
    pf.space_after = Pt(6)

    return doc


def add_classification_header(doc):
    """Add '(U) UNCLASSIFIED' header and footer to all sections.

    Args:
        doc: A python-docx Document object.
    """
    for section in doc.sections:
        _add_header_footer_text(section, CLASSIFICATION_TEXT, is_header=True)
        _add_header_footer_text(section, CLASSIFICATION_TEXT, is_header=False)


# ============================================================================
# Content helpers
# ============================================================================

def add_heading(doc, text, level=1):
    """Add a heading with classification prefix '(U) '.

    Args:
        doc: A python-docx Document object.
        text: The heading text (without classification prefix).
        level: Heading level (1=Section, 2=Subsection, 3=Subsubsection).

    Returns:
        The heading paragraph object.
    """
    full_text = f"(U) {text}"
    heading = doc.add_heading(full_text, level=level)

    # Override font to Calibri and set size
    for run in heading.runs:
        run.font.name = BODY_FONT_NAME
        run.font.size = HEADING_SIZES.get(level, Pt(11))
        run.font.bold = True
        run.font.color.rgb = RGBColor(0, 0, 0)

    return heading


def add_paragraph(doc, text, style=None, bold=False, italic=False):
    """Add a body paragraph.

    Args:
        doc: A python-docx Document object.
        text: The paragraph text.
        style: Optional Word style name (e.g., 'List Bullet').
        bold: Whether the text should be bold.
        italic: Whether the text should be italic.

    Returns:
        The paragraph object.
    """
    if style:
        para = doc.add_paragraph(style=style)
    else:
        para = doc.add_paragraph()

    run = para.add_run(text)
    run.font.name = BODY_FONT_NAME
    run.font.size = BODY_FONT_SIZE
    if bold:
        run.font.bold = True
    if italic:
        run.font.italic = True

    return para


def add_bullet_list(doc, items, level=0):
    """Add a bulleted list.

    Args:
        doc: A python-docx Document object.
        items: List of strings for each bullet point.
        level: Indentation level (0=first level, 1=sub-bullet, etc.).

    Returns:
        List of paragraph objects.
    """
    paragraphs = []
    for item in items:
        para = doc.add_paragraph(style="List Bullet")
        run = para.add_run(item)
        run.font.name = BODY_FONT_NAME
        run.font.size = BODY_FONT_SIZE
        if level > 0:
            pf = para.paragraph_format
            pf.left_indent = Inches(0.5 * level)
        paragraphs.append(para)
    return paragraphs


def add_numbered_list(doc, items):
    """Add a numbered list.

    Args:
        doc: A python-docx Document object.
        items: List of strings for each numbered item.

    Returns:
        List of paragraph objects.
    """
    paragraphs = []
    for item in items:
        para = doc.add_paragraph(style="List Number")
        run = para.add_run(item)
        run.font.name = BODY_FONT_NAME
        run.font.size = BODY_FONT_SIZE
        paragraphs.append(para)
    return paragraphs


def add_code_block(doc, text):
    """Add a monospace code block with light gray background.

    Args:
        doc: A python-docx Document object.
        text: The code text (may contain newlines).

    Returns:
        The table object containing the code block.
    """
    # Use a single-cell table to provide background color
    table = doc.add_table(rows=1, cols=1)
    table.style = "Table Grid"
    cell = table.rows[0].cells[0]
    set_cell_shading(cell, CODE_BG_COLOR)

    # Clear default paragraph
    cell.paragraphs[0].clear()

    # Add code text
    para = cell.paragraphs[0]
    run = para.add_run(text)
    run.font.name = CODE_FONT_NAME
    run.font.size = CODE_FONT_SIZE

    # Reduce paragraph spacing inside code block
    set_paragraph_spacing(para, before=2, after=2)

    # Add small spacer after code block
    spacer = doc.add_paragraph()
    spacer_run = spacer.add_run("")
    spacer_run.font.size = Pt(4)

    return table


def add_equation_box(doc, equation_text, caption=None):
    """Add a bordered equation box with optional caption.

    Args:
        doc: A python-docx Document object.
        equation_text: The equation text to display.
        caption: Optional caption text below the equation.

    Returns:
        The table object containing the equation.
    """
    table = doc.add_table(rows=1, cols=1)
    table.style = "Table Grid"
    cell = table.rows[0].cells[0]

    # Light blue background for equation box
    set_cell_shading(cell, "EBF5FB")

    # Center the equation
    para = cell.paragraphs[0]
    para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = para.add_run(equation_text)
    run.font.name = "Cambria Math"
    run.font.size = Pt(11)
    run.font.italic = True

    set_paragraph_spacing(para, before=4, after=4)

    # Add caption if provided
    if caption:
        cap_para = doc.add_paragraph()
        cap_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
        run = cap_para.add_run(caption)
        run.font.name = BODY_FONT_NAME
        run.font.size = Pt(9)
        run.font.italic = True

    return table


def add_table_with_header(doc, headers, rows, col_widths=None):
    """Create a styled table with NGIC blue header row and alternating shading.

    Args:
        doc: A python-docx Document object.
        headers: List of header column strings.
        rows: List of lists, each inner list is a row of cell values.
        col_widths: Optional list of column widths in inches.

    Returns:
        The table object.
    """
    num_cols = len(headers)
    num_rows = len(rows) + 1  # +1 for header
    table = doc.add_table(rows=num_rows, cols=num_cols)
    table.style = "Table Grid"
    table.alignment = WD_TABLE_ALIGNMENT.CENTER

    # Set table borders (0.5pt = sz 4)
    set_table_borders(table, sz=4, color="000000")

    # Header row
    for col_idx, header_text in enumerate(headers):
        cell = table.rows[0].cells[col_idx]
        set_cell_shading(cell, TABLE_HEADER_COLOR)
        set_cell_vertical_alignment(cell, "center")

        # Clear and set text
        cell.paragraphs[0].clear()
        run = cell.paragraphs[0].add_run(str(header_text))
        run.font.name = BODY_FONT_NAME
        run.font.size = Pt(10)
        run.font.bold = True
        run.font.color.rgb = RGBColor(255, 255, 255)

        # Set column width if specified
        if col_widths and col_idx < len(col_widths):
            set_cell_width(cell, col_widths[col_idx])

        set_paragraph_spacing(cell.paragraphs[0], before=2, after=2)

    # Data rows
    for row_idx, row_data in enumerate(rows):
        for col_idx in range(num_cols):
            cell = table.rows[row_idx + 1].cells[col_idx]

            # Alternating row shading
            if row_idx % 2 == 1:
                set_cell_shading(cell, ALT_ROW_COLOR)

            # Set column width if specified
            if col_widths and col_idx < len(col_widths):
                set_cell_width(cell, col_widths[col_idx])

            # Set text
            cell_text = str(row_data[col_idx]) if col_idx < len(row_data) else ""
            cell.paragraphs[0].clear()
            run = cell.paragraphs[0].add_run(cell_text)
            run.font.name = BODY_FONT_NAME
            run.font.size = Pt(9)

            set_paragraph_spacing(cell.paragraphs[0], before=1, after=1)

    # Add small spacer after table
    spacer = doc.add_paragraph()
    spacer_run = spacer.add_run("")
    spacer_run.font.size = Pt(4)

    return table


def add_title_page(doc, title, subtitle, version, date):
    """Create a formatted title page.

    Args:
        doc: A python-docx Document object.
        title: Main document title (e.g., 'User and Analyst Manual (UAM)').
        subtitle: Subtitle (e.g., 'T-150/TRV-150 Heavy-Lift UAS Simulation Model').
        version: Version string (e.g., '3.0').
        date: Date string (e.g., 'March 5, 2026').
    """
    # Classification at top
    class_top = doc.add_paragraph()
    class_top.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = class_top.add_run(CLASSIFICATION_TEXT)
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(12)
    run.font.bold = True

    # Vertical spacing
    for _ in range(4):
        spacer = doc.add_paragraph()
        spacer_run = spacer.add_run("")
        spacer_run.font.size = Pt(12)

    # Title
    title_para = doc.add_paragraph()
    title_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = title_para.add_run(title.upper())
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(26)
    run.font.bold = True

    # Subtitle
    sub_para = doc.add_paragraph()
    sub_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = sub_para.add_run(subtitle)
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(18)
    run.font.bold = True

    # Project line
    proj_para = doc.add_paragraph()
    proj_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = proj_para.add_run("AFSIM Project 005")
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(14)

    # Spacing
    for _ in range(2):
        spacer = doc.add_paragraph()
        spacer_run = spacer.add_run("")
        spacer_run.font.size = Pt(12)

    # Version and date
    ver_para = doc.add_paragraph()
    ver_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = ver_para.add_run(f"Version {version}")
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(14)

    date_para = doc.add_paragraph()
    date_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = date_para.add_run(date)
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(14)

    # Spacing
    for _ in range(2):
        spacer = doc.add_paragraph()
        spacer_run = spacer.add_run("")
        spacer_run.font.size = Pt(12)

    # Document metadata table
    meta_table = doc.add_table(rows=6, cols=2)
    meta_table.style = "Table Grid"
    meta_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_table_borders(meta_table, sz=4, color="000000")

    meta_data = [
        ("Classification", "UNCLASSIFIED"),
        ("Sponsoring Organization", "[PLACEHOLDER - Organization Name]"),
        ("Program Manager", "[PLACEHOLDER - Name/Title]"),
        ("Document Preparer", "[PLACEHOLDER - Organization/Contract]"),
        ("Distribution Statement", "[PLACEHOLDER - Distribution Statement]"),
        ("M&S Name and Version", "T-150 UAS AFSIM Plugin Suite v2.0"),
    ]

    for row_idx, (field, value) in enumerate(meta_data):
        # Field column
        cell_f = meta_table.rows[row_idx].cells[0]
        set_cell_shading(cell_f, TABLE_HEADER_COLOR)
        cell_f.paragraphs[0].clear()
        run = cell_f.paragraphs[0].add_run(field)
        run.font.name = BODY_FONT_NAME
        run.font.size = Pt(10)
        run.font.bold = True
        run.font.color.rgb = RGBColor(255, 255, 255)
        set_cell_width(cell_f, 2.0)

        # Value column
        cell_v = meta_table.rows[row_idx].cells[1]
        cell_v.paragraphs[0].clear()
        run = cell_v.paragraphs[0].add_run(value)
        run.font.name = BODY_FONT_NAME
        run.font.size = Pt(10)
        set_cell_width(cell_v, 4.5)

    # Bottom classification
    spacer = doc.add_paragraph()
    spacer_run = spacer.add_run("")
    spacer_run.font.size = Pt(12)

    class_bottom = doc.add_paragraph()
    class_bottom.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = class_bottom.add_run(CLASSIFICATION_TEXT)
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(12)
    run.font.bold = True

    doc.add_page_break()


def add_revision_table(doc, revisions):
    """Create a revision history table.

    Args:
        doc: A python-docx Document object.
        revisions: List of tuples (version, date, changes).
    """
    add_heading(doc, "Record of Changes", level=1)

    headers = ["Version", "Date", "Changes"]
    rows = [[v, d, c] for v, d, c in revisions]
    add_table_with_header(doc, headers, rows, col_widths=[0.8, 1.2, 5.0])

    doc.add_page_break()


def add_toc_placeholder(doc, sections):
    """Add a Table of Contents placeholder.

    Args:
        doc: A python-docx Document object.
        sections: List of section title strings.
    """
    add_heading(doc, "Table of Contents", level=1)

    toc_note = doc.add_paragraph()
    run = toc_note.add_run("(Auto-generate in Word: References > Table of Contents)")
    run.font.name = BODY_FONT_NAME
    run.font.size = Pt(10)
    run.font.italic = True

    for section in sections:
        para = doc.add_paragraph()
        run = para.add_run(section)
        run.font.name = BODY_FONT_NAME
        run.font.size = BODY_FONT_SIZE

    doc.add_page_break()


def add_note_box(doc, text):
    """Add a highlighted note/callout box.

    Args:
        doc: A python-docx Document object.
        text: The note text.

    Returns:
        The table object containing the note.
    """
    table = doc.add_table(rows=1, cols=1)
    table.style = "Table Grid"
    cell = table.rows[0].cells[0]
    set_cell_shading(cell, "FFF3CD")  # Light yellow

    para = cell.paragraphs[0]
    run_label = para.add_run("Note: ")
    run_label.font.name = BODY_FONT_NAME
    run_label.font.size = BODY_FONT_SIZE
    run_label.font.bold = True

    run_text = para.add_run(text)
    run_text.font.name = BODY_FONT_NAME
    run_text.font.size = BODY_FONT_SIZE

    set_paragraph_spacing(para, before=4, after=4)

    # Spacer
    spacer = doc.add_paragraph()
    spacer_run = spacer.add_run("")
    spacer_run.font.size = Pt(4)

    return table
