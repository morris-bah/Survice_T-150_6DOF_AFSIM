# V&V Documentation Generation Process Guide

## Repeatable Process for Generating Verification & Validation Documents

**Developed from**: T-150 UAS AFSIM Plugin Suite (Project 005)
**Date**: March 2026
**Tools**: Claude Code CLI, MiKTeX, Python 3.12, eraser.io

---

## Table of Contents

1. [Overview](#1-overview)
2. [Prerequisites](#2-prerequisites)
3. [Input Requirements](#3-input-requirements)
4. [Phase 1: Research & Analysis](#4-phase-1-research--analysis)
5. [Phase 2: Markdown Generation](#5-phase-2-markdown-generation)
6. [Phase 3: LaTeX Conversion](#6-phase-3-latex-conversion)
7. [Phase 4: Diagram Generation](#7-phase-4-diagram-generation)
8. [Phase 5: PDF Compilation](#8-phase-5-pdf-compilation)
9. [Phase 6: Python DOCX Generation](#9-phase-6-python-docx-generation)
10. [Output Structure](#10-output-structure)
11. [Known Issues & Solutions](#11-known-issues--solutions)
12. [Adapting for Other Projects](#12-adapting-for-other-projects)

---

## 1. Overview

This guide documents the process used to generate 8 V&V documents in 3 formats (Markdown, LaTeX/PDF, Python-DOCX) for the T-150 UAS AFSIM simulation project. The process is designed to be repeatable for any software project requiring formal V&V documentation per:

- **DoDI 5000.61** (DoD M&S VV&A, Sep 2024)
- **MIL-STD-3022** (VV&A Documentation Templates, Jan 2008)
- **NGIC 4.0 Document Templates** (ADD, ICD, TDD, UAM)

### Documents Produced

**NGIC Technical Document Set (4 documents):**

| Document | Abbreviation | Purpose |
|----------|-------------|---------|
| Architecture Description Document | ADD | Code structure, UML, class hierarchies, design decisions |
| Interface Control Document | ICD | Plugin/DLL interfaces, data structures, inter-component communication |
| Test Design Document | TDD | Test families, test cases, traceability matrix, results |
| User and Analyst Manual | UAM | Physics models, parameters, KPIs, analyst guidance |

**MIL-STD-3022 VV&A Document Set (4 documents):**

| Document | MIL-STD Section | Purpose |
|----------|----------------|---------|
| Accreditation Plan | Appendix A | Requirements, methodology, participants, resources |
| V&V Plan | Appendix B | V&V approach, test design, acceptance criteria |
| V&V Report | Appendix C | Test results, findings, recommendations |
| Accreditation Report | Appendix D | Final assessment, conditional accreditation decision |

### Formats Produced

| Format | Tool | Output | Use Case |
|--------|------|--------|----------|
| Markdown (.md) | Claude Code | Plain text | Version control, quick editing, GitHub rendering |
| LaTeX (.tex) → PDF | MiKTeX/pdflatex | Formatted PDF | Formal deliverables matching NGIC templates |
| Python → DOCX | python-docx | Word documents | Stakeholder review, collaborative editing |

---

## 2. Prerequisites

### Software Requirements

| Tool | Version | Purpose | Install Command |
|------|---------|---------|-----------------|
| Claude Code CLI | Latest | AI-assisted document generation | `npm install -g @anthropic-ai/claude-code` |
| MiKTeX | 25.12+ | LaTeX to PDF compilation | `winget install MiKTeX.MiKTeX` |
| Python | 3.12+ | DOCX generation, image processing | Pre-installed or `winget install Python.Python.3.12` |
| python-docx | Latest | Word document generation | `pip install python-docx` |
| Pillow | Latest | Image optimization | `pip install Pillow` |
| eraser.io | Web service | ASCII diagram → professional images | https://eraser.io |
| Git | Latest | Version control | Pre-installed |

### MiKTeX Configuration

After installing MiKTeX, enable auto-install of missing packages:

```bash
# Enable auto-install (one-time)
initexmf --set-config-value=[MPM]AutoInstall=1

# Compile with --enable-installer flag for first-time package downloads
pdflatex --enable-installer -interaction=nonstopmode document.tex
```

The NGIC templates require ~40 LaTeX packages. MiKTeX downloads them automatically on first compile.

### Template Files Required

The NGIC LaTeX templates provide the formatting foundation:

```
Original_templates/
├── 4.0_Final_Format_ADD_20260130.pdf    # Reference PDFs (what output should look like)
├── 4.0_Final_Format_ICD_20260130.pdf
├── 4.0_Final_Format_TDD_20260130.pdf
├── 4.0_Final_Format_UAM_20260130.pdf
└── 4_0_Latex_Docs_Delivery_BAH/
    └── DocumentTemplates/
        ├── format/
        │   ├── NGICStyle.sty            # Master formatting (margins, fonts, tables, figures)
        │   ├── NGICGlossary.sty         # Acronym glossary definitions
        │   └── DurandalBib.sty          # Bibliography/citation style
        ├── values/
        │   └── ModelValues.sty          # Template placeholder values (customize per project)
        ├── content/                     # Template section files (reference for structure)
        └── CoreDocuments/
            ├── ADD.tex, UAM.tex, etc.   # Template driver files (reference for structure)
            └── ref.bib                  # Bibliography template
```

---

## 3. Input Requirements

Before starting document generation, gather these inputs from the target project:

### Project Source Code Analysis

| Input | How to Gather | Used In |
|-------|--------------|---------|
| Source file inventory | `find plugins/ -name "*.cpp" -o -name "*.hpp"` | ADD, ICD |
| Class hierarchies | Read header files for inheritance | ADD (UML) |
| Public interfaces | Read .hpp files for method signatures | ADD, ICD |
| Inter-component communication | Search for AuxData/shared state keys | ICD |
| Configuration parameters | Search for ProcessInput/config parsing | UAM, ICD |
| Build system | Read CMakeLists.txt | ADD |
| Lines of code | `wc -l plugins/**/*.cpp plugins/**/*.hpp` | All docs |

### Test Data

| Input | How to Gather | Used In |
|-------|--------------|---------|
| Test scenarios | List scenario files | TDD |
| Test results | Read output/results files | TDD, V&V Report |
| Validation reports | Read existing validation docs | TDD, V&V Report |
| Known bugs/fixes | Git log, issue tracker | TDD, V&V Report |

### Reference Data

| Input | Source | Used In |
|-------|--------|---------|
| System specifications | Manufacturer data, open-source research | UAM, all MIL-STD |
| Physics references | Textbooks, published papers | UAM (equations) |
| Standards | DoDI 5000.61, MIL-STD-3022 | All documents |
| Previous V&V docs | Earlier versions if they exist | All documents |

---

## 4. Phase 1: Research & Analysis

**Tool**: Claude Code CLI with Agent subprocesses
**Time**: ~30 minutes
**Output**: Comprehensive understanding of the codebase

### Step 1.1: Explore the Codebase

Use Claude Code's Explore agent to understand the project structure:

```
# In Claude Code, ask:
"Explore the plugin source code and document the class hierarchy,
public interfaces, inter-component communication patterns, and
build system structure."
```

### Step 1.2: Read Template Structure

Analyze the NGIC template files to understand required sections:

```
# Read the LaTeX template drivers and content files
# Key files: ADD.tex, UAM.tex, TDD.tex, ICD.tex
# Key content: Introduction.tex, SampleComponentFile.tex,
#              SampleTestCase.tex, DLLInterface.tex
```

### Step 1.3: Read Standards

Extract requirements from governing standards:

```
# Read DoDI 5000.61 and MIL-STD-3022 to understand:
# - Required document sections per Appendices A-D
# - Shared content across documents (MIL-STD-3022 Section 5.2)
# - Tailoring guidance (Section 5.3)
```

### Step 1.4: Gather Test Results

Collect all validation data, test outputs, and known issues.

---

## 5. Phase 2: Markdown Generation

**Tool**: Claude Code CLI
**Time**: ~1-2 hours
**Output**: 8 markdown files (~5,000-6,000 lines total)

### Document Build Order

Build documents in dependency order — earlier documents establish content reused by later ones:

```
1. UAM  (establishes system description, KPIs, parameters)
2. ADD  (establishes architecture, code structure)
3. ICD  (depends on ADD architecture — interfaces)
4. TDD  (depends on UAM requirements, ADD components)
5. V&V Plan       (depends on TDD test design)
6. Accreditation Plan  (depends on requirements)
7. V&V Report     (depends on V&V Plan + TDD results)
8. Accreditation Report (depends on V&V Report)
```

### Key Prompt Pattern for Each Document

```
"Write the [DOCUMENT NAME] for the [PROJECT NAME]. Write it to:
[output path]

Structure per [NGIC 4.0 template / MIL-STD-3022 Appendix X]:

[List all required sections with specific content notes]

Key technical details:
[List architecture, component names, test results, etc.]

Classification: UNCLASSIFIED with [PLACEHOLDER] for org-specific fields.
Reference standards: DoDI 5000.61 [1], MIL-STD-3022 [2]
Write approximately [N] lines."
```

### Shared Content Across Documents

Per MIL-STD-3022 Section 5.2, these sections are written once and referenced across all 8 documents:

- Problem Statement / Intended Use
- System Description
- M&S Requirements & Acceptability Criteria
- Assumptions, Capabilities, Limitations
- Requirements Traceability Matrix
- Basis of Comparison
- References / Bibliography
- Acronyms

### Output

```
markdown/
├── T150_UAM_v3.0.md           (~1,200 lines)
├── T150_ADD_v3.0.md           (~1,000 lines)
├── T150_ICD_v1.0.md           (~700 lines)
├── T150_TDD_v1.0.md           (~550 lines)
├── T150_VV_Plan_v1.0.md       (~600 lines)
├── T150_VV_Report_v1.0.md     (~700 lines)
├── T150_Accreditation_Plan_v1.0.md  (~550 lines)
└── T150_Accreditation_Report_v1.0.md (~500 lines)
```

---

## 6. Phase 3: LaTeX Conversion

**Tool**: Claude Code CLI
**Time**: ~1-2 hours
**Output**: LaTeX source files using NGIC style packages

### Step 3.1: Create Infrastructure

```
latex/
├── format/                    # Copy from Original_templates
│   ├── NGICStyle.sty
│   ├── NGICGlossary.sty
│   └── DurandalBib.sty
├── values/
│   └── ProjectModelValues.sty  # Create: project-specific values
├── images/
│   ├── ngic.png               # Copy from Original_templates
│   └── placeholder.png
└── ref.bib                    # Create: project bibliography
```

### Step 3.2: Create ProjectModelValues.sty

Customize the model values file for your project:

```latex
\NeedsTeXFormat{LaTeX2e}
\ProvidesPackage{values/ProjectModelValues}

\colorlet{ModelValueColor}{black}
\newcommand{\newModelValue}[2]{%
    \newcommand{#1}{\textcolor{ModelValueColor}{#2}}%
}%

%% Allow line breaks after underscores (CRITICAL for table formatting)
\let\SavedUnderscore\_
\protected\def\_{\SavedUnderscore\discretionary{}{}{}}

%% Classification
\newcommand{\classlabel}{\unclasslabel}
\newcommand{\classlabelshort}{\unclass }
\newcommand{\documentlabel}{\textcolor{green5}{\textbf{UNCLASSIFIED}}}

%% Model Identity
\newModelValue{\modelname}{YOUR MODEL NAME HERE}
\newModelValue{\modelcodename}{YOUR\_CODE\_NAME}

%% Add project-specific parameter values here
```

**CRITICAL**: The `\discretionary{}{}{}` redefinition of `\_` is essential. Without it, long snake_case variable names (e.g., `WSF_ROTORCRAFT_6DOF_MOVER`) overflow table columns.

### Step 3.3: Fix ProvidesPackage Paths

The original template .sty files use `../format/` relative paths. Update them:

```bash
# In each .sty file, change:
#   \ProvidesPackage{../format/NGICStyle}
# To:
#   \ProvidesPackage{format/NGICStyle}
```

### Step 3.4: Convert Markdown to LaTeX

For each document, create:

**NGIC Documents (ADD, UAM, ICD, TDD):**
- `T150_DOC.tex` — Driver file (loads packages, sets up headers/footers, inputs content)
- `T150_DOC_Content.tex` — Body content (all sections, tables, equations, figures)

**MIL-STD-3022 Documents (Accreditation Plan, V&V Plan, V&V Report, Accreditation Report):**
- Single self-contained `.tex` file (driver + content in one file)

### NGIC Table Macro Reference

The NGIC style provides table macros with **strict argument counts**:

```latex
% 2-column: 6 args
\tableTwoNow{classlabel}{caption}{label}{col1width}{col2width}{content}

% 3-column: 7 args
\tableThrNow{classlabel}{caption}{label}{col1}{col2}{col3}{content}

% 4-column: 8 args
\tableFouNow{classlabel}{caption}{label}{col1}{col2}{col3}{col4}{content}

% 5-column: 9 args
\tableFivNow{classlabel}{caption}{label}{col1}{col2}{col3}{col4}{col5}{content}
```

Column width format: `{l,2in}` or `{c,1.5in}` (alignment, width).

**Common error**: Using `\tableFivNow` when the table only has 4 columns. Always verify the column count matches the macro.

### NGIC Equation Macro

```latex
\doMyEquation{classlabel}{caption}{label}{equation_math}
```

### Per-Document Folder Structure

Each document gets its own self-contained folder:

```
latex/docs/ADD/
├── T150_ADD.tex              # Driver
├── T150_ADD_Content.tex      # Body content
├── format/                   # Copy of shared styles
│   ├── NGICStyle.sty
│   ├── NGICGlossary.sty
│   └── DurandalBib.sty
├── values/
│   └── T150ModelValues.sty   # Project values
├── images/                   # Document-specific images
│   ├── ngic.png
│   └── *.png                 # Diagram images
└── ref.bib                   # Bibliography
```

This self-contained structure means each document can be compiled independently without path dependencies.

---

## 7. Phase 4: Diagram Generation

**Tool**: Claude Code CLI + eraser.io
**Time**: ~1-2 hours
**Output**: Professional PNG diagram images

### Step 4.1: Extract ASCII Diagrams

Use Claude Code to find all ASCII art diagrams in the LaTeX content files (verbatim and lstlisting blocks containing box-drawing characters) and extract each into a separate `.txt` file with:

1. Context header (source document, section, description)
2. Raw ASCII art
3. Suggested eraser.io prompt

### Step 4.2: Generate Images via eraser.io

For each `.txt` file:

1. Open eraser.io
2. Paste the suggested prompt from the bottom of the `.txt` file
3. Optionally paste the raw ASCII art as additional context
4. Generate the diagram
5. Export as PNG
6. Save with the same base filename as the `.txt` file

### Step 4.3: Optimize Images

Large PNGs from eraser.io can bloat PDFs. Resize to max 2000px wide:

```python
from PIL import Image
img = Image.open('diagram.png')
if img.size[0] > 2000:
    ratio = 2000 / img.size[0]
    img = img.resize((2000, int(img.size[1] * ratio)), Image.LANCZOS)
img.save('diagram.png', 'PNG', optimize=True)
```

### Step 4.4: Replace ASCII Art in LaTeX

Replace `\begin{verbatim}...\end{verbatim}` blocks with:

```latex
\begin{figure}[H]
\begin{center}
\includegraphics[width=\textwidth,height=0.85\textheight,keepaspectratio]{images/diagram.png}
\caption{\unclass Diagram Caption}
\label{fig:diagramLabel}
\end{center}
\end{figure}
```

**CRITICAL**: Always include `height=0.85\textheight,keepaspectratio`. Without the height constraint, tall images overflow across thousands of pages, creating multi-GB PDFs.

---

## 8. Phase 5: PDF Compilation

**Tool**: MiKTeX pdflatex
**Time**: ~10 minutes

### Compile Each Document

```bash
# Add MiKTeX to PATH
export PATH="/c/Users/.../AppData/Local/Programs/MiKTeX/miktex/bin/x64:$PATH"

# Compile (2 passes for cross-references)
cd latex/docs/ADD/
pdflatex --enable-installer -interaction=nonstopmode T150_ADD.tex
pdflatex --enable-installer -interaction=nonstopmode T150_ADD.tex
```

### Troubleshooting Compilation

| Error | Cause | Fix |
|-------|-------|-----|
| `File 'geometry.sty' not found` | Missing LaTeX package | Use `--enable-installer` flag |
| `Illegal unit of measure` | Wrong table macro column count | Verify `\tableXxxNow` matches data columns |
| `\begin{verbatim}` in `\fbox` | verbatim can't nest in boxes | Use `\begin{lstlisting}` or `\includegraphics` instead |
| `\begin{figure} ended by \end{document}` | Unclosed figure environment | Check brace matching in figure blocks |
| PDF is hundreds of MB | Image overflowing pages | Add `height=0.85\textheight,keepaspectratio` |
| `llowbreak` or `hspace{0pt}` in text | Corrupted sed replacement | Never use sed `\a` in replacements (it's BEL char) |
| `\_` not breakable in tables | Long identifiers overflow columns | Add `\let\SavedUnderscore\_` + `\protected\def\_{\SavedUnderscore\discretionary{}{}{}}` to preamble |
| `ProvidesPackage` mismatch warning | .sty path doesn't match `\usepackage` | Update `\ProvidesPackage{format/NGICStyle}` to match actual path |

---

## 9. Phase 6: Python DOCX Generation

**Tool**: Python 3.12 + python-docx
**Time**: ~30 minutes

### Utility Module (docx_utils.py)

Create a shared utility module with NGIC-matching formatting:

```python
# Key formatting constants (matching NGIC template)
TABLE_BLUE = RGBColor(14, 117, 188)     # NGIC table header blue
MARGINS = {'left': Inches(0.75), 'right': Inches(0.75),
           'top': Inches(0.85), 'bottom': Inches(0.85)}
BODY_FONT = 'Calibri'                    # Closest to Latin Modern Sans
BODY_SIZE = Pt(11)
CODE_FONT = 'Courier New'
CODE_SIZE = Pt(8)

# Key functions:
# create_document() - new doc with NGIC margins
# add_table_with_header() - blue header row, white text
# add_code_block() - monospace formatted code
# add_title_page() - centered title with NGIC logo
# add_revision_table() - version history
```

### Per-Document Generator

Each generator script reads content from the corresponding markdown file and produces a formatted .docx:

```python
python generate_uam.py    # Produces T150_UAM_v3.0.docx
python generate_all.py    # Produces all 8 documents
```

---

## 10. Output Structure

```
VandV_docs/Final_Documents/
├── markdown/                              # Source of truth (8 .md files)
│   ├── T150_UAM_v3.0.md
│   ├── T150_ADD_v3.0.md
│   ├── T150_ICD_v1.0.md
│   ├── T150_TDD_v1.0.md
│   ├── T150_Accreditation_Plan_v1.0.md
│   ├── T150_VV_Plan_v1.0.md
│   ├── T150_VV_Report_v1.0.md
│   └── T150_Accreditation_Report_v1.0.md
├── latex/docs/                            # Per-document LaTeX + PDF
│   ├── UAM/
│   │   ├── T150_UAM.tex                   # Driver
│   │   ├── T150_UAM_Content.tex           # Body content
│   │   ├── T150_UAM_FrontMatter.tex       # Title page
│   │   ├── T150_UAM.pdf                   # Compiled output
│   │   ├── format/ values/ images/ ref.bib
│   │   └── (build artifacts: .aux .log .toc .lof .lot)
│   ├── ADD/ ICD/ TDD/                     # Same structure
│   ├── Accreditation_Plan/                # Self-contained .tex + .pdf
│   ├── VV_Plan/ VV_Report/
│   ├── Accreditation_Report/
│   └── diagram_sources/                   # ASCII art + PNG originals
│       ├── 01_ADD_architecture_overview.txt
│       ├── 01_ADD_architecture_overview.png
│       ├── ... (17 diagrams)
│       └── README.md
└── docx/                                  # Python DOCX generators
    ├── docx_utils.py                      # Shared formatting
    ├── generate_uam.py                    # Full UAM generator
    └── generate_all.py                    # Master script (all 8)
```

---

## 11. Known Issues & Solutions

### LaTeX-Specific Issues

**Issue**: `sed` on Windows/Git Bash corrupts LaTeX files when using `\a` in replacements.
**Cause**: In sed, `\a` is the BEL character (0x07), not a literal `\a`.
**Solution**: Use Python for string replacements in LaTeX files instead of sed.

```python
# Safe replacement (use this instead of sed)
with open('file.tex', 'r', encoding='utf-8') as f:
    content = f.read()
content = content.replace(old_string, new_string)
with open('file.tex', 'w', encoding='utf-8') as f:
    f.write(content)
```

**Issue**: NGIC `\ContentFor{TAG}{content}` mechanism.
**Note**: The original templates use `\ContentFor{ADD}{...}` to conditionally include content based on `\thisdocument`. Our approach generates separate content files per document instead, which is simpler and avoids the complexity of the shared-file approach.

**Issue**: `tabularray` processes cell content differently than standard LaTeX.
**Impact**: Some LaTeX commands (like `\allowbreak`, `\hspace`) are treated as literal text inside `tabularray` cells.
**Solution**: Use TeX primitives (`\discretionary{}{}{}`) or preamble-level redefinitions rather than inline commands in table cells.

### Image-Specific Issues

**Issue**: eraser.io exports very tall images (e.g., 5000px tall for sequence diagrams).
**Impact**: Without height constraints, pdflatex creates thousands of pages trying to render the image.
**Solution**: Always use `height=0.85\textheight,keepaspectratio` in `\includegraphics`.

### Cross-Reference Issues

**Issue**: `Reference 'LastPage' undefined` on first compile.
**Solution**: Always run pdflatex twice. First pass establishes labels, second pass resolves references.

---

## 12. Adapting for Other Projects

### Step-by-Step for a New Project

1. **Copy the template infrastructure**:
   - Copy `format/` (NGICStyle.sty, NGICGlossary.sty, DurandalBib.sty)
   - Copy `images/ngic.png`
   - Copy `docx/docx_utils.py`

2. **Create project-specific ModelValues.sty**:
   - Change `\modelname`, `\modelcodename`
   - Add project-specific parameter macros
   - Keep the `\discretionary` underscore fix

3. **Create project bibliography (ref.bib)**:
   - Add entries for all references using the biblatex `@techreport` format
   - Include DoDI 5000.61 and MIL-STD-3022 as standard entries

4. **Gather project inputs** (see Section 3):
   - Source code analysis
   - Test data and results
   - Reference specifications

5. **Use Claude Code to generate markdown** (see Section 5):
   - Follow the dependency order: UAM → ADD → ICD → TDD → V&V Plan → Accred Plan → V&V Report → Accred Report
   - Provide detailed prompts with project-specific content

6. **Convert to LaTeX** (see Section 6):
   - Create driver + content files using NGIC table macros
   - Verify table macro column counts match data

7. **Generate diagrams** (see Section 7):
   - Extract ASCII art → eraser.io → PNG → `\includegraphics`

8. **Compile PDFs** (see Section 8):
   - Two passes per document
   - Check for oversized output (image height issues)

### Customizing for Non-AFSIM Projects

The NGIC templates were designed for ODESSA electronic warfare models. For other frameworks:

| ODESSA Concept | AFSIM Equivalent | Generic Equivalent |
|---------------|-------------------|-------------------|
| ODESSA GUI Setup | Scenario Configuration | Application Configuration |
| DLL Interface | Plugin (.dll/.so) | Module/Library Interface |
| Structs | AuxData Key-Value | Shared State / API |
| DLL Functions | Plugin Entry Points | Module Initialization |
| Signal Processing | Physics Simulation | Core Algorithm |
| Antenna Patterns | N/A | N/A (mark as such) |

Use the **hybrid approach**: Adapt sections where there's a clear equivalent, mark truly non-applicable sections as "N/A" per MIL-STD-3022 Section 5.3 tailoring guidance.

---

## Appendix A: Claude Code Prompt Templates

### UAM Prompt Template

```
Write the User and Analyst Manual (UAM) for [PROJECT NAME].
Write it to: [OUTPUT PATH]

Structure per NGIC 4.0 UAM template:
1. Introduction (purpose, scope, intended use)
2. Threat/System Description (specifications, CONOPS)
3. Model Description (block diagram, fidelity log, KPIs)
4. Model Components (one section per component with:
   - Overview, Functional Behavior, Equations, KPIs, Parameters, ALGs)
5. Model Setup (configuration, installation)
6. Analytic Outputs (output keys, log files)
7. Sources, Assumptions, Limitations, Gaps
8. References
9. Acronyms

Key technical details:
[List components, parameters, equations, test results]

Classification: UNCLASSIFIED with [PLACEHOLDER] for org-specific fields.
Reference DoDI 5000.61 and MIL-STD-3022.
```

### ADD Prompt Template

```
Write the Architecture Description Document (ADD) for [PROJECT NAME].
Focus on CODE STRUCTURE and ARCHITECTURE (not physics - that's in the UAM).

Structure per NGIC 4.0 ADD template:
1. Introduction
2. Architecture Description (UML, class hierarchy, registration pattern)
3. Component Sections (per component: code structure, public interfaces, pseudocode)
4. Design Decisions
5. Installation and Build
6. References, Acronyms

Template guidance says: "Do NOT include code snippets" - use pseudocode instead.
Include UML class diagrams and sequence diagrams as ASCII art.
```

---

## Appendix B: Lessons Learned

1. **Build markdown first** — It's the fastest to iterate on and serves as the content source of truth for all other formats.

2. **Parallelize where possible** — Independent documents (ADD and ICD, for example) can be generated simultaneously using background agents.

3. **Verify table macros early** — The most common LaTeX compilation error is mismatched column counts in NGIC table macros. Verify each `\tableXxxNow` call before compiling.

4. **Never use sed for LaTeX edits** — sed's `\a`, `\b`, `\f` escape sequences corrupt LaTeX backslash commands. Always use Python for LaTeX string replacements.

5. **Constrain image dimensions** — Always use both `width` and `height` with `keepaspectratio` in `\includegraphics`. Unconstrained images can create multi-GB PDFs.

6. **The preamble underscore fix is essential** — Without `\protected\def\_{\SavedUnderscore\discretionary{}{}{}}`, every snake_case identifier in tables will overflow its column.

7. **Use eraser.io for diagrams** — ASCII art diagrams in LaTeX render poorly. Professional diagram images dramatically improve document quality. Extract ASCII art into `.txt` files with prompts, generate via eraser.io, then embed as `\includegraphics`.

8. **Self-contained document folders** — Each document should have its own copies of format/, values/, images/, and ref.bib. This eliminates path dependency issues and allows independent compilation.
