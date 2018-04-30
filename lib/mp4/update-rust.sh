#!/bin/sh -e
# Script to update mp4parse-rust sources to latest upstream
# Copiesd from
#  https://hg.mozilla.org/mozilla-central/raw-file/tip/media/mp4parse-rust/update-rust.sh

# Default version.
VER="aa3ce2c187a7ab5f4219fab525227e7e7922966a"

# Accept version or commit from the command line.
if test -n "$1"; then
  VER=$1
fi

echo "Fetching sources..."
rm -rf _upstream
git clone https://github.com/hfiguiere/mp4parse-rust _upstream/mp4parse
# git clone https://github.com/alfredoyang/mp4parse_fallible _upstream/mp4parse_fallible
pushd _upstream/mp4parse
git checkout ${VER}
echo "Verifying sources..."
cargo test
popd
rm -rf mp4parse
mkdir -p mp4parse/src
cp _upstream/mp4parse/mp4parse/Cargo.toml mp4parse/
cp _upstream/mp4parse/mp4parse/src/*.rs mp4parse/src/
mkdir -p mp4parse/tests
cp _upstream/mp4parse/mp4parse/tests/*.rs mp4parse/tests/
cp _upstream/mp4parse/mp4parse/tests/*.mp4 mp4parse/tests/
rm -rf mp4parse_capi
mkdir -p mp4parse_capi/src
cp _upstream/mp4parse/mp4parse_capi/Cargo.toml mp4parse_capi/
cp _upstream/mp4parse/mp4parse_capi/build.rs mp4parse_capi/
cp _upstream/mp4parse/mp4parse_capi/include/mp4parse.h ./
cp _upstream/mp4parse/mp4parse_capi/src/*.rs mp4parse_capi/src/
#rm -rf mp4parse_fallible
#mkdir -p mp4parse_fallible
#cp _upstream/mp4parse_fallible/* mp4parse_fallible/

echo "Applying patches..."
patch -p4 < mp4parse-cargo.patch

echo "Cleaning up..."
rm -rf _upstream

#echo "Updating gecko Cargo.lock..."
#pushd ../../toolkit/library/rust/
#cargo update --package mp4parse_capi
#popd
#pushd ../../toolkit/library/gtest/rust/
#cargo update --package mp4parse_capi
#popd

echo "Updated to ${VER}."
