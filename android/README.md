# Android bootstrap

This directory contains minimal Android wrapper scaffolding for SDL2 + CMake integration.

## Expected packaging path
- Native lib target name: `main` (already set in CMake for Android).
- Add SDL2 Android Java activity and manifest wiring in Phase-2.

For CI, build currently validates NDK compile. APK signing/release task can be enabled after Gradle wrapper is added.
