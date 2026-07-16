name: Pull Request
about: Describe your change for reviewers
title: ""
labels: []

---

## Summary

<!-- What and why (1–3 bullets). -->

## Test plan

- [ ] `ctest --test-dir lpg2/build -L smoke --output-on-failure`
- [ ] Updated goldens if generated tables changed (`./scripts/update_golden_tables.sh`)
- [ ] Bootstrap path reviewed if `jikespg_*` touched ([BOOTSTRAP.md](../lpg2/BOOTSTRAP.md))

## Notes

<!-- Breaking changes, follow-ups, related issues. -->
