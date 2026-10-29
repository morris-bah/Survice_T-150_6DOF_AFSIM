# T-150 UAS Project — Claude Code Operational Notes

> This file holds project-specific operational notes that Claude Code reads
> on top of the root `CLAUDE.md` agent entry point. For general AFSIM
> development guidance see `.ai/system_instructions.md`.

## Git Remote & Authentication

**Active remote**: GitLab — `https://code.cdso.army.mil/ngic_fsd/model_development/t150_afsim_ngic-development.git`

**Archive remote** (`github-archive`): original GitHub repo (`DSGarrett/project005_t150_uas`). Unreachable from current network — retained locally as a museum of pre-GitLab history. Do not attempt to fetch/push it.

### Commit signing is REQUIRED

The GitLab pre-receive hook rejects unsigned commits. Signing is already
configured (x509 via smimesign, key `b2f4301b9ce262081034e9dd82afe9c3f7143dd5`,
`commit.gpgsign=true`). Every commit prompts for the CAC PIN.

- Never commit with `--no-gpg-sign` or `-c commit.gpgsign=false` — the push will be rejected.
- If a commit lands unsigned somehow, re-sign with `git commit --amend --no-edit -S` before pushing.
- Tags are also signed (`tag.gpgsign=true`).

### Branch history note

The GitLab `main` starts from a single signed "Initial import" commit
(`fa51bbd`, 2026-05-19) per the pre-receive signing policy. Full pre-import
history is available locally via `git log github-archive/main` and the other
`github-archive/*` refs (6DOF, 6DOF_Battery, 6DOF_Radar, battery_dev,
feature/spline-navigation, plugin_dev, version2).

## Plugin Installation Workflow

On Windows, use the root Conan/CMake presets documented in `README.md`.
The AFSIM build at `C:\code\AFSIM\afsim_2.2505.0\build` was previously named
`build2`; the Windows SDK recipe packages its actual files without using stale
generated project paths. The installed runtime is `C:\code\AFSIM\AFSIM`.
`cmake --build --preset windows-release` only builds; the separate
`windows-install` build preset deploys to the flat `mission_plugins` and
`wsf_plugins` directories. No sudo is required for this Windows destination.

The following workflow applies to Linux/WSL:

**AFSIM plugins MUST be installed to the AFSIM system directory to take effect.**
Building them in `plugins/build/` is not enough — the `mission` executable
loads from `/opt/afsim*/lib/wsf_plugins/` and `/opt/afsim*/bin/mission_plugins/`.

```bash
# Build all plugins (auto-detects AFSIM)
./ngic_afsim_build.sh

# Install to AFSIM (requires sudo)
./ngic_afsim_install.sh
```

**Claude has no sudo access in this environment.** The standard workflow when
plugin changes need testing:

1. Claude runs `./ngic_afsim_build.sh`
2. **Claude prompts the user**: "Please run `./ngic_afsim_install.sh` to install the updated plugins"
3. User runs the install (enters sudo password)
4. Claude proceeds with testing once user confirms

Do NOT copy built `.so`/`.dll` files to the project's local `lib/` directory
expecting them to be loaded — AFSIM does not look there.

## Python Environment

In **WSL** (Linux): use `python3.12`.
On **Windows**: use `python` or the full path `"C:/Program Files/Python312/python"`.

```bash
# WSL — correct:
python3.12 script.py
python3.12 -m pip install package --user

# WSL — wrong (will not work):
python3 script.py
pip3 install package
```

## Project layout pointer

For the canonical project structure and scenario organization, see the root
`README.md`. For agentic-AI integration details (Library at `C:\afsim\.library\`,
recursive learning system, knowledge sources), see `.ai/system_instructions.md`.

For repository history including the v2.0 → v2.1 restructure path mapping,
see `CHANGELOG.md`.
