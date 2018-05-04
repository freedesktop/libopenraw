//! Module for parsing Canon CR3 files that are ISO Base Media Format
//!  aka video/mp4 streams.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.


pub const HEADER_UUID: [u8; 16] = [
    0x85, 0xc0, 0xb6,0x87,
    0x82, 0x0f,
    0x11, 0xe0,
    0x81, 0x11,
    0xf4, 0xce, 0x46, 0x2b, 0x6a, 0x48 ];

#[allow(dead_code)]
pub const XPACKET_UUID: [u8; 16] = [
    0xbe, 0x7a, 0xcf, 0xcb,
    0x97, 0xa9,
    0x42, 0xe8,
    0x9c, 0x71,
    0x99, 0x94, 0x91, 0xe3, 0xaf, 0xac ];

#[allow(dead_code)]
pub const PREVIEW_UUID: [u8; 16] = [
    0xea, 0xf4, 0x2b, 0x5e,
    0x1c, 0x98,
    0x4b, 0x88,
    0xb9, 0xfb,
    0xb7, 0xdc, 0x40, 0x6e, 0x4d, 0x16 ];
