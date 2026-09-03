---
name: implement
description: "Implement a piece of work based on a spec or set of tickets."
disable-model-invocation: true
---

Implement the work described by the user in the spec or tickets.

Use /tdd where possible, at pre-agreed seams.

Run typechecking regularly, single test files regularly, and the full test suite once at the end.

Once done, use /code-review to review the work.

Commit your work to the current branch.

## Push `dev`

Ship the current `dev` commits to `origin`. Run this as the last step of every finished `/implement`.

1. `git status -sb` and `git branch --show-current`.
2. If the current branch is not `dev`, stop and report. Do not merge, rebase, or switch branches.
3. If files that belong to this implementation are still uncommitted, commit them first (same message style as `/implement`). Unrelated dirty files stay unstaged.
4. `git push -u origin dev`.
5. Report the remote URL and `git log -1 --oneline origin/dev`.

Done when `origin/dev` matches local `dev` and the push output is in the reply.

`--force` is a hard no. Other branches stay local.
