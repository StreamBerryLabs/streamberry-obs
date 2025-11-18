# Release Process

This document describes how to create a new release of BerryStreamCam OBS Plugin.

## Versioning

We follow [Semantic Versioning](https://semver.org/):

- **MAJOR**: Breaking changes
- **MINOR**: New features, backwards compatible
- **PATCH**: Bug fixes, backwards compatible

Current version: `1.0.0`

## Manual Release Process

### 1. Update Version Numbers

Update version in these files:

- `CMakeLists.txt` - `project(berrystreamcam VERSION x.y.z)`
- `README.md` - Badge and version references
- `make-release.sh` - `VERSION="x.y.z"`

### 2. Update Changelog

Add release notes to `CHANGELOG.md`:

```markdown
## [x.y.z] - YYYY-MM-DD

### Added

- New features

### Changed

- Changes in existing functionality

### Fixed

- Bug fixes
```

### 3. Build Release Packages

#### Linux

```bash
cd plugin
./make-release.sh
```

#### macOS

```bash
cd plugin
./make-release.sh
```

#### Windows

```powershell
cd plugin
.\make-release.ps1
```

### 4. Test Release Packages

- Extract package
- Install plugin
- Test in OBS Studio
- Verify all protocols work
- Check auto-discovery

### 5. Create Git Tag

```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

### 6. Create GitHub Release

1. Go to https://github.com/StreamBerryLabs/streamberry-obs/releases/new
2. Choose the tag you just created
3. Set release title: `BerryStreamCam v1.0.0`
4. Add release notes (from CHANGELOG.md)
5. Upload release packages:
   - `berrystreamcam-v1.0.0-linux.tar.gz`
   - `berrystreamcam-v1.0.0-macos.tar.gz`
   - `berrystreamcam-v1.0.0-windows.zip`
   - All `.sha256` checksum files
6. Publish release

## Automated Release (GitHub Actions)

We have automated build and release via GitHub Actions.

### Trigger Automatic Release

1. **Update version** in files (see step 1 above)
2. **Commit changes**:
   ```bash
   git add .
   git commit -m "Release v1.0.0"
   ```
3. **Create and push tag**:
   ```bash
   git tag v1.0.0
   git push origin main
   git push origin v1.0.0
   ```
4. **GitHub Actions will**:
   - Build for Linux, macOS, Windows
   - Run tests
   - Create release packages
   - Calculate checksums
   - Create GitHub Release automatically

### Manual Workflow Trigger

You can also trigger the workflow manually:

1. Go to **Actions** tab in GitHub
2. Select **Build and Release** workflow
3. Click **Run workflow**
4. Choose branch and version
5. Click **Run workflow**

## Release Checklist

- [ ] Version numbers updated in all files
- [ ] CHANGELOG.md updated with release notes
- [ ] Code tested on all target platforms
- [ ] Documentation updated
- [ ] Git tag created and pushed
- [ ] GitHub Release created
- [ ] Release packages uploaded
- [ ] Checksums verified
- [ ] Release announcement posted
- [ ] README badges updated

## Platform-Specific Notes

### Linux

- Test on: Arch, Ubuntu, Fedora
- Package format: `.tar.gz`
- Install location: `/usr/lib/obs-plugins/`

### macOS

- Test on: macOS 13+ (Ventura, Sonoma)
- Package format: `.tar.gz` with `.plugin` bundle
- Install location: `~/Library/Application Support/obs-studio/plugins/`
- Note: May require Gatekeeper bypass for unsigned builds

### Windows

- Test on: Windows 10, Windows 11
- Package format: `.zip`
- Install location: `C:\Program Files\obs-studio\obs-plugins\64bit\`
- Note: May require "Run as Administrator" for installation

## Rollback Procedure

If a release has critical bugs:

1. **Mark as pre-release** in GitHub
2. **Create hotfix branch**:
   ```bash
   git checkout -b hotfix/1.0.1 v1.0.0
   ```
3. **Fix the bug**
4. **Create patch release** (v1.0.1)
5. **Delete broken tag** (optional):
   ```bash
   git tag -d v1.0.0
   git push origin :refs/tags/v1.0.0
   ```

## Release Communication

After creating release:

1. **Update README.md** with new version
2. **Post announcement**:
   - GitHub Discussions
   - OBS Forums
   - Reddit (r/obs, r/LivestreamingSetup)
   - Discord servers
3. **Update documentation** with new features
4. **Notify users** via GitHub watch notifications

## Support After Release

- Monitor GitHub Issues for bug reports
- Check Discussions for questions
- Respond to installation problems
- Collect feedback for next version

## Version History

| Version | Date       | Highlights                       |
| ------- | ---------- | -------------------------------- |
| 1.0.0   | 2025-01-XX | Initial release with 5 protocols |

---

For questions about the release process, open an issue or contact the maintainers.
