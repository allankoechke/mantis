## Featureset
- [ ] Add wait for db operations to avoid throwing db locked errors immediately, esp on SQLite
- [ ] MSVC support?
- [ ] std::format bundling with optional adding if feature not supported by compiler?
- [ ] Strip all whitespaces from filenames `[,\t ]`

## Bugsets
- [ ] SEGFAULT on running debug builds on Windows ONLY
- [ ] SEGFAULT on running `ctest` but working fine on running test binary.
- [ ] Create endpoints > Dont insert on a field that was not passed in (avoid unintended default values)