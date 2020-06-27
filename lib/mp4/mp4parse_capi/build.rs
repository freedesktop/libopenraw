extern crate cbindgen;

use cbindgen::{Config, RenameRule};

fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");

    let config = {
        let mut c: Config = Default::default();
        c.header = Some(
            r##"
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef __cplusplus
extern "C" {
#endif
"##
            .trim()
            .into(),
        );
        c.trailer = Some(
            r##"
#ifdef __cplusplus
} /* extern "C" */
#endif
"##
            .trim()
            .into(),
        );
        c.include_guard = Some("MP4PARSE_CAPI_H".to_owned());
        c.autogen_warning = Some(
            "// THIS FILE IS AUTOGENERATED BY mp4parse_capi/build.rs - DO NOT EDIT".to_owned(),
        );
        c.language = cbindgen::Language::C;
        c.enumeration.rename_variants = Some(RenameRule::QualifiedScreamingSnakeCase);
        c
    };

    // Generate mp4parse.h.
    let crate_dir = std::env::var("CARGO_MANIFEST_DIR").unwrap();
    let header_path = std::env::var("CARGO_TARGET_DIR").unwrap();
    let mut header_path = std::path::PathBuf::from(header_path);
    header_path.push("..");
    header_path.push("mp4parse.h");
    cbindgen::generate_with_config(&crate_dir, config)
        .expect("Could not generate header")
        .write_to_file(header_path);
}
