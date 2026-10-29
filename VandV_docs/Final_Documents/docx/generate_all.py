#!/usr/bin/env python3
"""
Master script to generate ALL 8 NGIC V&V documents as Word (.docx) files.

Generates the complete T-150 UAS AFSIM V&V documentation suite:
  1. UAM  - User and Analyst Manual
  2. ADD  - Architecture Description Document
  3. VVP  - Verification and Validation Plan
  4. TDD  - Test Design Document
  5. TRR  - Test Results Report
  6. VVR  - Verification and Validation Report
  7. ASR  - Accreditation Support Report (formerly RAR)
  8. ICD  - Interface Control Document

Usage:
    python generate_all.py          # Generate all documents
    python generate_all.py uam      # Generate only the UAM
    python generate_all.py add vvp  # Generate ADD and VVP

Output:
    T150_<DOC>_v3.0.docx files in this directory.
"""

import os
import sys
import time

# Allow running from any directory
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)

try:
    from docx import Document
except ImportError:
    print("ERROR: python-docx is not installed.")
    print("Install it with:")
    print("    pip install python-docx")
    print("  or:")
    print("    python3.12 -m pip install python-docx --user")
    sys.exit(1)


# =============================================================================
# Document generators
# =============================================================================

def generate_uam_doc():
    """Generate the User and Analyst Manual (UAM)."""
    from generate_uam import generate_uam
    return generate_uam()


def generate_add_doc():
    """Generate the Architecture Description Document (ADD).

    TODO: Implement full ADD content from T150_ADD_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_add.py
      - Import and use docx_utils helpers
      - Include all sections: system architecture, component diagrams,
        interface descriptions, data flow, deployment architecture
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Architecture Description Document (ADD)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full ADD content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_ADD_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_vvp_doc():
    """Generate the Verification and Validation Plan (VVP).

    TODO: Implement full VVP content from T150_VVP_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_vvp.py
      - Import and use docx_utils helpers
      - Include all sections: V&V approach, test matrix, acceptance criteria,
        schedule, resources, risk assessment
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Verification and Validation Plan (VVP)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full VVP content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_VVP_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_tdd_doc():
    """Generate the Test Design Document (TDD).

    TODO: Implement full TDD content from T150_TDD_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_tdd.py
      - Import and use docx_utils helpers
      - Include all sections: test procedures, test cases (10+),
        test data requirements, pass/fail criteria, test environment
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Test Design Document (TDD)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full TDD content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_TDD_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_trr_doc():
    """Generate the Test Results Report (TRR).

    TODO: Implement full TRR content from T150_TRR_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_trr.py
      - Import and use docx_utils helpers
      - Include all sections: test execution results, pass/fail summary,
        deficiency analysis, test evidence, results tables
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Test Results Report (TRR)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full TRR content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_TRR_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_vvr_doc():
    """Generate the Verification and Validation Report (VVR).

    TODO: Implement full VVR content from T150_VVR_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_vvr.py
      - Import and use docx_utils helpers
      - Include all sections: V&V findings, confidence assessment,
        limitations, recommendations, suitability determination
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Verification and Validation Report (VVR)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full VVR content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_VVR_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_asr_doc():
    """Generate the Accreditation Support Report (ASR).

    TODO: Implement full ASR content from T150_ASR_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_asr.py
      - Import and use docx_utils helpers
      - Include all sections: accreditation recommendation, intended use
        assessment, risk analysis, accreditation decision support
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Accreditation Support Report (ASR)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full ASR content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_ASR_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


def generate_icd_doc():
    """Generate the Interface Control Document (ICD).

    TODO: Implement full ICD content from T150_ICD_v3.0.md.
    Follow the same pattern as generate_uam.py:
      - Create generate_icd.py
      - Import and use docx_utils helpers
      - Include all sections: interface identification, AuxData key
        definitions, data types, inter-plugin data flow, external interfaces
    """
    from docx_utils import (
        create_document, add_classification_header, add_title_page,
        add_revision_table, add_heading, add_paragraph,
    )
    doc = create_document()
    add_title_page(doc, "Interface Control Document (ICD)",
                   "T-150/TRV-150 Heavy-Lift UAS Simulation Model", "3.0", "March 5, 2026")
    add_revision_table(doc, [("3.0", "2026-03-05", "Initial NGIC 4.0 template version")])
    add_heading(doc, "TODO", level=1)
    add_paragraph(doc, "Full ICD content generation to be implemented. "
                  "Follow the same pattern as generate_uam.py using docx_utils helpers.")
    add_classification_header(doc)
    output_path = os.path.join(SCRIPT_DIR, "T150_ICD_v3.0.docx")
    doc.save(output_path)
    print(f"Generated: {output_path} (stub)")
    return output_path


# =============================================================================
# Registry of all documents
# =============================================================================

DOCUMENTS = {
    "uam": ("User and Analyst Manual (UAM)", generate_uam_doc),
    "add": ("Architecture Description Document (ADD)", generate_add_doc),
    "vvp": ("Verification and Validation Plan (VVP)", generate_vvp_doc),
    "tdd": ("Test Design Document (TDD)", generate_tdd_doc),
    "trr": ("Test Results Report (TRR)", generate_trr_doc),
    "vvr": ("Verification and Validation Report (VVR)", generate_vvr_doc),
    "asr": ("Accreditation Support Report (ASR)", generate_asr_doc),
    "icd": ("Interface Control Document (ICD)", generate_icd_doc),
}


# =============================================================================
# Main
# =============================================================================

def main():
    print("=" * 70)
    print("T-150 UAS AFSIM V&V Document Generator")
    print("NGIC 4.0 Template Format - DOCX Output")
    print("=" * 70)
    print()

    # Determine which documents to generate
    if len(sys.argv) > 1:
        requested = [arg.lower() for arg in sys.argv[1:]]
        invalid = [r for r in requested if r not in DOCUMENTS]
        if invalid:
            print(f"ERROR: Unknown document(s): {', '.join(invalid)}")
            print(f"Valid options: {', '.join(DOCUMENTS.keys())}")
            sys.exit(1)
        to_generate = requested
    else:
        to_generate = list(DOCUMENTS.keys())

    total = len(to_generate)
    generated = []
    failed = []

    for idx, doc_key in enumerate(to_generate, 1):
        doc_name, doc_func = DOCUMENTS[doc_key]
        print(f"[{idx}/{total}] Generating {doc_name}...")
        start = time.time()
        try:
            output_path = doc_func()
            elapsed = time.time() - start
            print(f"         Done in {elapsed:.1f}s")
            generated.append((doc_key.upper(), output_path))
        except Exception as e:
            elapsed = time.time() - start
            print(f"         FAILED after {elapsed:.1f}s: {e}")
            failed.append((doc_key.upper(), str(e)))

    # Summary
    print()
    print("=" * 70)
    print(f"Generation complete: {len(generated)}/{total} succeeded, "
          f"{len(failed)}/{total} failed")
    print("=" * 70)

    if generated:
        print()
        print("Generated files:")
        for doc_key, path in generated:
            print(f"  {doc_key}: {path}")

    if failed:
        print()
        print("Failed:")
        for doc_key, err in failed:
            print(f"  {doc_key}: {err}")
        sys.exit(1)


if __name__ == "__main__":
    main()
