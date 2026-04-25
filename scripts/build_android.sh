#!/bin/bash
# MC Clone Android Build Script

set -e

ANDROID_NDK="${ANDROID_NDK_HOME:-$HOME/android-ndk-r25b}"

echo "Building MC Clone for Android..."
echo "NDK: $ANDROID_NDK"

cd platforms/android

# Create native library directory
mkdir -p app/src/main/jniLibs/{armeabi-v7a,arm64-v8a,x86_64}

# Build for each ABI
for ABI in armeabi-v7a arm64-v8a x86_64; do
    echo "Building $ABI..."

    case $ABI in
        armeabi-v7a)
            TARGET="armv7-linux-android24"
            ;;
        arm64-v8a)
            TARGET="aarch64-linux-android24"
            ;;
        x86_64)
            TARGET="x86_64-linux-android24"
            ;;
    esac

    # Compile native code
    "$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++" \
        -target "$TARGET" \
        -sysroot "$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot" \
        -c ../../src/core/*.c ../../src/*.c \
        -I../../include \
        -I$ANDROID_NDK/sysroot/usr/include \
        -fPIC -Wall -O2 \
        -o app/src/main/jniLibs/$ABI/libmc-clone.a

    echo "$ABI build complete"
done

# Build APK
./gradlew assembleDebug

echo "Android build complete"